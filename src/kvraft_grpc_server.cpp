#include "kvraft_grpc_server.h"

// constructor
KVRaftServer::KVRaftServer(std::string name, std::sting addr,
                           std::string config_path)
    : name(name), addr(addr), config_path(config_path) {
    curr_time = std::chrono::high_resolution_clock::now();
    identity = Role::FOLLOWER;
    // todo init logs = Logs()
    read_server_config();
    update_stubs_();
}

// bool KVRaftServer::heartbeat() {
//     std::thread([this]() {
//         while (true) {
//             // if (identity == Role::LEADER) {
//             //     raft_->SendHeartbeats();
//             // }
//             // std::this_thread::sleep_for(
//             //     std::chrono::milliseconds(HEARTBEAT_INTERVAL_MS));
//         }
//     }).detach();

//     return true;
// }
// RunServer可以是 KVRaftServer 的一个 function 吗？
void KVRaftServer::RunServer() {
    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;
    builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
    builder.RegisterService(this);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Raft server listening on " << addr << std::endl;
    server->Wait();

    while
        true {
            if (identity == Role::LEADER) {
                // send heartbeat
                std::cout << "I am leader: send heartbeat" << std::endl;
            } else if (identity == Role::FOLLOWER) {
                std::cout << "I am follower: wait for heartbeat" << std::endl;
                // wait for heartbeat
                // if no heartbeat -> wait for random minutes -> become
                // candidate
            } else if (identity == Role::CANDIDATE) {
                std::cout << "I am Candidate: start election" << std::endl;
                // start election
            }
        }
}

bool KVRaftServer::read_server_config() {
    // file format:
    // <name>/<addr> e.g. A/0.0.0.0:50001
    std::ifstream infile(config_path);
    std::string line;
    while (std::getline(infile, line)) {
        size_t pos = line.find('/');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1, line.size() - pos - 1);
            server_config->Put(key, value);
        }
    }
    return true;
}

bool KVRaftServer::update_stubs_() {
    for (const auto& kv : raft_client_stubs_) {
        kv.second.release();
    }
    raft_client_stubs_.clear();
    for (const auto& pair : server_config) {
        const std::string& cur_name = pair.first;
        const std::string& cur_addr = pair.second;
        if (cur_name != name) {
            raft_client_stubs_[cur_name] = KVRaft::NewStub(grpc::CreateChannel(
                cur_addr, grpc::InsecureChannelCredentials()));
        }
    }
    return true;
}

// -------------------------------------------------------------------------------------------------
// GRPC Server API
Status KVRaftServer::Put(ServerContext* context, const PutRequest* request,
                         PutResponse* response) {
    std::cout << "KVRaftServer::put" << std::endl;
    std::string k(request->key());
    std::string v(request->value());
    if (identity == Role::LEADER) {
        // int cnt = 0;
        // for (const auto& pair : raft_client_stubs_) {
        //     const std::string& name = pair.first;
        //     const std::string& stub_ = pair.second;
        //     bool ret = stub_->ClientAppendEntries(/* todo */);
        //     cnt += ret;
        // }
        // if (cnt >= /*majority*/) {
        //     return /*ok*/
        // } else {
        //     return /*not ok*/
        // }

        // If command received from client: append entry to local log, respond after entry applied to state machine
        // applied to local log
        logs.put(logs.getMaxIndex() + 1, to_string(term) + "_" + transferCommand("Put", k, v));
        send_append_entries(false);

    } else {
        // todo return leader's address
    }

    inmem_store.Put(k, v);
    response->set_success(0);

    return Status::OK;
}

Status KVRaftServer::Get(ServerContext* context, const GetRequest* request,
                         GetResponse* response) {
    std::cout << "KVRaftServer::get" << std::endl;
    std::string v(inmem_store.Get(request->key()));
    response->set_success(0);
    response->set_value(v);
    return Status::OK;
}

Status KVRaftServer::AppendEntries(ServerContext* context,
                                   const AppendEntriesRequest* request,
                                   AppendEntriesResponse* response) {
    // heartbeat
    curr_time = std::chrono::high_resolution_clock::now();
    // get all data from the the leader's request
    int req_term = request->term();
    int req_leader_id = request->leader_id();
    int req_prev_log_index = request->prev_log_index();
    int req_prev_log_term = request->prev_log_term();
    int req_leader_commit = request->leader_commit();
    vector<LogEntry> req_entries;
    for (const auto& tmp_log_entry : request->entries()) {
        LogEntry log_entry;
        log_entry.index = tmp_log_entry.index();
        log_entry.term = tmp_log_entry.term();
        log_entry.command = tmp_log_entry.command();
        req_entries.push_back(log_entry);
    }
    // The follower does the following checks
    // Checks if its term is up-to-date.
    // If the follower's term is greater than the leader's term, it rejects the
    // RPC.
    if (term > req_term) {
        response->set_term = term;
        response->set_success = false;
        return Status::OK;
    }
    // The follower then checks if it has a log entry at prev_log_index
    // with a term that matches prev_log_term. If not, it rejects the RPC.
    int last_index = logs.getMaxIndex();
    if (last_index < req_prev_log_index) {
        response->set_term = term;
        response->set_success = false;
        return Status::OK;
    }
    int target_term = logs.getTermByIndex(req_prev_log_index);
    if (target_term != req_prev_log_term) {
        logs.removeAfterIndex(req_prev_log_index);
        response->set_term = term;
        response->set_success = false;
        return Status::OK;
    }
    // If the checks pass, the follower removes any conflicting entries and
    // appends the new entries from the entries field of the RPC to its log.
    for (auto entry : req_entries) {
        logs.put(entry.index, to_string(entry.term) + "_" + entry.command);
    }
    // The follower updates its commitIndex according to the leader_commit
    // field, applying any newly committed entries to its state machine.
    leader_commit = req_leader_commit;
    // Finally, the follower sends a response to the leader,
    // indicating whether the AppendEntries RPC was successful or not.
    response->set_term = term;
    response->set_success = true;
    return Status::OK;
}


