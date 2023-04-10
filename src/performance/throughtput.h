#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <random>
#include <sstream>
#include "../kv_grpc_client.h"

using namespace std;

class ThroughputTest {
   public:
    ThroughputTest(std::string config_path, std::string leader_addr, bool test_recovery, int crash_after_n_logs);

    void run_test(int num_iterations, int key_length, int value_length);
    

   private:
    KeyValueStoreClient client_;
    int num_requests_;
    int request_size_;
    bool test_recovery;
    int crash_after_n_logs;
    unordered_map<string, string> test_case;

    std::string random_string(size_t length);
    void run_put_operation(int num_iterations);
    void run_get_operation(int num_iterations);
    void prepare_testcase(int num_iterations, int key_length, int value_length);
    
};