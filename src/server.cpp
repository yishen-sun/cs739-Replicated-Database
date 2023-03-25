#include "./kv_grpc_server.h"

using namespace std;

#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char** argv) {
    // set defaults
   
    
    // set configs from arguments
    if (argc != 3)  {
        std::cout << "Usage ./server <name> <address>" << std::endl;
        return 0;
    }

    // RunServer(argv[1], argv[2]);

    return 0;
}
// int main(int argc, char** argv) {
//     std::thread raft_thread(RunRaftServer);
//     raft_thread.join();
//     std::thread kv_thread(RunKVServer);
//     kv_thread.join();

//     return 0;
// }