#include "util.hpp"

std::string readFile(const char* path) {
    std::fstream file(path);
    std::string source;
    std::string line;
    
    if(file.is_open()) {
        while(getline(file, line)) {
            source += line;
            source += '\n';
        }
        
        file.close();
    } else {
        std::string errormessage = "Cannot open file ";
        errormessage += path;
        throw errormessage;
    }
    
    return source;
}
