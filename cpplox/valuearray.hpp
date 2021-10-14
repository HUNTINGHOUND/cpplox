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
    std::vector<Value> values;
    
    ValueArray();
    ValueArray(VM* vm);
    void writeValueArray(Value value);
    
};


#endif /* valuearray_hpp */
