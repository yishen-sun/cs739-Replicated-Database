#include <string>
#include <unordered_map>

class KeyValueStore {
   public:
    bool Put(const std::string& key, const std::string& value) {
        store_[key] = value;
        return true;
    }

    std::string Get(const std::string& key) {
        auto it = store_.find(key);
        if (it != store_.end()) {
            return it->second;
        }
        return "";
    }

   private:
    std::unordered_map<std::string, std::string> store_;
};
