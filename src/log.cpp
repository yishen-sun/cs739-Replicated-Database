#include "log.h"

std::string INVALID_LOG = "INVALID_LOG";

Log::Log(std::string name) : filename(name + "_log.txt"), max_index(0) {
    // readFromDisk();
}

bool Log::put(const int index, const std::string& term_operand) {
    if (max_index + 1 != index) {
        return false;
    }
    // modify the shared data
    store_[index] = term_operand;
    max_index = index;
    appendToDisk(index);
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
    overwriteToDisk();
    return true;
}

int Log::getMaxIndex() { return max_index; }

std::string Log::getByIndex(const int index) {
    if ((index > max_index) || (index < 0)) {
        return INVALID_LOG;
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
        return INVALID_LOG;
    }
    std::string term_command = store_[index];
    size_t pos = term_command.find('_');
    if (pos != std::string::npos) {
        std::string command = term_command.substr(pos + 1, term_command.size() - pos - 1);
        return command;
    }
    return INVALID_LOG;
}

std::string Log::transferCommand(const std::string& behavior, const std::string& key,
                                 const std::string& value) {
    if (behavior == "Put") {
        return std::string("P@K=" + key + "@V=" + value);
    } else if (behavior == "Get") {
        return std::string("G@K=" + key);
    } else {
        return "";
    }
}

void Log::parseCommand(const std::string& command, std::string& behavior, std::string& key,
                       std::string& val) {
    // P@K=test1@V=test_reply
    size_t pos1 = command.find('@');
    size_t pos2 = command.find('=');
    size_t pos3 = command.find('@', pos1 + 1);
    if (pos1 == std::string::npos || pos2 == std::string::npos) {
        return;
    }
    behavior = command.substr(0, pos1);
    if (pos3 == std::string::npos) {
        // parse get
        key = command.substr(pos2 + 1, command.size() - pos2 - 1);

    } else {
        // parse put
        key = command.substr(pos2 + 1, pos3 - pos2 - 1);
        size_t pos4 = command.find('=', pos3 + 1);
        if (pos4 == std::string::npos) {
            return;
        }
        val = command.substr(pos4 + 1, command.size() - pos4 - 1);
    }
}

void Log::overwriteToDisk() {
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

void Log::appendToDisk(int index) {
    // cout << "append called" << endl;
    std::ofstream file(filename, std::ios::app);
    if (!file.is_open()) {
        std::cout << "Error opening file " << filename << " for writing" << std::endl;
        return;
    }
    file << index << " " << store_[index] << std::endl;
    file.close();
}
bool Log::readFromDisk() {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Error opening file " << filename << " for reading" << std::endl;
        return false;
    }
    int index;
    std::string term_operand;
    while (file >> index >> term_operand) {
        store_[index] = term_operand;
        max_index = std::max(max_index, index);
    }
    file.close();
    return true;
}