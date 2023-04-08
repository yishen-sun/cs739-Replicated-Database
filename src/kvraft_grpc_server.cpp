#include "kvraft_grpc_server.h"

// constructor
KVRaftServer::KVRaftServer(std::string name, std::string addr, std::string config_path)
    : name(name),
      addr(addr),
      config_path(config_path),
      logs(Log(name)),
      term(0),
      last_applied(0),
      commit_index(0),
      persistent_voted_for(name + "_vote_for.txt"),
#ifndef USE_REDIS
      state_machine_interface(name + "_state_machine.txt"),
#endif
      can_vote(true) {
    cout << "I'm follower now (line 16 - init)" << endl;
    identity = Role::FOLLOWER;

    election_timer = std::chrono::high_resolution_clock::now();
    prev_heartbeat = election_timer;
    random_election_duration = std::chrono::milliseconds(random_election_timeout());
    read_server_config();
    update_stubs_();
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

bool KVRaftServer::update_stubs_() {
    raft_client_stubs_.clear();
    for (const auto& pair : server_config) {
        const std::string& cur_name = pair.first;
        const std::string& cur_addr = pair.second;
        if (cur_name != name) {
            raft_client_stubs_[cur_addr] =
                KVRaft::NewStub(grpc::CreateChannel(cur_addr, grpc::InsecureChannelCredentials()));
        }
    }
    return true;
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
    // heartbeat clock
    // election clock
    while (true) {
        // get
        auto cur_time = std::chrono::high_resolution_clock::now();
        auto heartbeat_duration_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(cur_time - prev_heartbeat);
        auto election_duration_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(cur_time - election_timer);
        if (identity == Role::LEADER) {
            if (heartbeat_duration_ms > 15ms) {
                send_append_entries(true);
                prev_heartbeat = cur_time;
            }
        } else if (identity == Role::FOLLOWER) {
            if (heartbeat_duration_ms > 2000ms && can_vote == false) {
                std::cout << "No receiving heartbeat, turn to can vote and prepare for election"
                          << std::endl;
                can_vote = true;
                leader_addr.clear();
                prev_heartbeat = cur_time;
                election_timer = cur_time;
                random_election_duration = std::chrono::milliseconds(random_election_timeout());
            } else if (election_duration_ms > random_election_duration && can_vote) {
                std::cout << "Waiting for random period and turn to candidate" << std::endl;
                identity = Role::CANDIDATE;
                prev_heartbeat = cur_time;
                can_vote = false;
                start_election();
            }
        } else if (identity == Role::CANDIDATE) {
            if (heartbeat_duration_ms > 500ms) {
                std::cout << "No result: step down" << std::endl;
                identity = Role::FOLLOWER;
            }
        } else {
            std::cout << "error" << std::endl;
        }
    }
}

void KVRaftServer::RunServer() {
    thread server_thread(&KVRaftServer::server_loop, this);
    thread state_machine_thread(&KVRaftServer::state_machine_loop, this);

    server_thread.join();
    state_machine_thread.join();
}

// -------------------------------------------------------------------------------------------------
// GRPC Server API
Status KVRaftServer::Put(ServerContext* context, const PutRequest* request, PutResponse* response) {
    std::cout << "KVRaftServer::put" << std::endl;
    std::string k(request->key());
    std::string v(request->value());
    if (identity == Role::LEADER) {
        // If command received from client: append entry to local log, respond
        // after entry applied to state machine applied to local log
        logs.put(logs.getMaxIndex() + 1, to_string(term) + "_" + logs.transferCommand("Put", k, v));
        std::cout << "send append entries" << std::endl;
        while (send_append_entries(false) == false) {
            ;
        }
        commit_index += 1;
        // apply to state machine
        // std::cout << "start to apply" << std::endl;
        std::cout << "send update commit" << std::endl;
        applied_log(commit_index);
        response->set_success(0);
    } else {
        if (leader_addr.empty()) {
            response->set_master_addr("NO_MASTER_YET");
        } else {
            response->set_master_addr(leader_addr);
        }
        response->set_success(1);
    }
    return Status::OK;
}

Status KVRaftServer::Get(ServerContext* context, const GetRequest* request, GetResponse* response) {
    std::cout << "KVRaftServer::get" << std::endl;

    std::string k(request->key());
    std::string v;
    std::string result;
    if (identity == Role::LEADER) {
        // If command received from client: append entry to local log, respond
        // after entry applied to state machine applied to local log
        logs.put(logs.getMaxIndex() + 1, to_string(term) + "_" + logs.transferCommand("Get", k, v));
        while (send_append_entries(false) == false) {
            ;
        }
        commit_index += 1;

        result = applied_log(commit_index);
        response->set_success(0);
        response->set_value(result);
    } else {
        if (leader_addr.empty()) {
            response->set_master_addr("NO_MASTER_YET");
        } else {
            response->set_master_addr(leader_addr);
        }
        response->set_success(1);
    }
    return Status::OK;
}

Status KVRaftServer::RequestVote(ServerContext* context, const RequestVoteRequest* request,
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
        cout << "deny because I'm leader" << endl;
        return Status::OK;
    }
    if (term > req_term) {
        cout << "deny because I have larger term" << endl;
        return Status::OK;
    }
    // persistent_voted_for
    if (persistent_voted_for.Get(to_string(req_term)).empty() ||
        persistent_voted_for.Get(to_string(req_term)) == req_candidate_name) {
        if (logs.getTermByIndex(logs.getMaxIndex()) > req_last_log_term ||
            (logs.getTermByIndex(logs.getMaxIndex()) == req_last_log_term &&
             logs.getMaxIndex() > req_last_log_index)) {
            // deny
            cout << "term: " << term << " req_last_log_term: " << req_last_log_term
                 << " logs max index: " << logs.getMaxIndex()
                 << " last log term: " << logs.getTermByIndex(logs.getMaxIndex()) << endl;
            cout << "deny because I have larger last log term or larger last "
                    "log index"
                 << endl;
        } else {
            // grant vote
            persistent_voted_for.Put(to_string(term), req_candidate_name);
            response->set_vote_granted(true);

            // heartbeat
            can_vote = false;
            prev_heartbeat = std::chrono::high_resolution_clock::now();
        }
        return Status::OK;
    }
    return Status::OK;
}

