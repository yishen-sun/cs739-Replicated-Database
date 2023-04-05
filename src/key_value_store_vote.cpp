#include "key_value_store_vote.h"

KeyValueStoreVote::KeyValueStoreVote(std::string fname) : filename(fname) {}

bool KeyValueStoreVote::Put(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(kv_mutex);
    store_[key] = value;
    writeToDisk();
    return true;
}

std::string KeyValueStoreVote::Get(const std::string& key) {
    std::lock_guard<std::mutex> lock(kv_mutex);
    auto it = store_.find(key);
    if (it != store_.end()) {
        return it->second;
    }
    return "";
}

bool KeyValueStoreVote::Delete(const std::string& key) {
    std::lock_guard<std::mutex> lock(kv_mutex);
    if (Get(key) != "") {
        store_.erase(key);
        writeToDisk();
        return true;
    }
    return false;
}

void KeyValueStoreVote::writeToDisk() {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cout << "Error opening file " << filename << " for writing"
                  << std::endl;
        return;
    }
    for (const auto& entry : store_) {
        file << entry.first << " " << entry.second << std::endl;
    }
    file.flush();
    file.close();
}