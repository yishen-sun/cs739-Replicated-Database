#ifndef KEY_VALUE_STORE_H_
#define KEY_VALUE_STORE_H_

#include <string>
#include <unordered_map>

class KeyValueStore {
   public:
    bool Put(const std::string& key, const std::string& value);
    std::string Get(const std::string& key);

   private:
    std::unordered_map<std::string, std::string> store_;
};

#endif // KEY_VALUE_STORE_H_