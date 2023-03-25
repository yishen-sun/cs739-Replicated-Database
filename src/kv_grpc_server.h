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

#include "./key_value_store.h"
#include "raft.grpc.pb.h"
#include "./key_value_store.hpp"
#include "./log.hpp"
#include "./kv_grpc_client.h"

#define CHUNK_SIZE 1572864

namespace fs = std::filesystem;
using namespace std;
using namespace grpc;
using namespace raft;

// Logic and data behind the server's behavior.
class KeyValueStoreServer final : public Raft::Service {
   public:
    Status Put(ServerContext* context, const PutRequest* request, PutResponse* response) override;

    Status Get(ServerContext* context, const GetRequest* request, GetResponse* response) override;
   
    Status SayHello(ServerContext* context, const HelloRequest* request,
                    HelloReply* reply) override;
   private:
      std::shared_ptr<int> identity;
      std::shared_ptr<KeyValueStore> server_config;
      // std::shared_ptr<Vector<raft_client>> raft_clients;   // grpc client to send raft info to other server.
};