Status KVRaftServer::AppendEntries(ServerContext* context, const AppendEntriesRequest* request,
                                   AppendEntriesResponse* response) {
    std::cout << "rec append entries" << std::endl;
    int req_term = request->term();
    string req_leader_name = request->leader_name();
    int req_leader_commit = request->leader_commit();
    std::cout << "AppendEntries from " << req_leader_name << std::endl;

    can_vote = false;
    // heartbeat

    prev_heartbeat = std::chrono::high_resolution_clock::now();
    // convert to follower
    if (identity == Role::CANDIDATE && term <= req_term) {
        can_vote = false;
        cout << "I'm follower now 234" << endl;
        identity = Role::FOLLOWER;
        vote_result.clear();
    } else if (identity == Role::CANDIDATE) {
        std::cout << "candidate don't want to response to leader" << std::endl;
        can_vote = true;
        response->set_term(term);
        response->set_success(false);
        return Status::OK;
    }
    leader_addr = server_config[req_leader_name];

    if (request->entries().size() == 0) {
        std::cout << "heartbeat received from " << req_leader_name << std::endl;
        // std::cout << "follower term is " << term << std::endl;
        // std::cout << "follower req_term is " << req_term << std::endl;
        if (term < req_term) {
            term = req_term;
        }
        commit_index = req_leader_commit;
        response->set_term(term);
        response->set_success(true);
        return Status::OK;
    }
    // get remain data from the the leader's request
    int req_prev_log_index = request->prev_log_index();
    int req_prev_log_term = request->prev_log_term();

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
    // If the follower's term is greater than the leader's term, it rejects the RPC.
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
        logs.put(entry.index(), to_string(entry.term()) + "_" + entry.command());
    }
    // The follower updates its commitIndex according to the leader_commit
    // field, applying any newly committed entries to its state machine.
    std::cout << "Update commit from: " << commit_index << " to " << req_leader_commit << std::endl;
    commit_index = req_leader_commit;
    // Finally, the follower sends a response to the leader,
    // indicating whether the AppendEntries RPC was successful or not.
    response->set_term(term);
    response->set_success(true);
    return Status::OK;
}

