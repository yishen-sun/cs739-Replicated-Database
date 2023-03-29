#include "kv_grpc_client.h"

std::string NO_MASTER_YET = "NO_MASTER_YET";

KeyValueStoreClient::KeyValueStoreClient(std::string target_str){
        grpc::ChannelArguments channel_args;
        channel_args.SetInt(GRPC_ARG_MAX_RECEIVE_MESSAGE_LENGTH, INT_MAX);
        channel_ = grpc::CreateCustomChannel(target_str, grpc::InsecureChannelCredentials(), channel_args);
        stub_ = KVRaft::NewStub(channel_);
    }

bool KeyValueStoreClient::Put(const std::string& key, const std::string& value) {

    while (true) {
        PutRequest request;
        request.set_key(key);
        request.set_value(value);

        PutResponse response;
        
        Status status;
    
        ClientContext context;
        status = stub_->Put(&context, request, &response);
        if (status.ok()) {
            if (response.success() == 0) return true;
            if (response.master_addr() == NO_MASTER_YET) {
                // sleep and retries
                std::cout << "NO_MASTER_YET" << std::endl;
                sleep(0.2);
            } else {
                // update stub channel
                std::cout << "update stub channel" << std::endl;
                stub_.release();
                std::string master_addr = response.master_addr();
                std::cout << master_addr << std::endl;
                stub_ = KVRaft::NewStub(grpc::CreateChannel(master_addr, grpc::InsecureChannelCredentials()));
            }
        } else {
            return false;
        }
    }
}

bool KeyValueStoreClient::Get(const std::string& key, std::string& result) {
    while (true) {
        GetRequest request;
        request.set_key(key);
        
        GetResponse response;
        
        Status status;
        ClientContext context;
        status = stub_->Get(&context, request, &response);
        if (status.ok()) {
            if (response.success() == 0) {
                result = response.value();
                return true;
            }
            if (response.master_addr() == NO_MASTER_YET) {
                // sleep and retries
                std::cout << "NO_MASTER_YET" << std::endl;
                sleep(0.2);
            } else {
                std::cout << "update stub channel" << std::endl;
                std::string master_addr = response.master_addr();
                std::cout << master_addr << std::endl;
                stub_ = KVRaft::NewStub(grpc::CreateChannel(master_addr, grpc::InsecureChannelCredentials()));
            }
        } else {
            return false;
        }
    }

}


std::string KeyValueStoreClient::SayHello(const std::string& user) {
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
        // std::cout << status.error_code() << ": " << status.error_message() << std::endl;
        return "RPC failed";
    }
}