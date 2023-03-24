#include <grpcpp/grpcpp.h>

#include <iostream>
#include <memory>
#include <string>

#include "raft.grpc.pb.h"

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

std::string NO_MASTER_YET = "NO_MASTER";

class KeyValueStoreClient {
   public:
    KeyValueStoreClient(std::shared_ptr<Channel> channel)
        : stub_(Raft::NewStub(channel)) {}

    bool Put(const std::string& key, const std::string& value) {
        PutRequest request;
        request.set_key(key);
        request.set_value(value);

        PutResponse response;
        ClientContext context;

        Status status;
        while (true) {
            status = stub_->Put(&context, request, &response);
            if (status.ok()) {
                if (response.success() == 0) return true;
                if (response.master_addr() == NO_MASTER_YET) {
                    // sleep and retries
                    sleep(0.2);
                } else {
                    // update stub channel
                    stub_.release();
                    std::string master_addr = response.master_addr();
                    stub_ = Raft::NewStub(grpc::CreateChannel(master_addr, grpc::InsecureChannelCredentials()));
                }
            } else {
                return false;
            }
        }
    }

    bool Get(const std::string& key, std::string& result) {
        GetRequest request;
        request.set_key(key);

        GetResponse response;
        ClientContext context;

        Status status;
        while (true) {
            status = stub_->Get(&context, request, &response);
            if (status.ok()) {
                if (response.success() == 0) {
                    result = response.value();
                    return true;
                }
                if (response.master_addr() == NO_MASTER_YET) {
                    // sleep and retries
                    sleep(0.2);
                } else {
                    // update stub channel
                    stub_.release();
                    std::string master_addr = response.master_addr();
                    stub_ = Raft::NewStub(grpc::CreateChannel(master_addr, grpc::InsecureChannelCredentials()));
                }
            } else {
                return false;
            }
        }

    }

    std::string SayHello(const std::string& user) {
        HelloRequest request;
        request.set_name(user);

        // Container for the data we expect from the server.
        HelloReply reply;

        // Context for the client. It could be used to convey extra information to
        // the server and/or tweak certain RPC behaviors.
        ClientContext context;

        // The actual RPC.
        Status status = stub_->SayHello(&context, request, &reply);

        // Act upon its status.
        if (status.ok()) {
            return reply.message();
        } else {
            //std::cout << status.error_code() << ": " << status.error_message() << std::endl;
            return "RPC failed";
        }
    }

   private:
    std::unique_ptr<Raft::Stub> stub_;
};

int main(int argc, char** argv) {
  std::string target_str("0.0.0.0:50051");
  KeyValueStoreClient kv(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
  std::string user("world");
  std::string reply = kv.SayHello(user);
  std::cout << "Greeter received: " << reply << std::endl;

  kv.Get("NOOOO_value", reply);
  std::cout << "expected: None, actual: " << reply << std::endl;

  kv.Put("test1", "test_reply");
  kv.Get("test1", reply);
  std::cout << "expected: test_reply, actual: " << reply << std::endl;

  return 0;
}
