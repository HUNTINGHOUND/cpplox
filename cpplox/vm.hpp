
//TODO: Finish Documentation

#ifndef vm_h
#define vm_h

#include "chunk.hpp"
#include "table.hpp"
#include "object.hpp"
#include <queue>
#include <cstdarg>
#include <ctime>

#define STACK_MAX 256

class Compiler;
class ClassCompiler;

enum InterpretResult {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
};

class CallFrame {
public:
    Obj* function;
    uint8_t* ip;
    size_t slots;
    
    CallFrame(Obj* function, uint8_t* ip, size_t slots);
};

class VM {
    
    InterpretResult run();
    
    uint8_t read_byte(CallFrame* frame);
    
    Value read_constant(CallFrame* frame);
    
    uint16_t read_short(CallFrame* frame);
    
    ObjString* read_string(CallFrame* frame);
    
    template <typename T, typename U>
    InterpretResult binary_op(Value (*valuetype)(T),std::function<T (U, U)> func);
    
    Value peek(int distance);
    
    bool isFalsey(Value value);
    
    void concatenate();
    
    void runtimeError(const std::string& format, ... );
    
    bool callValue(Value callee, int argCount);
    
    bool call(Obj* callee, ObjFunction* function, int argCount);
    
    bool callClosure(ObjClosure* closure, int argCount);
    
    bool callFunction(ObjFunction* function, int argCount);
    
    void defineNative(const std::string& name, NativeFn function, int arity);
    
    ObjUpvalue* captureUpvalue(int localIndex);
    
    void closeUpvalues(Value* last);
    
    ObjFunction* getFrameFunction(CallFrame* frame);
    
    void resetStacks();
    
    void defineMethod(ObjString* name);
    
    bool bindMethod(ObjClass* _class, ObjString* name);
    
    //Native functions
    
    bool clockNative(int argCount, Value *args);
    
    bool errNative(int argCount, Value* args);
    
    bool runtimeErrNative(int argCount, Value* args);
    
    bool getLineNative(int argCount, Value* args);
    
    bool hasFieldNative(int argCount, Value* args);
    
    bool getFieldNative(int argCount, Value* args);
    
    bool setFieldNative(int argCount, Value* args);
    
    bool invoke(ObjString* name, int argCount);
    
    bool invokeFromClass(ObjClass* _class, ObjString* name, int argCount);
    
public:
    Compiler* current;
    ClassCompiler* currentClass;
    
    Chunk* chunk;
    uint8_t* ip;
    
    ObjUpvalue* openUpvalues;
    
    std::deque<CallFrame> frames;
    std::deque<Value> stack;
    std::deque<Obj*> grayStack;
    
    size_t bytesAllocated;
    size_t nextGC;
    
    Table strings;
    Table globalNames;
    ValueArray globalValues;
    
    ObjString* initString;
    
    Obj* objects;
    
    bool marker;
    
    VM();
    void freeVM();
    InterpretResult interpret(const std::string& source);
};



#endif /* vm_h */
