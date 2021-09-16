#ifndef valuearray_hpp
#define valuearray_hpp

#include "pch.pch"

#ifdef NANBOXING
#include "nanvalue.hpp"
#else
#include "value.hpp"
#endif

class VM;

class ValueArray {
    VM* vm;
public:
    size_t count;
    size_t capacity;
    Value* values;
    
    ValueArray(VM* vm);
    void writeValueArray(Value value);
    void freeValueArray();
    
};


#endif /* valuearray_hpp */
