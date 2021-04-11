
//TODO: Finish Documentation

#ifndef vm_h
#define vm_h

#include "chunk.hpp"
#include "table.hpp"
#include <queue>
#include <cstdarg>
#include <ctime>

#define STACK_MAX 256


enum InterpretResult {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
};

class CallFrame {
public:
    ObjFunction* function;
    uint8_t* ip;
    size_t slots;
    
    CallFrame(ObjFunction* function, uint8_t* ip, size_t slots);
};

class VM {
    
    InterpretResult run();
    
    uint8_t read_byte(CallFrame* frame);
    
    Value read_constant(CallFrame* frame);
    
    uint16_t read_short(CallFrame* frame);
    
    template <typename T, typename U>
    InterpretResult binary_op(Value (*valuetype)(T),std::function<T (U, U)> func);
    
    Value peek(int distance);
    
    bool isFalsey(Value value);
    
    void concatenate();
    
    void runtimeError(const std::string& format, ... );
    
    bool callValue(Value callee, int argCount);
    
    bool call(ObjFunction* function, int argCount);
    
    void defineNative(const std::string& name, NativeFn function, int arity);
    
    //Native functions
    
    bool clockNative(int argCount, Value *args);
    
    bool errNative(int argCount, Value* args);
    
    bool runtimeErrNative(int argCount, Value* args);
    
    bool getLineNative(int argCount, Value* args);
    
public:
    Chunk* chunk;
    uint8_t* ip;
    
    std::deque<CallFrame> frames;
    std::deque<Value> stack;
    
    Table strings;
    Table globalNames;
    ValueArray globalValues;
    
    Obj* objects;
    
    VM();
    void freeVM();
    InterpretResult interpret(const std::string& source);
};



#endif /* vm_h */
