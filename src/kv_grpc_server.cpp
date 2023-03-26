#include "./kv_grpc_server.h"

namespace fs = std::filesystem;
using namespace std;
using namespace raft;

using raft::Raft;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerWriter;
using grpc::Status;  // https://grpc.github.io/grpc/core/md_doc_statuscodes.html

/** mimics `stat`/`lstat` functionality */

KeyValueStoreServer::KeyValueStoreServer(shared_ptr<Role> identity, shared_ptr<vector<RaftClient>> raft_clients):identity(identity), raft_client(raft_client) {
    cout << "KeyValueStoreServer constructor - identity: " << *identity << endl;
}
  
Status KeyValueStoreServer::Put(ServerContext* context, const PutRequest* request, PutResponse* response){
    std::cout << "KeyValueStoreServer::put" << std::endl;
    std::string k(request->key());
    std::string v(request->value());
    if (*identity == Role::LEADER) {
        int cnt = 0;
        for (auto client : *raft_clients) {
            bool ret = client.appendEntries(/* todo */);
            cnt += ret;
        }
        if (cnt >= /*majority*/) {
            return /*ok*/
        } else {
            return /*not ok*/
        }
        

    } else {
        // todo return leader's address
    }

    inmem_store.Put(k, v);
    response->set_success(0);
    
    return Status::OK;
}


Status KeyValueStoreServer::Get(ServerContext* context, const GetRequest* request, GetResponse* response){
    std::cout << "GPRC_Server::get" << std::endl;
    std::string v(inmem_store.Get(request->key()));
    response->set_success(0);
    response->set_value(v);
    return Status::OK;
}

// -------------------------------------------------------------------------------------------------
// EXAMPLE API
Status KeyValueStoreServer::SayHello(ServerContext* context,
                             const HelloRequest* request, HelloReply* reply) {
    std::string prefix("Hello ");
    reply->set_message(prefix + request->name());
    return Status::OK;
}

// void RunServer(std::string name, std::string address) {
//     GRPC_Server service;

//     grpc::EnableDefaultHealthCheckService(true);
//     grpc::reflection::InitProtoReflectionServerBuilderPlugin();
//     ServerBuilder builder;
//     // Listen on the given address without any authentication mechanism.
//     builder.AddListeningPort(address, grpc::InsecureServerCredentials());
//     // Register "service" as the instance through which we'll communicate with
//     // clients. In this case it corresponds to an *synchronous* service.
//     builder.RegisterService(&service);
//     // Finally assemble the server.
//     std::unique_ptr<Server> server(builder.BuildAndStart());

//     // Wait for the server to shutdown. Note that some other thread must be
//     // responsible for shutting down the server for this call to ever return.
//     server->Wait();
// }

