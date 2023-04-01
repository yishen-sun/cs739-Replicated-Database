#ifndef KEY_VALUE_STORE_H_
#define KEY_VALUE_STORE_H_

#include <cpp_redis/cpp_redis>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>

using namespace std;

class KeyValueStore {
   public:
    KeyValueStore(std::string fname);
    bool Put(const std::string& key, const std::string& value);
    std::string Get(const std::string& key);
    bool Delete(const std::string& key);

   private:
    std::string filename;
#ifndef KV_REDIS_
    std::unordered_map<std::string, std::string> store_;
    void writeToDisk();
    std::mutex kv_mutex;
#else
    cpp_redis::client client;
#endif
};

#endif  // KEY_VALUE_STORE_H_