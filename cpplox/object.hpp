#ifndef object_h
#define object_h

#include "flags.hpp"
#include "chunk.hpp"

class VM;

enum ObjType : short{
    OBJ_STRING,
    OBJ_FUNCTION,
    OBJ_NATIVE
};

enum FunctionType {
    TYPE_FUNCTION,
    TYPE_SCRIPT
};

class Obj {
    
    static Obj* allocateObject(size_t size, ObjType type);
    
public:
    ObjType type;
    Obj* next;
    
    template<typename T>
    static T* allocate_obj(ObjType objectType, size_t extra);
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
    Chunk chunk;
    ObjString* name;
    
    static ObjFunction* newFunction();
    
};

using NativeFn = bool(VM::*)(int argCount, Value* args);

class ObjNative : public Obj {
public:
    int arity;
    NativeFn function;
    
    static ObjNative* newNative(NativeFn function, int arity);
};

#endif /* object_h */
