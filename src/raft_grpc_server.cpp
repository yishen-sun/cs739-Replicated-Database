#include <grpcpp/grpcpp.h>

#include <memory>
#include <string>
#include <thread>

#include "raft.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using raft::AppendEntriesRequest;
using raft::AppendEntriesResponse;
using raft::Raft;
using raft::RequestVoteRequest;
using raft::RequestVoteResponse;

class RaftNode : public Raft::Service {
   public:
    // Constructor, destructor, and other member functions for the Raft node.

    Status RequestVote(ServerContext* context,
                       const RequestVoteRequest* request,
                       RequestVoteResponse* response) override {
        // Process the RequestVote RPC.
    }

    Status AppendEntries(ServerContext* context,
                         const AppendEntriesRequest* request,
                         AppendEntriesResponse* response) override {
        // Process the AppendEntries RPC.
    }
};

void RunRaftServer() {
    std::string server_address("0.0.0.0:50051");
    RaftNode raft_node;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&raft_node);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Raft server listening on " << server_address << std::endl;
    server->Wait();
}

int main(int argc, char** argv) {
    std::thread server_thread(RunRaftServer);
    server_thread.join();

    return 0;
}
