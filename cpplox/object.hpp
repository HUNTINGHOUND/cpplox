#ifndef object_h
#define object_h

#include "flags.hpp"
#include "chunk.hpp"
#include "table.hpp"

class VM;

enum ObjType : short{
    OBJ_STRING,
    OBJ_FUNCTION,
    OBJ_NATIVE,
    OBJ_CLOSURE,
    OBJ_UPVALUE,
    OBJ_CLASS,
    OBJ_INSTANCE,
    OBJ_BOUND_METHOD
};

enum FunctionType : short{
    TYPE_FUNCTION,
    TYPE_SCRIPT
};

class Obj {
    
    static Obj* allocateObject(size_t size, ObjType type, VM* vm);
    
public:
    ObjType type;
    Obj* next;
    bool mark;
    
    template<typename T>
    static T* allocate_obj(ObjType objectType, size_t extra, VM* vm);
};

class ObjString : public Obj {
    
    static ObjString* makeString(VM* vm, int length, uint32_t hash);
    
public:
    int length;
    uint32_t hash;
    char chars[];
    static ObjString* copyString(VM* vm, const char* chars, int length);
    static uint32_t hashString(const char* key, int length);
};

class ObjFunction : public Obj{
public:
    int arity;
    int upvalueCount;
    Chunk chunk;
    ObjString* name;
    
    static ObjFunction* newFunction(VM* vm);
    
};

using NativeFn = bool(VM::*)(int argCount, Value* args);

class ObjNative : public Obj {
public:
    int arity;
    NativeFn function;
    
    static ObjNative* newNative(NativeFn function, int arity, VM* vm);
};

class ObjUpvalue : public Obj {
public:
    Value* location;
    
    ObjUpvalue* nextUp;
    Value closed;
    
    static ObjUpvalue* newUpvalue(Value* slot, VM* vm);
};

class ObjClosure : public Obj {
public:
    ObjFunction* function;
    ObjUpvalue** upvalues;
    int upvalueCount;
    
    static ObjClosure* newClosure(ObjFunction* function, VM* vm);
};

class ObjClass : public Obj {
public:
    ObjString* name;
    Table methods;
    
    static ObjClass* newClass(ObjString* name, VM* vm);
};

class ObjInstance : public Obj {
public:
    ObjClass* _class;
    Table fields;
    
    static ObjInstance* newInstance(ObjClass* _class, VM* vm);
};

class ObjBoundMethod : public Obj {
public:
    Value receiver;
    Obj* method;
    
    static ObjBoundMethod* newBoundMethod(Value receiver, Obj* method, VM* vm);
};

#endif /* object_h */
