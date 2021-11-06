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
        std::cerr << "Could not open file \\" << path;
        exit(74);
    }
    
    return source;
}
