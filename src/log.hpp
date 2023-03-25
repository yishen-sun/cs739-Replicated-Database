#ifndef LOG_H_
#define LOG_H_

#include <string>
#include <unordered_map>

class Log {
   public:
    bool put(const int index, const std::string& term_operand);
    bool removeAfterIndex(const int index);
    std::string getByIndex(const int index);
    int getMaxIndex();
   private:
    std::unordered_map<int, std::string> store_;
    int max_index;
};

#endif // LOG_H_