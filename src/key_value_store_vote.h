#ifndef KEY_VALUE_STORE_VOTE_H_
#define KEY_VALUE_STORE_VOTE_H_

#include <cpp_redis/cpp_redis>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>

using namespace std;

class KeyValueStoreVote {
   public:
    KeyValueStoreVote(std::string fname);
    bool Put(const std::string& key, const std::string& value);
    std::string Get(const std::string& key);
    bool Delete(const std::string& key);

   private:
    std::string filename;
    std::unordered_map<std::string, std::string> store_;
    void writeToDisk();
    std::mutex kv_mutex;
};

#endif  // KEY_VALUE_STOR_VOTE_H_