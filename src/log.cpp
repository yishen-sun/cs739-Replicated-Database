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