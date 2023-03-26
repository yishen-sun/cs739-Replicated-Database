#include "raft_server.hpp"

#include <fstream>
#include <sstream>



Server::Server(std::string name, std::sting addr, std::string config_path):
        name(name), addr(addr), config_path(config_path){
    identity = make_shared<Role>(Role::FOLLOWER);
    logs = new Log(); 
    server_config = new KeyValueStore();
    read_server_config(server_config);
    update_channel();
    kv_server = new KeyValueStoreServer(identity=identity, raft_clients=raft_clients); // assign port name, raft_clients
    raft_server = new RaftServer(); //assign port
}

Server::heartbeat() {
    // RaftClient rc;
    // for (int i = 0; i < raft_clients.size(); i++) {
    //     rc = raft_clients[i];
    //     rc.AppendEntries();
    // }
    std::thread([this]() {
    while (true) {
      if (identity == Role::LEADER) {
        raft_->SendHeartbeats();
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(HEARTBEAT_INTERVAL_MS));
    }
  }).detach();
}


Server::run() {
    ServerBuilder builder;
    builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
    builder.RegisterService(&raft_server);
    builder.RegisterService(&kv_server)
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Raft server listening on " << server_address << std::endl;
    server->Wait();

    while true {
        if (identity == Role::LEADER) {
            // send heartbeat
            std::cout << "I am leader: send heartbeat" << std::endl;
        } else if (identity == Role::FOLLOWER) {
            std::cout << "I am follower: wait for heartbeat" << std::endl;
            // wait for heartbeat
            // if no heartbeat -> wait for random minutes -> become candidate
        } else if (identity == Role::CANDIDATE) {
            std::cout << "I am Candidate: start election" << std::endl;
            // start election
        }
    }
}

bool Server::read_server_config(std::string config_path, shared_ptr<KeyValueStore>& server_config) {
    std::ifstream infile(config_path);
    std::string line;
    while (std::getline(infile, line)) {
        size_t pos = line.find('/');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1, line.size() - pos - 1);
            server_config->Put(key, value);
        }
    }
    return true;
}

Server::update_channel() {
    for (const auto& pair : *server_config) {
        const std::string& cur_name = pair.first;
        const std::string& cur_addr = pair.second;
        if (cur_name != name) {
            raft_clients->push_back(new RaftClient(addr));
        }
    }
}