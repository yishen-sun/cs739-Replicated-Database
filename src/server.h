#include <fstream>
#include <sstream>

#include "./kvraft_grpc_server.h"
#include "./raft_grpc_server.hpp"

enum Role { LEADER, FOLLOWER, CANDIDATE };  // Role::LEADER
class Server {
   public:
    Server();
    void run();

   private:
    std::shared_ptr<Log> logs;
    std::shared_ptr<KeyValueStoreServer> kv_server;
    std::shared_ptr<RaftServer> raft_server;
    std::shared_ptr<vector<RaftClient>> raft_clients;
    std::shared_ptr<KeyValueStore> server_config;
    std::shared_ptr<Role> identity;  // 0 = leader, 1 = candidate, 2 = follower
    std::string config_path;
    std::string name;
    std::string addr;
    bool read_server_config(std::string config_path,
                            std::shared_ptr<KeyValueStore>& server_config);
    void send_heartbeat();
    void update_channel()
};