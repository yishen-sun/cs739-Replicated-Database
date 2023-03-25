#pragma once

#include <grpcpp/grpcpp.h>
#include <iostream>
#include <memory>
#include <string>
#include "raft.grpc.pb.h"
#include "log.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using raft::Raft;
using raft::GetRequest;
using raft::GetResponse;
using raft::PutRequest;
using raft::PutResponse;
using raft::HelloRequest;
using raft::HelloReply;

class KeyValueStoreClient {
   public:
    KeyValueStoreClient(std::shared_ptr<Channel> channel);

    bool Put(const std::string& key, const std::string& value);

    bool Get(const std::string& key, std::string& result);
   
    std::string SayHello(const std::string& user);

   private:
    std::unique_ptr<Raft::Stub> stub_;
};