// -------------------------------------------------------------------------------------------------
// GRPC Client API
bool KVRaftServer::ClientAppendEntries(unique_ptr<KVRaft::Stub> stub_ , Log log_entries,
                         bool is_heartbeat, int prev_log_index,
                         int prev_log_term, int commit_index, int term) {
    AppendEntriesRequest request;
    AppendEntriesResponse response;
    Status status;
    ClientContext context;


    if (is_heartbeat) {
        status = stub_->AppendEntries(&context, request, &response);
        if (status.ok() && response.success() == 0) return true;
        return false;
    }

    request->set_term(term);
    request->set_leader_id(leader_id);
    request->set_prev_log_index(prev_log_index);
    request->set_prev_log_term(prev_log_term);
    request->set_leader_commit(commit_index);
    request->set_heartbeat(false);

    LogEntry log;
    vector<LogEntry> sent_logs;

    log.set_index(i);
    log.set_term(log_entries.getTermByIndex(i));
    log.set_command(log_entries.getCommandByIndex(i));
    sent_logs.push_back(log);
    for (const auto& log : sent_logs) {
        *(request.add_entries()) = log;
    }

    status = stub_->AppendEntries(&context, request, &response);
    if (status.ok()) {
        if (response.term() > term) {
            term = response.term();
            return ClientAppendEntries(stub_, log_entries, is_heartbeat,
                                       prev_log_index, prev_log_term, term);
        }
        // resend logic should be handled by caller.
        return response.success();
    }
    return false;
}

// Server function API

void KVRaftServer::send_append_entries(bool is_heartbeat) {
    int check_majority = 0;
    for (const auto& pair : raft_client_stubs_) {
        const std::string& cur_server = pair.first;
        const std::string& cur_stub_ = pair.second;
        
        if (is_heartbeat == true) {
            ClientAppendEntries(cur_stub_, NULL, is_heartbeat, -1, -1, -1, term);
        } else {
            // If last log index ≥ nextIndex for a follower: send AppendEntries RPC with log entries starting at nextIndex
            //   • If successful: update nextIndex and matchIndex for follower (§5.3)
            //   • If AppendEntries fails because of log inconsistency: decrement nextIndex and retry (§5.3)
            int cur_next_index;
            while (logs.getMaxIndex() >= next_index[cur_server]) {
                cur_next_index = next_index[cur_server];
                if (ClientAppendEntries(cur_stub_, logs, is_heartbeat,
                                       cur_next_index - 1, logs.getTermByIndex(cur_next_index - 1), term)) {
                    next_index[cur_server] = cur_next_index;
                    match_index[cur_server] = cur_next_index - 1;
                } else {
                    next_index[cur_server] -= 1;
                }
            }
            
        }
        // TODO:
        // check_majority += 1;
        // if (check majority > 3/2) {
        //     applied_commited
            
        // }
    }
}



// -------------------------------------------------------------------------------------------------
// EXAMPLE API
Status KVRaftServer::SayHello(ServerContext* context,
                              const HelloRequest* request, HelloReply* reply) {
    std::string prefix("Hello ");
    reply->set_message(prefix + request->name());
    return Status::OK;
}
// -------------------------------------------------------------------------------------------------
// Heartbeat interval API
void leader_heartbeat_loop(KVRaftServer& raft_node) {
    while (raft_node.is_leader() && !raft_node.is_shutdown()) {
        raft_node.send_append_entries_to_all_followers();
        std::this_thread::sleep_for(
            std::chrono::milliseconds(HEARTBEAT_INTERVAL_MS));
    }
}
// -------------------------------------------------------------------------------------------------
// Election timeout API
int random_election_timeout() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(MIN_ELECTION_TIMEOUT,
                                         MAX_ELECTION_TIMEOUT);
    return dist(gen);
}

void election_timer_loop(KVRaftServer& raft_node) {
    while (!raft_node.is_shutdown()) {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(random_election_timeout()));
        if (!raft_node.has_received_recent_communication()) {
            raft_node.start_election();
        }
    }
}
// -------------------------------------------------------------------------------------------------
int main(int argc, char** argv) {
    KVRaftServer raft_node;
    thread election_timer_thread(election_timer_loop, std::ref(raft_node));
    thread heartbeat_thread(leader_heartbeat_loop, std::ref(raft_node));
    heartbeat_thread.join();
    election_timer_thread.join();
    // RunServer();

    return 0;
}