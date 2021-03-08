#include "object.hpp"
#include "memory.hpp"

template<typename T>
T* Obj::allocate_obj(ObjType objectType) {
    return (T*)allocateObject(sizeof(T), objectType);
}

Obj* Obj::allocateObject(size_t size, ObjType type) {
    Obj* object = (Obj*)reallocate(nullptr, 0, size);
    object->type = type;
    return object;
}

ObjString* ObjString::copyString(const char* chars, int length) {
    char* heapChars = allocate<char>(length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';
    
    return allocateString(heapChars, length);
}

ObjString* ObjString::allocateString(char* chars, int length) {
    ObjString* string = Obj::allocate_obj<ObjString>(OBJ_STRING);
    string->length = length;
    string->chars = chars;
    
    return string;
}

ObjString* ObjString::takeString(char* chars, int length) {
    return allocateString(chars, length);
}
