#include "pch.pch"
#include "vm.hpp"
#include "debug.hpp"
#include "loxtext/startEditor.hpp"

void repl(VM* vm) {
    std::stringstream ss;
    while(true) {
        std::cout << "> ";
        
        std::string line;
        while(std::getline(std::cin, line)) {
            ss << line;
            ss << '\n';
            if(line.length() == 0) {
                break;
            }
        }
        
        vm->interpret(ss.str());
        ss.str(std::string());
    }
}

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

void runFile(VM* vm, const char* path) {
    std::string source = readFile(path);
    InterpretResult result = vm->interpret(source.c_str());
    
    if(result == INTERPRET_COMPILE_ERROR) exit(65);
    if(result == INTERPRET_RUNTIME_ERROR) exit(70);
}

int main(int argc, const char* argv[]) {
    
    VM vm;
    
    if(argc == 1) {
        repl(&vm);
    } else if(argc >= 2) {
        if(strcmp(argv[1], "editor") == 0) {
            std::string filename;
            if(argc == 3) {
                filename = std::string(argv[2]);
            }
            startEditor(filename);
        } else {
            runFile(&vm, argv[1]);
        }
    } else {
        std::cerr << "Usage: cpplox [options]" << std::endl << std::endl;
        std::cerr << "options:" << std::endl;
        std::cerr << "   <filename>          Will compile and run the file specified" << std::endl << std::endl;
        std::cerr << "   editor [filename]   Will open up the built in text editor on the given file, if [filename] is empty, will open an empty text editor." << std::endl << std::endl;
        exit(64);
    }
    
    vm.freeVM();
}
