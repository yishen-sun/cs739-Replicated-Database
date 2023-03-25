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
#include <vector>

#include "raft.grpc.pb.h"
#include "./log.hpp"

#define CHUNK_SIZE 1572864

namespace fs = std::filesystem;
using namespace std;
using namespace grpc;
using namespace raft;
using raft::HelloRequest;
using raft::HelloReply;

// Logic and data behind the server's behavior.
class RaftClient final : public Raft::Service {
   public:
    // rpc RequestVote (RequestVoteRequest) returns (RequestVoteResponse);
    bool RequestVote(const int term, const int candidate_id, const int last_log_index, const int last_log_term);

    // rpc AppendEntries (AppendEntriesRequest) returns (AppendEntriesResponse);
    bool AppendEntries(const int term, const int leader_id, const int prev_log_index, const int prev_log_term, Log log_entries, const int leader_commit);

    std::string SayHello(const std::string& user);

   private:
    std::unique_ptr<Raft::Stub> stub_;
};
