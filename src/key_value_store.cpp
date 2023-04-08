#include "key_value_store.h"

BasicKeyValueStore::BasicKeyValueStore(std::string fname) : filename(fname) {}

bool BasicKeyValueStore::Put(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(kv_mutex);
    store_[key] = value;
    writeToDisk();
    return true;
}

std::string BasicKeyValueStore::Get(const std::string& key) {
    std::lock_guard<std::mutex> lock(kv_mutex);
    auto it = store_.find(key);
    if (it != store_.end()) {
        return it->second;
    }
    return "";
}

bool BasicKeyValueStore::Delete(const std::string& key) {
    std::lock_guard<std::mutex> lock(kv_mutex);
    if (Get(key) != "") {
        store_.erase(key);
        writeToDisk();
        return true;
    }
    return false;
}

void BasicKeyValueStore::writeToDisk() {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cout << "Error opening file " << filename << " for writing" << std::endl;
        return;
    }
    for (const auto& entry : store_) {
        file << entry.first << " " << entry.second << std::endl;
    }
    file.flush();
    file.close();
}

#ifdef USE_REDIS

RedisKeyValueStore::RedisKeyValueStore() {
    // Connect to the Redis server
    client.connect(
        "127.0.0.1", 6379,
        [](const std::string& host, std::size_t port, cpp_redis::client::connect_state status) {
            if (status == cpp_redis::client::connect_state::dropped) {
                std::cerr << "Connection to Redis dropped" << std::endl;
            } else {
                std::cout << "Connected to Redis" << std::endl;
            }
        });
}

bool RedisKeyValueStore::Put(const std::string& key, const std::string& value) {
    client.set(key, value);
    client.sync_commit();
    return true;
}

std::string RedisKeyValueStore::Get(const std::string& key) {
    std::string result;
    std::cout << "The key " << key << std::endl;
    client.get(key, [&result](cpp_redis::reply& reply) {
        if (reply.is_null()) {
            std::cout << "The key doesn't exist" << std::endl;
        } else {
            std::cout << "Retrieved value: " << reply.as_string() << std::endl;
            result = reply.as_string();
        }
    });
    client.sync_commit();
    return result;
}

bool RedisKeyValueStore::Delete(const std::string& key) {
    client.del({key});
    client.sync_commit();
    return true;
}

#endif