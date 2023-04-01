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
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#define KV_REDIS_

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
// TODO: too short -> start election before receive first heartbeat
constexpr int MIN_ELECTION_TIMEOUT = 1200;
constexpr int MAX_ELECTION_TIMEOUT = 2000;
bool test_without_election = false;
class KVRaftServer final : public KVRaft::Service {
   public:
    // Constructor
    KVRaftServer(std::string name, std::string addr, std::string config_path);
    //
    void RunServer();
    // grpc server part
    Status RequestVote(ServerContext* context,
                       const RequestVoteRequest* request,
                       RequestVoteResponse* response) override;

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
    bool ClientRequestVote(shared_ptr<KVRaft::Stub> stub_,
                           const std::string receive_name,
                           const string candidate_name,
                           const int last_log_index, const int last_log_term);

    bool ClientAppendEntries(shared_ptr<KVRaft::Stub> stub_, Log log_entries,
                             bool is_heartbeat, int prev_log_index,
                             int prev_log_term, int commit_index, int msg_term);

    std::string ClientSayHello(const std::string& user);
    // server thread
    void server_loop();
    // server function
    void send_append_entries(bool is_heartbeat);
    void start_election();
    void check_vote();

    std::string applied_log();

   private:
    Role identity;

    KeyValueStore state_machine_interface;
    unordered_map<std::string, std::shared_ptr<KVRaft::Stub>>
        raft_client_stubs_;  // k = addr:port, v = stub_
    unordered_map<std::string, std::string>
        server_config;  // k = name A, v = addr:port 0.0.0.0:50001

    unordered_map<std::string, bool> vote_result;  // k = name, v = vote or not;

    string config_path;
    string name;
    string addr;
    // Timeout
    std::chrono::time_point<std::chrono::high_resolution_clock> election_timer;
    // persistent state on servers
    int term;                            // currentTerm
    string voted_for;                    // TODO: persistent state
    KeyValueStore persistent_voted_for;  // key = term, v = vote for addr
    Log logs;

    string leader_addr;

    // volatile state on servers
    int commit_index;
    int last_applied;
    bool can_vote;

    // volatile state on leaders
    unordered_map<std::string, int>
        next_index;  // unordered_map<std::string addr:port , int index of next
                     // log send to that server> for each server, index of the
                     // next log entry to send to that server (initialized to
                     // leader last log index + 1)

    unordered_map<std::string, int>
        match_index;  // unordered_map<std::string addr:port , for each server,
                      // index of highest log entry known to be replicated on
                      // server (initialized to 0, increases monotonically)

    bool read_server_config();
    bool read_state_machine_config(std::string filename);
    bool read_voted_for_config(std::string filename);
    bool send_heartbeat();
    bool update_stubs_();
    // heartbeats and election threads functions
    void leader_heartbeat_loop();
    int random_election_timeout();
    void election_timer_loop();
    void state_machine_loop();
    // void change_identity(Role new_role, Role previous_role);
    // Role get_identity();
    // void set_identity(Role new_role);
    std::mutex role_mutex;
};