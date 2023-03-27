#include "log.hpp"

Log::Log(std::string name): filename(name + "_log.txt"), max_index(0) {}

bool Log::put(const int index, const std::string& term_operand) {
    if (max_index + 1 != index) {
        return false;
    }
    // modify the shared data
    store_[index] = term_operand;
    max_index = index;
    writeToDisk();
    return true;
}

bool Log::removeAfterIndex(const int index) {
    if (index > max_index) {
        return true;
    }
    // modify the shared data
    for (int i = max_index; i > index; i--) {
        store_.erase(i);
    }
    max_index = index - 1;
    writeToDisk();
    return true;
}

int Log::getMaxIndex() {
    return max_index;
}

std::string Log::getByIndex(const int index) {
    if ((index > max_index) || (index < 0)) {
        return "invalidLog";
    }
    return store_[index];
}

int Log::getTermByIndex(const int index) {
    if ((index > max_index) || (index < 0)) {
        return -1;
    }
    std::string term_command = store_[index];
    size_t pos = term_command.find('_');
    if (pos != std::string::npos) {
        std::string term = term_command.substr(0, pos);
        return std::stoi(term);
    }
    return -1;
}

std::string Log::getCommandByIndex(const int index) {
    if ((index > max_index) || (index < 0)) {
        return "invalidLog";
    }
    std::cout << "max index: " << max_index << std::endl;
    std::string term_command = store_[index];
    size_t pos = term_command.find('_');
    if (pos != std::string::npos) {
        std::string command = term_command.substr(pos + 1, term_command.size() - pos - 1);
        return command;
    }
    return "invalidLog";
}

std::string Log::transferCommand(const std::string& behavior,const std::string& key, const std::string& value) {
    if (behavior == "Put") {
        return std::string("P@K=" + key + "@V=" + value);
    }
    else if (behavior == "Get") {
        return std::string("G@K=" + key);
    } else {
        return "";
    }
}

void Log::parseCommand(const std::string& command, std::string& behavior, std::string& key, std::string& val) {
    size_t pos1 = command.find('@');
    size_t pos2 = command.find('=');
    size_t pos3 = command.find('@', pos1 + 1);
    if (pos1 == std::string::npos || pos2 == std::string::npos || pos3 == std::string::npos) {
        return;
    }
    behavior = command.substr(0, pos1);
    key = command.substr(pos1 + pos2 + 2, pos3 - pos1 - pos2 - 2);
    if (behavior == "P") {
        size_t pos4 = command.find('=', pos3 + 1);
        if (pos4 == std::string::npos) {
            return;
        }
        val = command.substr(pos4 + 1, command.size() - pos4 - 1);
    }
}


void Log::writeToDisk() {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cout << "Error opening file " << filename << " for writing" << std::endl;
        return;
    }
    for (const auto& entry : store_) {
        file << entry.first << " " << entry.second << std::endl;
    }
    file.close();
}
