#include <grpcpp/grpcpp.h>

#include <iostream>
#include <memory>
#include <string>

#include "key_value_store.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using key_value_store::GetRequest;
using key_value_store::GetResponse;
using key_value_store::KeyValueStore;
using key_value_store::PutRequest;
using key_value_store::PutResponse;

class KeyValueStoreClient {
   public:
    KeyValueStoreClient(std::shared_ptr<Channel> channel)
        : stub_(KeyValueStore::NewStub(channel)) {}

    bool Put(const std::string& key, const std::string& value) {
        PutRequest request;
        request.set_key(key);
        request.set_value(value);

        PutResponse response;
        ClientContext context;

        Status status = stub_->Put(&context, request, &response);
        if (status.ok()) {
            return response.success();
        } else {
            return false;
        }
    }

    std::string Get(const std::string& key) {
        GetRequest request;
        request.set_key(key);

        GetResponse response;
        ClientContext context;

        Status status = stub_->Get(&context, request, &response);
        if (status.ok()) {
            return response.value();
        } else {
            return "";
        }
    }

   private:
    std::unique_ptr<KeyValueStore::Stub> stub_;
};
