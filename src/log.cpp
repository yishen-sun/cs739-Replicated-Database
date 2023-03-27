#include "log.hpp"

bool Log::put(const int index, const std::string& term_operand) {
    if (max_index + 1 != index) {
        return false;
    }
    // modify the shared data
    store_[index] = term_operand;
    max_index = index;

    return true;
}

bool Log::removeAfterIndex(const int index) {
    if (index > max_index) {
        return true;
    }
    // modify the shared data
    for (int i = max_index; i > index; i--) {
        store_.erase(i);
        max_index = i - 1;
    }
    return true;
}

int Log::getMaxIndex() {
    return max_index;
}

std::string Log::getByIndex(const int index) {
    if ((index > max_index) && (index < 0)) {
        return "invalidLog";
    }
    return store_[index];
}

int Log::getTermByIndex(const int index) {
    if ((index > max_index) && (index < 0)) {
        return -1;
    }
    std::string term_command = store_[index];
    size_t pos = term_command.find('_');
    if (pos != std::string::npos) {
        std::string term = term_command.substr(0, pos);
        std::string value = term_command.substr(pos + 1, term_command.size() - pos - 1);
        return std::stoi(term);
    }
    return -1;
}

std::string Log::getCommandByIndex(const int index) {
    if ((index > max_index) && (index < 0)) {
        return "invalidLog";
    }
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
        return ""
    }
}