#pragma once

#include <grpcpp/grpcpp.h>

#include <memory>
#include <string>
#include <thread>

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

class RaftServer : public Raft::Service {
   public:
    // Constructor, destructor, and other member functions for the Raft node.

    Status RequestVote(ServerContext* context,
                       const RequestVoteRequest* request,
                       RequestVoteResponse* response) override {
        // Process the RequestVote RPC.
    }

    Status AppendEntries(ServerContext* context,
                         const AppendEntriesRequest* request,
                         AppendEntriesResponse* response) override {
        // Process the AppendEntries RPC.
    }
};