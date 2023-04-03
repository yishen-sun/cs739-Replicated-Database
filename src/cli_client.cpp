#include "./kv_grpc_client.h"
#include <sstream>

using namespace std;

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "you must provide three arguments: addr" << std::endl;
        std::cout << "For example: 0.0.0.0:50001" << std::endl;
        return 0;
    }
    string target_str = argv[1];
    KeyValueStoreClient kv(target_str);
    // std::string reply;  // = kv.SayHello(user);
    // bool success;
    // success = kv.Get("test1", reply);
    // std::cout << "expected: , actual: " << reply << std::endl;

    // success = kv.Get("test2", reply);
    // std::cout << "expected: , actual: " << reply << std::endl;

    // success = kv.Get("test3", reply);
    // std::cout << "expected: , actual: " << reply << std::endl;

    // success = kv.Put("test1", "test_reply");

    // success = kv.Put("test2", "test2_reply");

    // success = kv.Put("test3", "test3_reply");

    // std::cout << "Put done" << std::endl;

    // success = kv.Get("test1", reply);
    // std::cout << "expected: test_reply, actual: " << reply << std::endl;
    // success = kv.Get("test2", reply);
    // std::cout << "expected: test2_reply, actual: " << reply << std::endl;
    // success = kv.Get("test3", reply);
    // std::cout << "expected: test3_reply, actual: " << reply << std::endl;

    // success = kv.Put("test4", "test4_reply");

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
                std::cout << "Get successful. Result: " << result << std::endl;
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
