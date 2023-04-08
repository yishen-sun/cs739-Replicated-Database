#ifndef KEY_VALUE_STORE_H_
#define KEY_VALUE_STORE_H_

#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>

using namespace std;

class KeyValueStore {
   public:
    virtual bool Put(const std::string& key, const std::string& value) = 0;
    virtual std::string Get(const std::string& key) = 0;
    virtual bool Delete(const std::string& key) = 0;
};

class BasicKeyValueStore : public KeyValueStore {
   public:
    BasicKeyValueStore(std::string fname);
    bool Put(const std::string& key, const std::string& value);
    std::string Get(const std::string& key);
    bool Delete(const std::string& key);

   private:
    std::string filename;
    std::unordered_map<std::string, std::string> store_;
    void writeToDisk();
    std::mutex kv_mutex;
};

#ifdef USE_REDIS
#include <cpp_redis/cpp_redis>

class RedisKeyValueStore : public KeyValueStore {
   public:
    RedisKeyValueStore();
    bool Put(const std::string& key, const std::string& value);
    std::string Get(const std::string& key);
    bool Delete(const std::string& key);

   private:
    cpp_redis::client client;
};
#endif

#endif  // KEY_VALUE_STORE_H_