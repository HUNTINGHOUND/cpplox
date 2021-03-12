#include "memory.hpp"
#include "value.hpp"
#include "chunk.hpp"

void* reallocate(void* pointer, size_t oldsize, size_t newsize) {
    if (newsize == 0) {
        free(pointer);
        return nullptr;
    }
    
    void* result = realloc(pointer, newsize);
    return result;
}

void freeObjects(VM* vm) {
    Obj* object = vm->objects;
    while (object != nullptr) {
        Obj* next = object->next;
        freeObject(object);
        object = next;
    }
}

void freeObject(Obj* object) {
    switch (object->type) {
        case OBJ_STRING: {
            ObjString* string = (ObjString*) object;
            reallocate(string->chars, sizeof(char) * (string->length + 1), 0);
            reallocate(object, sizeof(ObjString), 0);
        }
    }
}

