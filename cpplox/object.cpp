#include "object.hpp"
#include "memory.hpp"
#include "vm.hpp"


uint32_t ObjString::hashString(const char* key, int length) {
    uint32_t hash = 2166136261u;

    for (int i = 0; i < length; i++) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }

    return hash;
}


template<typename T>
T* Obj::allocate_obj(ObjType objectType, size_t extra, VM* vm) {
    return (T*)allocateObject(sizeof(T) + extra, objectType, vm);
}

Obj* Obj::allocateObject(size_t size, ObjType type, VM* vm) {
    Obj* object = (Obj*)reallocate(nullptr, 0, size, vm);
    object->type = type;
    object->isMarked = false;
    
#ifdef DEBUG_LOG_GC
    std::cout << (void*)object << " allocate " << size << " for " << type << std::endl;
#endif
    
    return object;
}

ObjString* ObjString::makeString(VM* vm, int length, uint32_t hash) {
    ObjString* string = Obj::allocate_obj<ObjString>(OBJ_STRING, sizeof(char) * (length + 1), vm);
    string->length = length;
    string->hash = hash;
    
    string->next = vm->objects;
    vm->objects = string;
    
    vm->strings.tableSet(Value::obj_val(string), Value::nul_val());
    
    return string;
}

ObjString* ObjString::copyString(VM* vm, const char* chars, int length) {
    uint32_t hash = hashString(chars, length);
    ObjString* interned = vm->strings.tableFindString(chars, length,
                                             hash);
    
    if (interned != nullptr) {
        return interned;
    }
    
    ObjString* string = makeString(vm, length, hash);
    memcpy(string->chars, chars, length);
    string->chars[length] = '\0';
    
    return string;
}

ObjFunction* ObjFunction::newFunction(VM* vm) {
    ObjFunction* function = allocate_obj<ObjFunction>(OBJ_FUNCTION, 0, vm);
    
    function->arity = 0;
    function->upvalueCount = 0;
    function->name = nullptr;
    function->chunk = Chunk(vm);
    return function;
}

ObjNative* ObjNative::newNative(NativeFn function, int arity, VM* vm) {
    ObjNative* native = allocate_obj<ObjNative>(OBJ_NATIVE, 0, vm);
    native->function = function;
    native->arity = arity;
    return native;
}

ObjClosure* ObjClosure::newClosure(ObjFunction* function, VM* vm) {
    ObjUpvalue** upvalues = allocate<ObjUpvalue*>(function->upvalueCount, vm);
    for (int i = 0; i < function->upvalueCount; i++) {
        upvalues[i] = NULL;
    }
    
    ObjClosure* closure = allocate_obj<ObjClosure>(OBJ_CLOSURE, 0, vm);
    closure->function = function;
    closure->upvalues = upvalues;
    closure->upvalueCount = function->upvalueCount;
    return closure;
}

ObjUpvalue* ObjUpvalue::newUpvalue(Value* slot, VM* vm) {
    ObjUpvalue* upvalue = allocate_obj<ObjUpvalue>(OBJ_UPVALUE, 0, vm);
    upvalue->next = nullptr;
    upvalue->location = slot;
    upvalue->closed = Value::nul_val();
    return upvalue;
}
