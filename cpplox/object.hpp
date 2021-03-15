#ifndef object_h
#define object_h

#include "flags.hpp"

class VM;

enum ObjType{
    OBJ_STRING
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

#endif /* object_h */
