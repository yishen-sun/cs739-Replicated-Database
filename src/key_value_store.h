#ifndef KEY_VALUE_STORE_H_
#define KEY_VALUE_STORE_H_

#include <string>
#include <unordered_map>
#include <fstream>
#include <mutex>
#include <iostream>

using namespace std;


class KeyValueStore {
   public:
    KeyValueStore(std::string fname);
    bool Put(const std::string& key, const std::string& value);
    std::string Get(const std::string& key);
    bool Delete(const std::string& key);    

   private:
    std::unordered_map<std::string, std::string> store_;
    void writeToDisk();
    std::string filename;
    std::mutex kv_mutex;
};

#endif // KEY_VALUE_STORE_H_