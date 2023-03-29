#include "kvraft_grpc_server.h"

// constructor
KVRaftServer::KVRaftServer(std::string name, std::string addr,
                           std::string config_path)
    : name(name),
      addr(addr),
      config_path(config_path),
      logs(Log(name)),
      term(0),
      last_applied(0),
      commit_index(0),
      persistent_voted_for(name + "_vote_for.txt"),
      state_machine_interface(name + "_state_machine.txt"),
      can_vote(true)
       {
    if (test_without_election) {
        std::cout << "Test without election." << std::endl;
        (name == "server_a") ? identity = Role::LEADER
                             : identity = Role::FOLLOWER;
        // (name == "server_a") ? set_identity(Role::LEADER)
        //                      : set_identity(Role::FOLLOWER);
        // TODO: init index
        next_index["server_b"] = 1;
    } else {
        cout << "I'm follower now" << endl;
        identity = Role::FOLLOWER;
        // set_identity(Role::FOLLOWER);
    }

    read_server_config();
    update_stubs_();
}

void KVRaftServer::server_loop() {
    std::cout << "server thread starts" << std::endl;
    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;
    builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
    builder.RegisterService(this);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Raft server listening on " << addr << std::endl;
    server->Wait();
}

void KVRaftServer::RunServer() {
    // threads
    // server_loop();
    thread server_thread(&KVRaftServer::server_loop, this);
    // server_thread.join();
    thread election_timer_thread(&KVRaftServer::election_timer_loop, this);
    thread heartbeat_thread(&KVRaftServer::leader_heartbeat_loop, this);
    
    
    heartbeat_thread.join();
    election_timer_thread.join();

    // while
    //     true {
    //         if (identity == Role::LEADER) {
    //             // send heartbeat
    //             std::cout << "I am leader: send heartbeat" << std::endl;
    //         } else if (identity == Role::FOLLOWER) {
    //             std::cout << "I am follower: wait for heartbeat" <<
    //             std::endl;
    //             // wait for heartbeat
    //             // if no heartbeat -> wait for random minutes -> become
    //             // candidate
    //         } else if (identity == Role::CANDIDATE) {
    //             std::cout << "I am Candidate: start election" << std::endl;
    //             // start election
    //         }
    //     }
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
            server_config[key] = value;
        }
    }
    return true;
}

// bool KVRaftServer::read_state_machine_config(std::string filename) {
//     // file format:
//     // TODO: keep persistent
//     // <k>/<v> e.g. A/0.0.0.0:50001
//     state_machine_interface = KeyValueStore(filename);
//     return true;
// }

// bool KVRaftServer::read_voted_for_config(std::string filename) {
//     // file format:
//     // TODO: keep persistent
//     // <term>/<vote_for_who> e.g. 1/server_a
//     persistent_voted_for = KeyValueStore(filename);
//     return true;
// }

