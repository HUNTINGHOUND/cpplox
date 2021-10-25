#include "pch.pch"
#include "vm.hpp"
#include "flags.hpp"
#include "loxtext/startEditor.hpp"
#include <boost/program_options.hpp>

namespace po = boost::program_options;

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

int main(int argc, const char* argv[]) {
    
    VM vm;
    
    bool openeditor = false;
    std::string filename = "";
    
    po::options_description desc("Allowed options");
    desc.add_options()
    ("help,h", "produce help message")
    ("print_code,p", "print the bytecode")
    ("trace_exec,t", "trace the execution of the byte")
    ("stress_gc,s", "stress test the garbage collector")
    ("debug_gc,d", "print debug log for garbage collector")
    ("input-file,I", po::value<std::string>(), "open given file")
    ("editor,e", "open editor");
    
    po::positional_options_description p;
    p.add("input-file", -1);
    
    po::variables_map varm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), varm);
    po::notify(varm);
    
    if(varm.count("help")) {
        std::cout << desc << "\n";
        return 0;
    }
    
    if(varm.count("print_code")) {
        DEBUG_PRINT_CODE = true;
    }
    if(varm.count("trace_exec")) {
        DEBUG_TRACE_EXECUTION = true;
    }
    if(varm.count("stress_gc")) {
        DEBUG_STRESS_GC = true;
    }
    if(varm.count("debug_gc")) {
        DEBUG_LOG_GC = true;
    }
    if(varm.count("editor")) {
        openeditor = true;
    }
    if(varm.count("input-file")) {
        filename = varm["input-file"].as<std::string>();
    }
    
    if(openeditor) startEditor(filename);
    else if(!filename.empty()) runFile(&vm, filename.c_str());
    else repl(&vm);
    
    vm.freeVM();
}
