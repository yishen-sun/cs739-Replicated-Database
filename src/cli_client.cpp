#include <sstream>

#include "./kv_grpc_client.h"

using namespace std;

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "you must provide one arguments: config_path assigned_port" << std::endl;
        std::cout << "Usage: ./cli_client ./src/server_config.txt 0.0.0.0:50001" << std::endl;
        return 0;
    }
    string config_path = argv[1];
    string assigned_port = argv[2];
    KeyValueStoreClient kv(config_path, assigned_port);

    std::string input;
    std::string key, value, result;
    while (true) {
        std::cout << "Enter command (put/get): ";
        std::getline(std::cin, input);
        std::istringstream iss(input);
        std::string command;
        iss >> command;
        if (command == "put") {
            iss >> key >> value;
            if (kv.Put(key, value)) {
                std::cout << "Put successful." << std::endl;
            } else {
                std::cout << "Put failed." << std::endl;
            }
        } else if (command == "get") {
            iss >> key;
            if (kv.Get(key, result)) {
                if (result.size() == 0) {
                    std::cout << "key doesn't exist" << std::endl;
                } else {
                    std::cout << "Get successful. Result: " << result << std::endl;
                }
            } else {
                std::cout << "Get failed." << std::endl;
            }
        } else if (command == "hello") {
            iss >> value;
            std::cout << kv.SayHello(value) << std::endl;
        } else if (command == "exit") {
            break;
        } else {
            std::cout << "Invalid command." << std::endl;
        }
    }
    return 0;
}
