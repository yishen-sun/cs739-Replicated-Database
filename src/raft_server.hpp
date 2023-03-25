#include "./kv_grpc_server.h"
#include "./raft_grpc_client.hpp"
#include "./raft_grpc_server.hpp"
#include "./log.h"
#include "./key_value_store.hpp"

class Server {
    public:
    // intialize: activate raft_grpc_service / actiave kv_grpc_service
        void run();
        // heartbeat->identity=leader
    private:
        std::shared_ptr<Log> logs;
        std::shared_ptr<KeyValueStoreServer> kv_server;
        std::shared_ptr<RaftServer> raft_server;
        std::shared_ptr<KeyValueStore> server_config;
        std::shared_ptr<int> identity; // 0 = leader, 1 = candidate, 2 = follower

};