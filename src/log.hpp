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
    int getTermByIndex(const int index);
    std::string getCommandByIndex(const int index);
    std::string transferCommand(const std::string& behavior, const std::string& key, const std::string& value);
   private:
    // TODO: unordered_map<int, pair<int,string>>
    std::unordered_map<int, std::string> store_; 
    int max_index;
};

#endif // LOG_H_