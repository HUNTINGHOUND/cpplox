#include "object.hpp"
#include "memory.hpp"

template<typename T>
T* Obj::allocate_obj(ObjType objectType, size_t extra) {
    return (T*)allocateObject(sizeof(T) + extra, objectType);
}

Obj* Obj::allocateObject(size_t size, ObjType type) {
    Obj* object = (Obj*)reallocate(nullptr, 0, size);
    object->type = type;
    
    return object;
}

ObjString* ObjString::makeString(int length) {
    ObjString* string = Obj::allocate_obj<ObjString>(OBJ_STRING, sizeof(char) * (length + 1));
    string->length = length;
    
    return string;
}

ObjString* ObjString::copyString(const char* chars, int length) {
    ObjString* string = makeString(length);
    memcpy(string->chars, chars, length);
    string->chars[length] = '\0';
    
    return string;
}
