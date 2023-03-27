#pragma once

#include <grpcpp/grpcpp.h>
#include <iostream>
#include <memory>
#include <string>
#include "kvraft.grpc.pb.h"
#include "log.hpp"

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
    KeyValueStoreClient(std::shared_ptr<Channel> channel);

    bool Put(const std::string& key, const std::string& value);

    bool Get(const std::string& key, std::string& result);
   
    std::string SayHello(const std::string& user);

   private:
    std::unique_ptr<KVRaft::Stub> stub_;
};

