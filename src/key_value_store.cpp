#include "key_value_store.hpp"

KeyValueStore global_store;

bool KeyValueStore::Put(const std::string& key, const std::string& value) {
    store_[key] = value;
    return true;
}

std::string KeyValueStore::Get(const std::string& key) {
    auto it = store_.find(key);
    if (it != store_.end()) {
        return it->second;
    }
    return "";
}