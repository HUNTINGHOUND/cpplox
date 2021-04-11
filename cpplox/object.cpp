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
T* Obj::allocate_obj(ObjType objectType, size_t extra) {
    return (T*)allocateObject(sizeof(T) + extra, objectType);
}

Obj* Obj::allocateObject(size_t size, ObjType type) {
    Obj* object = (Obj*)reallocate(nullptr, 0, size);
    object->type = type;
    
    return object;
}

ObjString* ObjString::makeString(VM* vm, int length, uint32_t hash) {
    ObjString* string = Obj::allocate_obj<ObjString>(OBJ_STRING, sizeof(char) * (length + 1));
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

ObjFunction* ObjFunction::newFunction() {
    ObjFunction* function = allocate_obj<ObjFunction>(OBJ_FUNCTION, 0);
    
    function->arity = 0;
    function->name = nullptr;
    function->chunk = Chunk();
    return function;
}

ObjNative* ObjNative::newNative(NativeFn function, int arity) {
    ObjNative* native = allocate_obj<ObjNative>(OBJ_NATIVE, 0);
    native->function = function;
    native->arity = arity;
    return native;
}
