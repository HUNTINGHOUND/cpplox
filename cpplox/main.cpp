#include "pch.pch"
#include "vm.hpp"
#include "flags.hpp"
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
        
        if(ss.str().compare("exit\n\n") == 0) exit(0);
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

void usageError(){
    std::cout << "Usage: ./cpplox -[options] [editor] [filepath]" << std::endl;
    return exit(1);
}

int main(int argc, const char* argv[]) {
    
    VM vm;
    
    if(argc == 1) {
        repl(&vm);
    } else if(argc >= 2) {
        
        bool openeditor = false;
        for(int i = 1; i < argc; i++) {
            std::string option(argv[i]);
            if(option.compare("editor") == 0) {
                if(i != argc - 2 || i != argc - 1) usageError();
                
                openeditor = true;
                std::string filename;
                if(i == argc - 2) {
                    filename = std::string(argv[i + 1]);
                }
                startEditor(filename);
            } else if(option[0] == '-') {
                if(option.compare("-verb") == 0) {
                    DEBUG_PRINT_CODE = true;
                } else if(option.compare("-trace") == 0) {
                    DEBUG_TRACE_EXECUTION = true;
                } else if(option.compare("-stress") == 0) {
                    DEBUG_STRESS_GC = true;
                } else if(option.compare("-loggc") == 0) {
                    DEBUG_LOG_GC = true;
                } else {
                    std::cout << option << " not recognized as valid option." << std::endl;
                    exit(1);
                }
            } else if(i == argc - 1) {
                runFile(&vm, argv[i]);
            } else {
                usageError();
            }
            
        }
        
        if(!openeditor) repl(&vm);
    } else {
        usageError();
    }
    
    vm.freeVM();
}
