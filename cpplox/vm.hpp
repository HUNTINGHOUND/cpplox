
//TODO: Finish Documentation

#ifndef vm_h
#define vm_h

#include "chunk.hpp"
#include <queue>
#include <cstdarg>

#define STACK_MAX 256

enum InterpretResult {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
};

class VM {
    
    InterpretResult run();
    
    uint8_t read_byte();
    
    Value read_constant();
    
    template <typename T>
    InterpretResult VM::binary_op(Value (*valuetype)(T),std::function<double (double, double)> func);
    
    Value peek(int distance);
    
    void runtimeError(const std::string& format, ... );
    
public:
    Chunk* chunk;
    uint8_t* ip;
    std::deque<Value> stack;
    
    VM();
    void freeVM();
    InterpretResult interpret(const std::string& source);
};

#endif /* vm_h */
