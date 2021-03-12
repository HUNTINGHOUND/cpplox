#ifndef object_h
#define object_h

#include "flags.hpp"


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
    
public:
    int length;
    char chars[];
    static ObjString* makeString(int length);
    static ObjString* copyString(const char* chars, int length);
};

#endif /* object_h */
