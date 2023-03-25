#include "raft_grpc_client.hpp"

std::string RaftClient::SayHello(const std::string& user) {
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