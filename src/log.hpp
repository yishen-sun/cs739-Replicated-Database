#ifndef LOG_H_
#define LOG_H_

#include <string>
#include <iostream>
#include <unordered_map>
#include <fstream>

using namespace std;

class Log {
   public:
    
    Log(std::string name);
    bool put(const int index, const std::string& term_operand);
    bool removeAfterIndex(const int index);
    std::string getByIndex(const int index);
    int getMaxIndex();
    int getTermByIndex(const int index);
    std::string getCommandByIndex(const int index);
    std::string transferCommand(const std::string& behavior, const std::string& key, const std::string& value);
    void writeToDisk();
    void parseCommand(const std::string& command, std::string& behavior, std::string& arg1, std::string& arg2); // interface to extract data
    
   private:
    // TODO: unordered_map<int, pair<int,string>>
    std::string filename;
    std::unordered_map<int, std::string> store_; 
    int max_index;
    
};

#endif // LOG_H_