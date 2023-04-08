#include "throughtput.h"

ThroughputTest::ThroughputTest(std::string config_path) : client_(config_path) {}

void ThroughputTest::run_test(int num_iterations, int key_length, int value_length) {
    prepare_testcase(num_iterations, key_length, value_length);
    run_put_operation(num_iterations);
    run_get_operation(num_iterations);
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

    for (const auto& entry : test_case) {
        client_.Put(entry.first, entry.second);
    }

    auto end_time = std::chrono::high_resolution_clock::now();

    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();

    std::cout << "Put operation completed " << num_iterations << " iterations in " << duration
              << " microseconds" << std::endl;
}

// Run the Get operation for a specified number of iterations
void ThroughputTest::run_get_operation(int num_iterations) {
    std::string result;
    auto start_time = std::chrono::high_resolution_clock::now();

    for (const auto& entry : test_case) {
        client_.Get(entry.first, result);
    }

    auto end_time = std::chrono::high_resolution_clock::now();

    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();

    std::cout << "Get operation completed " << num_iterations << " iterations in " << duration
              << " microseconds" << std::endl;
}

int main(int argc, char** argv) {
    if (argc != 5) {
        std::cout
            << "you must provide four arguments: config_path, iteration, key_length, value_length"
            << std::endl;
        std::cout << "Usage: ./throughput ./src/server_config.txt 10000 1000 1000" << std::endl;
        return 0;
    }
    ThroughputTest tt(argv[1]);
    tt.run_test(stoi(argv[2]), stoi(argv[3]), stoi(argv[4]));

    return 0;
}