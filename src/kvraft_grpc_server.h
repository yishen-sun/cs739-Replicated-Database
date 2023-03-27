#pragma once

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <random>

#include "./key_value_store.h"
#include "./log.hpp"
#include "kvraft.grpc.pb.h"

using grpc::ClientContext;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerWriter;
using grpc::Status;

using kvraft::AppendEntriesRequest;
using kvraft::AppendEntriesResponse;
using kvraft::RequestVoteRequest;
using kvraft::RequestVoteResponse;

using namespace std;
using namespace grpc;
using namespace kvraft;
using namespace std::chrono_literals;

enum Role { LEADER, FOLLOWER, CANDIDATE };
constexpr int HEARTBEAT_INTERVAL = 50;
constexpr int MIN_ELECTION_TIMEOUT = 150;
constexpr int MAX_ELECTION_TIMEOUT = 300;
bool test_without_election = true;
class KVRaftServer final : public KVRaft::Service {
   public:
    // Constructor
    KVRaftServer(std::string name, std::string addr,
                           std::string config_path);
    //
    void RunServer();
    // grpc server part
    // Status RequestVote(ServerContext* context,
    //                    const RequestVoteRequest* request,
    //                    RequestVoteResponse* response) override;

    Status AppendEntries(ServerContext* context,
                         const AppendEntriesRequest* request,
                         AppendEntriesResponse* response) override;

    Status Put(ServerContext* context, const PutRequest* request,
               PutResponse* response) override;

    Status Get(ServerContext* context, const GetRequest* request,
               GetResponse* response) override;

    Status SayHello(ServerContext* context, const HelloRequest* request,
                    HelloReply* reply) override;

    // grpc client part
    // bool ClientRequestVote(const int candidate_id, const int last_log_index,
    // const int last_log_term);

    bool ClientAppendEntries(shared_ptr<KVRaft::Stub> stub_, Log log_entries,
                             bool is_heartbeat, int prev_log_index,
                             int prev_log_term, int commit_index, int msg_term);

    std::string ClientSayHello(const std::string& user);
    // server thread
    void server_loop();
    // server function
    void send_append_entries(bool is_heartbeat);
    void start_election();

    std::string applied_log();

   private:
    Role identity;

    KeyValueStore state_machine_interface;
    unordered_map<std::string, std::shared_ptr<KVRaft::Stub>>
        raft_client_stubs_;       // k = addr:port, v = stub_
    unordered_map<std::string, std::string> server_config;  // k = name A, v = addr:port 0.0.0.0:50001

    string config_path;
    string name;
    string addr;
    // Timeout
    std::chrono::time_point<std::chrono::high_resolution_clock> last_receive_appendEntries_time;
    // persistent state on servers
    int term;       // currentTerm
    int leader_id;  // votedFor? currentLeader?
    Log logs;

    // volatile state on servers
    int commit_index;
    int last_applied;

    // volatile state on leaders
    unordered_map<std::string, int>
        next_index;  // unordered_map<std::string addr:port , int index of next
                     // log send to that server> for each server, index of the
                     // next log entry to send to that server (initialized to
                     // leader last log index + 1)

    unordered_map<std::string, int>
        match_index;  // unordered_map<std::string addr:port , for each server, 
                      // index of highest log entry known to be replicated on server
                      // (initialized to 0, increases monotonically)

    bool read_server_config();
    bool send_heartbeat();
    bool update_stubs_();
    // heartbeats and election threads functions
    void leader_heartbeat_loop();
    int random_election_timeout();
    void election_timer_loop();
};
