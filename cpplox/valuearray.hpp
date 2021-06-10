//
//  valuearray.hpp
//  cpplox
//
//  Created by Morgan Xu on 5/29/21.
//

#ifndef valuearray_hpp
#define valuearray_hpp

#include "pch.pch"
#include "flags.hpp"

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
