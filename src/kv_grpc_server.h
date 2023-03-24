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
#include "./key_value_store.hpp"

#define CHUNK_SIZE 1572864

namespace fs = std::filesystem;
using namespace std;
using namespace grpc;
using namespace raft;

// Logic and data behind the server's behavior.
class GRPC_Server final : public Raft::Service {
   public:
   //  rpc RequestVote (RequestVoteRequest) returns (RequestVoteResponse);
   //  rpc AppendEntries (AppendEntriesRequest) returns (AppendEntriesResponse);
    Status Put(ServerContext* context, const PutRequest* request, PutResponse* response) override;
    Status Get(ServerContext* context, const GetRequest* request, GetResponse* response) override;
    // Status readDirectory(ServerContext* context, const Path* request,
    //                      ServerWriter<afs::ReadDirResponse>* writer)
    //                      override;
    // Status createDirectory(ServerContext* context, const MkDirRequest*
    // request,
    //                        Response* response) override;
    // Status removeDirectory(ServerContext* context, const Path* request,
    //                        Response* response) override;
    // Status removeFile(ServerContext* context, const Path* request,
    //                   Response* response) override;
    // Status getFileAttributes(ServerContext* context, const Path* request,
    //                          /*char* string*/ Attributes* response) override;
    // Status createEmptyFile(ServerContext* context, const OpenRequest*
    // request,
    //                        OpenResponse* response) override;
    // Status getFileContents(ServerContext* context, const ReadRequest*
    // request,
    //                        ServerWriter<ReadReply>* writer) override;
    // Status putFileContents(ServerContext* context,
    //                        ServerReader<WriteRequest>* reader,
    //                        WriteReply* reply) override;
    Status SayHello(ServerContext* context, const HelloRequest* request,
                    HelloReply* reply) override;
   private:
      KeyValueStore inmem_store;
};