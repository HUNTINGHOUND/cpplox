//
//  object.hpp
//  cpplox
//
//  Created by Morgan Xu on 3/6/21.
//

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
    
    template<typename T>
    static T* allocate_obj(ObjType objectType);
};

class ObjString : public Obj {
    
    static ObjString* allocateString(char* chars, int length);
    
public:
    char* chars;
    int length;
    static ObjString* copyString(const char* chars, int length);
    static ObjString* takeString(char* chars, int length);
};

#endif /* object_h */
