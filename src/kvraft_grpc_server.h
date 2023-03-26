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

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "./key_value_store.h"
#include "./key_value_store.hpp"
#include "./kv_grpc_client.h"
#include "./log.hpp"
#include "./raft_grpc_client.hpp"
#include "raft.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using raft::AppendEntriesRequest;
using raft::AppendEntriesResponse;
using raft::Raft;
using raft::RequestVoteRequest;
using raft::RequestVoteResponse;

#define CHUNK_SIZE 1572864
using namespace std;
using namespace grpc;
using namespace raft;
enum Role { LEADER, FOLLOWER, CANDIDATE };

class KVRaftServer : public KVRaft::Service {
   public:
    // Constructor, destructor, and other member functions for the Raft node.
    KVRaftServer();

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

   private:
    shared_ptr<Role> identity;
    // shared_ptr<KeyValueStore> server_config; //{XXX: 0.0.0.0:50001};
    shared_ptr<vector<RaftClient>>
        raft_clients;  // grpc client to send raft info to other server.
};
