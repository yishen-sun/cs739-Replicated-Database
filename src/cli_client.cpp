#include <grpcpp/grpcpp.h>

#include "./kv_grpc_client.h"

using namespace std;

int main(int argc, char** argv) {
    std::string target_str("0.0.0.0:50001");
    KeyValueStoreClient kv(
        grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
    std::string user("world");
    std::string reply = kv.SayHello(user);
    std::cout << "Greeter received: " << reply << std::endl;

    // kv.Get("NOOOO_value", reply);
    // std::cout << "expected: None, actual: " << reply << std::endl;

    kv.Put("test1", "test_reply");
    kv.Put("test2", "test2_reply");
    kv.Put("test3", "test3_reply");
    std::cout << "Put done" << std::endl;
    kv.Get("test1", reply);
    std::cout << "expected: test_reply, actual: " << reply << std::endl;
    kv.Get("test2", reply);
    std::cout << "expected: test2_reply, actual: " << reply << std::endl;
    kv.Get("test3", reply);
    std::cout << "expected: test3_reply, actual: " << reply << std::endl;

    return 0;
}