bool KVRaftServer::update_stubs_() {
    // for (const auto& kv : raft_client_stubs_) {
    //     //TODO:
    //     raft_client_stubs_[kv.first]->reset();
    //     //kv.second.release();
    // }
    raft_client_stubs_.clear();
    for (const auto& pair : server_config) {
        const std::string& cur_name = pair.first;
        const std::string& cur_addr = pair.second;
        if (cur_name != name) {
            raft_client_stubs_[cur_addr] = KVRaft::NewStub(grpc::CreateChannel(
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
    // if (get_identity() == Role::LEADER) {
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

        // If command received from client: append entry to local log, respond
        // after entry applied to state machine applied to local log
        logs.put(logs.getMaxIndex() + 1,
                 to_string(term) + "_" + logs.transferCommand("Put", k, v));
        send_append_entries(false);
        commit_index += 1;
        // apply to state machine
        // std::cout << "start to apply" << std::endl;
        applied_log();
        // std::cout << "applied success" << std::endl;

    } else {
        // todo return leader's address
    }

    response->set_success(0);
    return Status::OK;
}

Status KVRaftServer::Get(ServerContext* context, const GetRequest* request,
                         GetResponse* response) {
    std::cout << "KVRaftServer::get" << std::endl;

    std::string k(request->key());
    std::string v;
    std::string result;
    if (identity == Role::LEADER) {
    // if (get_identity() == Role::LEADER) {
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

        // If command received from client: append entry to local log, respond
        // after entry applied to state machine applied to local log
        logs.put(logs.getMaxIndex() + 1,
                 to_string(term) + "_" + logs.transferCommand("Get", k, v));
        send_append_entries(false);
        // apply to state machine
        commit_index += 1;
        result = applied_log();

    } else {
        // todo return leader's address
    }

    response->set_success(0);
    response->set_value(result);
    return Status::OK;
}

Status KVRaftServer::RequestVote(ServerContext* context,
                                 const RequestVoteRequest* request,
                                 RequestVoteResponse* response) {
    int req_term = request->term();
    string req_candidate_name = request->candidate_name();
    int req_last_log_index = request->last_log_index();
    int req_last_log_term = request->last_log_term();
    response->set_term(term);
    response->set_vote_granted(false);
    if (can_vote == false) {
        return Status::OK;
    }
    if (identity == Role::LEADER) {
    // if (get_identity() == Role::LEADER) {
        cout << "deny0" << endl;
        return Status::OK;
    }
    if (term > req_term) {
        cout << "deny1" << endl;
        return Status::OK;
    }
    // persistent_voted_for
    if (persistent_voted_for.Get(to_string(req_term)).empty() || persistent_voted_for.Get(to_string(req_term)) == req_candidate_name) {
        if (logs.getTermByIndex(logs.getMaxIndex()) > req_last_log_term ||
            (logs.getTermByIndex(logs.getMaxIndex()) == req_last_log_term &&
             logs.getMaxIndex() > req_last_log_index)) {
            // deny
            cout << "term: " << term
                 << " req_last_log_term: " << req_last_log_term
                 << " logs max index: " << logs.getMaxIndex()
                 << " last log term: "  
                 << logs.getTermByIndex(logs.getMaxIndex()) << endl;
            cout << "deny2" << endl;
        } else {
            // grant vote
            persistent_voted_for.Put(to_string(term), req_candidate_name);
            response->set_vote_granted(true);
            
            // heartbeat
            election_timer = std::chrono::high_resolution_clock::now();
        }
        return Status::OK;
    }
    // if (voted_for.empty() || voted_for == req_candidate_name) {
    //     if (logs.getTermByIndex(logs.getMaxIndex()) > req_last_log_term ||
    //         (logs.getTermByIndex(logs.getMaxIndex()) == req_last_log_term &&
    //          logs.getMaxIndex() > req_last_log_index)) {
    //         // deny
    //         cout << "term: " << term
    //              << " req_last_log_term: " << req_last_log_term
    //              << " logs max index: " << logs.getMaxIndex()
    //              << " last log term: "
    //              << logs.getTermByIndex(logs.getMaxIndex()) << endl;
    //         cout << "deny2" << endl;
    //     } else {
    //         // grant vote
    //         response->set_vote_granted(true);
    //         // heartbeat
    //         election_timer = std::chrono::high_resolution_clock::now();
    //     }
    //     return Status::OK;
    // }
    // cout << "deny3" << endl;
    return Status::OK;
}

Status KVRaftServer::AppendEntries(ServerContext* context,
                                   const AppendEntriesRequest* request,
                                   AppendEntriesResponse* response) {
    int req_term = request->term();
    can_vote = false;
    // heartbeat
    election_timer = std::chrono::high_resolution_clock::now();
    // convert to follower
    if (identity == Role::CANDIDATE) {
    // if (get_identity() == Role::CANDIDATE) {
        cout << "I'm follower now" << endl;
        identity = Role::FOLLOWER;
        // set_identity(Role::FOLLOWER);
        // voted_for.clear();
        vote_result.clear();
    }
    if (request->entries().size() == 0) {
        std::cout << "heartbeat received" << std::endl;
        std::cout << "follower term is " << term << std::endl;
        std::cout << "follower req_term is " << req_term << std::endl;
        // TODO: trigger hearbeat timeout reset
        if (term < req_term) {
            term = req_term;
        }
        response->set_term(term);
        response->set_success(true);
        return Status::OK;
    }
    // get all data from the the leader's request
    string req_leader_name = request->leader_name();
    int req_prev_log_index = request->prev_log_index();
    int req_prev_log_term = request->prev_log_term();
    int req_leader_commit = request->leader_commit();
    vector<LogEntry> req_entries;
    LogEntry log_entry;
    for (const auto& tmp_log_entry : request->entries()) {
        log_entry.set_index(tmp_log_entry.index());
        log_entry.set_term(tmp_log_entry.term());
        log_entry.set_command(tmp_log_entry.command());
        req_entries.push_back(log_entry);
    }
    // The follower does the following checks
    // Checks if its term is up-to-date.
    // If the follower's term is greater than the leader's term, it rejects the
    // RPC.
    if (term > req_term) {
        response->set_term(term);
        response->set_success(false);
        return Status::OK;
    }
    // The follower then checks if it has a log entry at prev_log_index
    // with a term that matches prev_log_term. If not, it rejects the RPC.
    int last_index = logs.getMaxIndex();
    if (last_index < req_prev_log_index) {
        response->set_term(term);
        response->set_success(false);
        return Status::OK;
    }
    int target_term = logs.getTermByIndex(req_prev_log_index);
    if (target_term != req_prev_log_term) {
        logs.removeAfterIndex(req_prev_log_index);
        response->set_term(term);
        response->set_success(false);
        return Status::OK;
    }
    // If the checks pass, the follower removes any conflicting entries and
    // appends the new entries from the entries field of the RPC to its log.
    for (auto entry : req_entries) {
        logs.put(entry.index(),
                 to_string(entry.term()) + "_" + entry.command());
    }
    // The follower updates its commitIndex according to the leader_commit
    // field, applying any newly committed entries to its state machine.
    commit_index = req_leader_commit;
    // Finally, the follower sends a response to the leader,
    // indicating whether the AppendEntries RPC was successful or not.
    response->set_term(term);
    response->set_success(true);
    return Status::OK;
}

// -------------------------------------------------------------------------------------------------
// GRPC Client API
bool KVRaftServer::ClientAppendEntries(shared_ptr<KVRaft::Stub> stub_,
                                       Log log_entries, bool is_heartbeat,
                                       int prev_log_index, int prev_log_term,
                                       int commit_index, int msg_term) {
    AppendEntriesRequest request;
    AppendEntriesResponse response;
    Status status;
    ClientContext context;

    request.set_term(msg_term);
    request.set_leader_name(name);
    request.set_prev_log_index(prev_log_index);
    request.set_prev_log_term(prev_log_term);
    request.set_leader_commit(commit_index);

    if (is_heartbeat) {
        status = stub_->AppendEntries(&context, request, &response);
        if (status.ok() && response.success() == 0) {
            if (term < response.term()) {
                // step back if other server have higher term
                identity = Role::FOLLOWER;
                // set_identity(Role::FOLLOWER);
            }
            return true;
        }
        return false;
    }

    LogEntry log;
    vector<LogEntry> sent_logs;

    log.set_index(prev_log_index + 1);
    log.set_term(logs.getTermByIndex(prev_log_index + 1));
    log.set_command(logs.getCommandByIndex(prev_log_index + 1));

    sent_logs.push_back(log);
    for (const auto& log : sent_logs) {
        *(request.add_entries()) = log;
    }

    status = stub_->AppendEntries(&context, request, &response);
    if (status.ok()) {
        if (response.term() > term) {
            // step back if other server have higher term
            term = response.term();
            identity = Role::FOLLOWER;
            // return ClientAppendEntries(stub_, logs, is_heartbeat,
            //                            prev_log_index, prev_log_term,
            //                            commit_index, term);
            // return true;
        }
        // resend logic should be handled by caller.
        return response.success();
    }
    return false;
}

bool KVRaftServer::ClientRequestVote(shared_ptr<KVRaft::Stub> stub_,
                                     std::string receive_name,
                                     const string candidate_name,
                                     const int last_log_index,
                                     const int last_log_term) {
    RequestVoteRequest request;
    RequestVoteResponse response;
    Status status;
    ClientContext context;

    request.set_term(term);
    request.set_candidate_name(candidate_name);
    request.set_last_log_index(last_log_index);
    request.set_last_log_term(last_log_term);

    status = stub_->RequestVote(&context, request, &response);
    if (status.ok()) {
        cout << "get vote response from " << receive_name
             << " term: " << response.term()
             << " result: " << response.vote_granted() << endl;
        if (term < response.term()) {
            // step back if other server have higher term
            term = response.term();
            identity = Role::FOLLOWER;
            // set_identity(Role::FOLLOWER);
        }
        if (response.vote_granted()) {
            // update vote
            vote_result[receive_name] = response.vote_granted();
            check_vote();
        }
    }

    return true;
}

void KVRaftServer::check_vote() {
    int total_voter_n = raft_client_stubs_.size();
    int voted_n = 0;
    for (auto& pair : vote_result) {
        if (pair.second) {
            voted_n++;
        }
    }
    if (voted_n * 2 > total_voter_n) {
        // become leader
        cout << "I'm leader now" << endl;
        if (identity == Role::CANDIDATE) identity = Role::LEADER;
        // change_identity(Role::LEADER, Role::CANDIDATE);
        // voted_for.clear();
        vote_result.clear();
    }
}

// Server function API

void KVRaftServer::send_append_entries(bool is_heartbeat) {
    Role cur_identity = identity;
    int check_majority = 0;
    for (const auto& pair : raft_client_stubs_) {
        const std::string cur_server = pair.first;
        std::shared_ptr<KVRaft::Stub> cur_stub_ = pair.second;

        if (is_heartbeat == true) {
            std::cout << "send heartbeat to: " << cur_server << std::endl;
            ClientAppendEntries(cur_stub_, logs, is_heartbeat, -1, -1, -1,
                                term);
        } else {
            // If last log index ≥ nextIndex for a follower: send AppendEntries
            // RPC with log entries starting at nextIndex
            //   • If successful: update nextIndex and matchIndex for follower
            //   (§5.3) • If AppendEntries fails because of log inconsistency:
            //   decrement nextIndex and retry (§5.3)
            int cur_next_index;
            while (logs.getMaxIndex() >= next_index[cur_server] && cur_identity == identity) {
                cur_next_index = next_index[cur_server];
                if (ClientAppendEntries(cur_stub_, logs, is_heartbeat,
                                        cur_next_index - 1,
                                        logs.getTermByIndex(cur_next_index - 1),
                                        commit_index, term)) {
                    next_index[cur_server] += 1;   // cur_next_index + 1;
                    match_index[cur_server] += 1;  // cur_next_index;
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
//
std::string KVRaftServer::applied_log() {
    // TODO: current asumption: no delay applied log
    std::string result = "";
    while (last_applied + 1 <= commit_index) {
        std::string command = logs.getCommandByIndex(last_applied + 1);
        std::string behavior;
        std::string key;
        std::string val;
        logs.parseCommand(command, behavior, key, val);
        std::cout << "last applied index: " << last_applied + 1 << std::endl;

        std::cout << "command: " << command << " b: " << behavior
                  << " key: " << key << " val: " << val << std::endl;
        if (behavior == "P") {
            state_machine_interface.Put(key, val);
        } else if (behavior == "G") {
            result = state_machine_interface.Get(key);
        } else {
            std::cout << "eerrrorrr" << std::endl;
        }
        last_applied++;
    }
    return result;
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
void KVRaftServer::leader_heartbeat_loop() {
    std::cout << "leader heartbeat thread starts" << std::endl;
    while (true) {
        if (identity == Role::LEADER) {
            std::cout << "heartbeat thread is sending a heartbeat" << std::endl;
            std::cout << "leader term: " << term << std::endl;
            send_append_entries(true);
        }
        std::this_thread::sleep_for(
            std::chrono::milliseconds(HEARTBEAT_INTERVAL));
    }
}
// -------------------------------------------------------------------------------------------------
// Election timeout API
int KVRaftServer::random_election_timeout() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(MIN_ELECTION_TIMEOUT,
                                         MAX_ELECTION_TIMEOUT);
    return dist(gen);
}

void KVRaftServer::election_timer_loop() {
    std::cout << "election timer thread starts" << std::endl;
    while (true) {
        auto sleep_start = std::chrono::high_resolution_clock::now();
        std::this_thread::sleep_for(
            std::chrono::milliseconds(random_election_timeout()));
        if (election_timer <= sleep_start && identity == Role::FOLLOWER) {
            // if during sleep time, the server recieve appendEntries,
            can_vote = true;
            start_election();
        }
        if (election_timer <= sleep_start && identity == Role::CANDIDATE) {
            // if a period of time goes by with no winner
            cout << "I'm follower now" << endl;
            identity = Role::FOLLOWER;
            // voted_for.clear();
            vote_result.clear();
        }
    }
}
// -------------------------------------------------------------------------------------------------
// Election API
void KVRaftServer::start_election() {
    std::cout << "starting election" << std::endl;
    identity = Role::CANDIDATE;
    term++;
    if (!persistent_voted_for.Get(to_string(term)).empty()) {
        election_timer = std::chrono::high_resolution_clock::now();
        return;
    };
    persistent_voted_for.Put(to_string(term), addr);
    vote_result[addr] = true;
    // voted_for = addr;
    
    // Reset election timer
    election_timer = std::chrono::high_resolution_clock::now();
    // Send RequestVote RPCs to all other servers
    for (const auto& pair : raft_client_stubs_) {
        if (identity == Role::CANDIDATE){
            const std::string cur_addr = pair.first;
            std::shared_ptr<KVRaft::Stub> cur_stub_ = pair.second;
            cout << "send ClientRequestVote to " << cur_addr << endl;
            ClientRequestVote(cur_stub_, cur_addr, voted_for, logs.getMaxIndex(),
                            logs.getTermByIndex(logs.getMaxIndex()));
        }
        
    }
}

// void KVRaftServer::change_identity(Role new_role, Role previous_role) {
//     std::lock_guard<std::mutex> lock(role_mutex);
//     if (identity = previous_role) {
//         identity = new_role;
//     }
// }

// Role KVRaftServer::get_identity() {
//     std::lock_guard<std::mutex> lock(role_mutex);
//     return identity;
// };

// void KVRaftServer::set_identity(Role new_role) {
//     std::lock_guard<std::mutex> lock(role_mutex);
//     identity = new_role;
// }

// -------------------------------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "you must provide three arguments: name, addr, config_path"
                  << std::endl;
        std::cout << "For example: A, 0.0.0.0:50001, ./config.txt" << std::endl;
        return 0;
    }
    string name = argv[1];
    string addr = argv[2];
    string path = argv[3];
    KVRaftServer raft_node = KVRaftServer(name, addr, path);
    raft_node.RunServer();

    return 0;
}