// -------------------------------------------------------------------------------------------------
// GRPC Client API
bool KVRaftServer::ClientAppendEntries(shared_ptr<KVRaft::Stub> stub_, Log& log_entries,
                                       bool is_heartbeat, int prev_log_index, int prev_log_term,
                                       int commit_index_, int msg_term) {
    AppendEntriesRequest request;
    AppendEntriesResponse response;
    Status status;
    ClientContext context;

    request.set_term(msg_term);
    request.set_leader_name(name);
    request.set_prev_log_index(prev_log_index);
    request.set_prev_log_term(prev_log_term);
    request.set_leader_commit(commit_index_);

    if (is_heartbeat) {
        std::cout << "send heartbeat inside clientAppendEntries" << std::endl;
        status = stub_->AppendEntries(&context, request, &response);
        if (status.ok() && response.success() == true) {
            if (term < response.term()) {
                // step back if other server have higher term
                identity = Role::FOLLOWER;
                cout << "I'm follower now 332" << endl;
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
            cout << "I'm follower now 358" << endl;
            return false;
        }
        // resend logic should be handled by caller.
        return true;
    }
    return false;
}

bool KVRaftServer::ClientRequestVote(shared_ptr<KVRaft::Stub> stub_, std::string receive_name,
                                     const string candidate_name, const int last_log_index,
                                     const int last_log_term, int& max_term) {
    RequestVoteRequest request;
    RequestVoteResponse response;
    Status status;
    ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + std::chrono::milliseconds(100));

    request.set_term(term);
    request.set_candidate_name(candidate_name);
    request.set_last_log_index(last_log_index);
    request.set_last_log_term(last_log_term);

    status = stub_->RequestVote(&context, request, &response);
    if (status.ok()) {
        cout << "get vote response from " << receive_name << " term: " << response.term()
             << " result: " << response.vote_granted() << endl;
        if (term < response.term()) {
            // step back if other server have higher term
            int res_term_int = static_cast<int>(response.term());
            max_term = std::max(max_term, res_term_int);
            identity = Role::FOLLOWER;
            cout << "I'm follower now 389" << endl;
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
        cout << "I'm leader now 412" << endl;
        if (identity == Role::CANDIDATE) identity = Role::LEADER;
        vote_result.clear();
    }
}

bool KVRaftServer::send_append_entries(bool is_heartbeat) {
    Role cur_identity = identity;
    int check_majority = 1;
    int total_server = 1;
    for (const auto& pair : raft_client_stubs_) {
        const std::string cur_server = pair.first;
        std::shared_ptr<KVRaft::Stub> cur_stub_ = pair.second;
        total_server += 1;
        if (is_heartbeat == true) {
            std::cout << "send heartbeat to: " << cur_server << std::endl;
            if (cur_identity != identity) return false;
            // if (cur_identity != get_identity()) return false;
            bool res;
            res = ClientAppendEntries(cur_stub_, logs, is_heartbeat, -1, -1, commit_index, term);
            std::cout << "result for " << cur_server << " is " << res << std::endl;
            if (res) {
                check_alive[cur_server] = true;
                check_majority += 1;
            } else {
                check_alive[cur_server] = false;
                // TODO:: should be delete after complete read persistent log ?
                raft_client_stubs_[cur_server].reset();
                raft_client_stubs_[cur_server] = KVRaft::NewStub(
                    grpc::CreateChannel(cur_server, grpc::InsecureChannelCredentials()));

                next_index[cur_server] = 1;
                match_index[cur_server] = 0;
            }
        }

        else {
            // If last log index ≥ nextIndex for a follower: send AppendEntries
            // RPC with log entries starting at nextIndex
            //   • If successful: update nextIndex and matchIndex for follower
            //   (§5.3) • If AppendEntries fails because of log inconsistency:
            //   decrement nextIndex and retry (§5.3)
            int cur_next_index;
            while ((check_alive[cur_server]) && (logs.getMaxIndex() >= next_index[cur_server])) {
                if (cur_identity != identity) return false;
                // if (cur_identity != get_identity()) return false;
                cur_next_index = next_index[cur_server];
                if (ClientAppendEntries(cur_stub_, logs, is_heartbeat, cur_next_index - 1,
                                        logs.getTermByIndex(cur_next_index - 1), commit_index,
                                        term)) {
                    next_index[cur_server] += 1;   // cur_next_index + 1;
                    match_index[cur_server] += 1;  // cur_next_index;
                } else {
                    next_index[cur_server] -= 1;
                }
            }
            if (check_alive[cur_server]) {
                check_majority += 1;
            }
        }
    }
    if (check_majority * 2 > total_server) {
        return true;
    }
    return false;
}

std::string KVRaftServer::applied_log(int commitable_index) {
    // TODO: current asumption: no delay applied log
    std::string result = "";
    while (last_applied + 1 <= commitable_index) {
        std::string command;
        do {
            command = logs.getCommandByIndex(last_applied + 1);
            //     std::cout << command << std::endl;
        } while (command == INVALID_LOG);
        std::string behavior;
        std::string key;
        std::string val;
        logs.parseCommand(command, behavior, key, val);
        std::cout << "last applied index: " << last_applied + 1 << std::endl;

        std::cout << "command: " << command << " b: " << behavior << " key: " << key
                  << " val: " << val << std::endl;
        if (behavior == "P") {
            state_machine_interface.Put(key, val);
        } else if (behavior == "G") {
            result = state_machine_interface.Get(key);
        } else {
            std::cout << "error 515" << std::endl;
        }
        last_applied++;
    }
    return result;
}

// -------------------------------------------------------------------------------------------------
// EXAMPLE API
Status KVRaftServer::SayHello(ServerContext* context, const HelloRequest* request,
                              HelloReply* reply) {
    std::string prefix("Hello ");
    reply->set_message(prefix + request->name());
    return Status::OK;
}

// -------------------------------------------------------------------------------------------------
// Election timeout API
int KVRaftServer::random_election_timeout() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(MIN_ELECTION_TIMEOUT, MAX_ELECTION_TIMEOUT);
    return dist(gen);
}

// -------------------------------------------------------------------------------------------------
// Election API
void KVRaftServer::start_election() {
    std::cout << "starting election" << std::endl;

    cout << "I'm candidate now 545" << endl;
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
    int max_term = term;
    for (const auto& pair : raft_client_stubs_) {
        if (identity == Role::CANDIDATE) {
            const std::string cur_addr = pair.first;
            std::shared_ptr<KVRaft::Stub> cur_stub_ = pair.second;
            cout << "send ClientRequestVote to " << cur_addr << endl;
            ClientRequestVote(cur_stub_, cur_addr, voted_for, logs.getMaxIndex(),
                              logs.getTermByIndex(logs.getMaxIndex()), max_term);
        }
    }
    term = max_term;
}
// -------------------------------------------------------------------------------------------------
// State machine timeout API
void KVRaftServer::state_machine_loop() {
    std::cout << "State machine thread start" << std::endl;
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(HEARTBEAT_INTERVAL));
        while (commit_index >= last_applied && identity == Role::FOLLOWER) {
            while (logs.getMaxIndex() < commit_index) {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
            applied_log(commit_index);
        }
    }
}

// -------------------------------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "you must provide three arguments: name, addr, config_path" << std::endl;
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