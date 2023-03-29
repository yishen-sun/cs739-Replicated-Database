#include "./kv_grpc_client.h"

using namespace std;

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "you must provide three arguments: addr"
                  << std::endl;
        std::cout << "For example: 0.0.0.0:50001" << std::endl;
        return 0;
    }
    string target_str = argv[1];
    KeyValueStoreClient kv(target_str);
    std::string reply;// = kv.SayHello(user);
    bool success;
    success = kv.Put("test1", "test_reply");
    std::cout << "success 1: " << success << std::endl;
    
    success = kv.Put("test2", "test2_reply");
    std::cout << "success 2: " << success << std::endl;
    
    success = kv.Put("test3", "test3_reply");
    std::cout << "success 3: " << success << std::endl;
    std::cout << "Put done" << std::endl;

    success = kv.Get("test1", reply);
    std::cout << "success 4: " << success << std::endl;
    std::cout << "expected: test_reply, actual: " << reply << std::endl;
    success = kv.Get("test2", reply);
    std::cout << "success 5: " << success << std::endl;
    std::cout << "expected: test2_reply, actual: " << reply << std::endl;
    success = kv.Get("test3", reply);
    std::cout << "success 6: " << success << std::endl;
    std::cout << "expected: test3_reply, actual: " << reply << std::endl;

    return 0;
}
