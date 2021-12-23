#ifndef object_h
#define object_h

#include "pch.pch"
#include "table.hpp"
#include "chunk.hpp"
#include "valuearray.hpp"

class VM;

enum ObjType : short{
    OBJ_STRING,
    OBJ_FUNCTION,
    OBJ_NATIVE,
    OBJ_CLOSURE,
    OBJ_UPVALUE,
    OBJ_CLASS,
    OBJ_INSTANCE,
    OBJ_BOUND_METHOD,
    OBJ_NATIVE_CLASS_METHOD,
    OBJ_NATIVE_CLASS,
    OBJ_NATIVE_COLLECTION,
};

enum FunctionType : short{
    TYPE_FUNCTION,
    TYPE_SCRIPT,
    TYPE_METHOD,
    TYPE_INITIALIZER,
    TYPE_IMPORT
};

class Obj {
    
    static Obj* allocateObject(size_t size, ObjType type, VM* vm);
    
public:
    ObjType type;
    Obj* next;
    bool mark;
    
    template<typename T>
    static T* allocate_obj(ObjType objectType, VM* vm);
};

class ObjString : public Obj {
    
    static ObjString* makeString(VM* vm, size_t length, uint32_t hash);
    
public:
    uint32_t hash;
    std::string chars;
    static ObjString* copyString(VM* vm, const char* chars, size_t length);
    static uint32_t hashString(const char* key, size_t length);
};

class ObjFunction : public Obj{
public:
    int arity;
    int upvalueCount;
    int defaults;
    FunctionType funcType;
    Chunk chunk;
    ObjString* name;
    
    static ObjFunction* newFunction(VM* vm, FunctionType type);
    
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
    std::vector<ObjUpvalue*> upvalues;
    int upvalueCount;
    
    static ObjClosure* newClosure(ObjFunction* function, VM* vm);
};

class ObjClass : public Obj {
public:
    ObjString* name;
    Table methods;
    Obj* initializer;
    
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


struct NativeClassRes;
class ObjNativeClass;
using NativeClassMethod = NativeClassRes(ObjNativeClass::*)(int argCount, Value* args);

class ObjNativeClassMethod : public Obj {
public:
    int arity;
    NativeClassMethod method;
    
    static ObjNativeClassMethod* newNativeClassMethod(NativeClassMethod method, int arity, VM* vm);
};

struct NativeClassRes {
    static NativeClassRes genError(const std::string& errorMessage, bool propertyMissing=false);
    static NativeClassRes genResponse(Value returnVal, bool isVoid=false);
    
    bool hasErr;
    bool propertyMissing;
    
    std::string errorMessage;
    bool isVoid;
    Value returnVal;
};


class ObjNativeClass : public ObjClass {
public:
    ObjType subType;
    
    static ObjNativeClass* newNativeClass(ObjString* name, VM* vm, ObjType subType);
    
    void addMethod(const std::string& name, NativeClassMethod method, int arity, VM* vm);
    virtual NativeClassRes invokeMethod(ObjString* name, int argCount, Value* args)=0;
};


class ObjCollectionClass : public ObjNativeClass{
    NativeClassRes initialize(int argCount, Value* args);
    NativeClassRes addValue(int argCount, Value* args);
    NativeClassRes indexAccess(int argCount, Value* args);
public:
    using CollectionClassMethod = NativeClassRes(ObjCollectionClass::*)(int argCount, Value* args);
    
    ValueArray values;
    
    static ObjCollectionClass* newCollectionClass(ObjString* name, VM* vm);
    NativeClassRes invokeMethod(ObjString* name, int argCount, Value* args);
};

#endif /* object_h */
