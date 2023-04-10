#include "throughtput.h"

ThroughputTest::ThroughputTest(std::string config_path, std::string leader_addr, bool test_recovery, int crash_after_n_logs): 
        client_(config_path, leader_addr), test_recovery(test_recovery), crash_after_n_logs(crash_after_n_logs) {

}

void ThroughputTest::run_test(int num_iterations, int key_length, int value_length) {
    prepare_testcase(num_iterations, key_length, value_length);
    if (test_recovery) {
        run_put_operation(num_iterations);
    } else {
        run_put_operation(num_iterations);
        run_get_operation(num_iterations);
    }
}

// Create a random string of the specified length
std::string ThroughputTest::random_string(size_t length) {
    static const std::string CHARACTERS =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    static std::random_device rd;
    static std::mt19937 generator(rd());

    std::uniform_int_distribution<> distribution(0, CHARACTERS.size() - 1);

    std::string random_string;

    for (size_t i = 0; i < length; ++i) {
        random_string += CHARACTERS[distribution(generator)];
    }

    return random_string;
}

void ThroughputTest::prepare_testcase(int num_iterations, int key_length, int value_length) {
    std::string key;
    std::string value;
    while (test_case.size() < num_iterations) {
        key = random_string(key_length);
        value = random_string(value_length);
        test_case[key] = value;
    }
}

// Run the Put operation for a specified number of iterations
void ThroughputTest::run_put_operation(int num_iterations) {
    auto start_time = std::chrono::high_resolution_clock::now();
    bool res = true;
    int i = 0;
    for (const auto& entry : test_case) {
        i++;
        auto before_crash_time = std::chrono::high_resolution_clock::now();
        res = (client_.Put(entry.first, entry.second) || res);
        auto after_crash_time = std::chrono::high_resolution_clock::now();
        if (i == crash_after_n_logs) {
            std::cout << "Put operation time with server reelection is" 
                    << std::chrono::duration_cast<std::chrono::microseconds>(after_crash_time - before_crash_time).count() 
                    << std::endl;
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    if (res == false) {
        std::cout << "Failed to execute commands" << std::endl;
        exit(0);
    }

    std::cout << "Put operation completed:" << num_iterations << " kv pairs in " << duration << " microseconds" << std::endl;
}

// Run the Get operation for a specified number of iterations
void ThroughputTest::run_get_operation(int num_iterations) {
    std::string result;
    auto start_time = std::chrono::high_resolution_clock::now();
    bool res = true;
    for (const auto& entry : test_case) {
        res = (client_.Get(entry.first, result) || res);
    }

    auto end_time = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    
    if (res == false) {
        std::cout << "Failed to execute commands" << std::endl;
        exit(0);
    }
    std::cout << "Get operation completed " << num_iterations << " kv pairs in " << duration << " microseconds" << std::endl;
}

int main(int argc, char** argv) {
    if (argc != 8) {
        std::cout << "you must provide four arguments: config_path server_addr, iteration, key_length, value_length, test_recovery, aftet_n_log_crash"
                  << std::endl;
        std::cout << "Usage: ./throughput one_server_config.txt 0.0.0.0:50001 10000 1000 1000 false 500" << std::endl;
        return 0;
    }
    bool test_recovery;
    std::istringstream(argv[6]) >> std::boolalpha >> test_recovery;

    ThroughputTest tt(argv[1], argv[2], test_recovery, stoi(argv[7]));
    tt.run_test(stoi(argv[3]), stoi(argv[4]), stoi(argv[5]));

    return 0;
}