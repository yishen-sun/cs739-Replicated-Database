#include "key_value_store.h"

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

bool::KeyValueStore::Delete(const std::string& key) {
    if (Get(key) != "") {
        store_.erase(key);
        return true;
    }
    return false;
}