#pragma once

#include <grpcpp/grpcpp.h>
#include <iostream>
#include <memory>
#include <string>
#include "kvraft.grpc.pb.h"
#include "log.hpp"
#include <random>
#include <thread>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using kvraft::KVRaft;
using kvraft::GetRequest;
using kvraft::GetResponse;
using kvraft::PutRequest;
using kvraft::PutResponse;
using kvraft::HelloRequest;
using kvraft::HelloReply;

class KeyValueStoreClient {
   public:
    KeyValueStoreClient(std::string config_path);

    bool Put(const std::string& key, const std::string& value);

    bool Get(const std::string& key, std::string& result);
   
    std::string SayHello(const std::string& user);

   private:
    std::unique_ptr<KVRaft::Stub> stub_;
    std::shared_ptr<Channel> channel_;
    std::string config_path;
    unordered_map<std::string, std::string>
        server_config; // k = name A, v = addr:port 0.0.0.0:50001
    bool read_server_config();
    void random_pick_server();
    int rand_between(int start, int end);
};

