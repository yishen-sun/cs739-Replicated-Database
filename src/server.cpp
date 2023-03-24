<<<<<<< HEAD
#include "./kv_grpc_server.h"

using namespace std;
=======
#include <thread>
>>>>>>> bfbe7f7 (basic skeleton 1)

int main(int argc, char** argv) {

    // set defaults
    const std::string address("0.0.0.0:50051");
    
    // set configs from arguments
    // if (argc == 2) serverDirectory = argv[1];

    // std::cout << "serverDirectory: " << serverDirectory << std::endl;

    RunServer(address);

    return 0;
}
// int main(int argc, char** argv) {
//     std::thread raft_thread(RunRaftServer);
//     raft_thread.join();
//     std::thread kv_thread(RunKVServer);
//     kv_thread.join();

//     return 0;
// }