//
//  valuearray.cpp
//  cpplox
//
//  Created by Morgan Xu on 5/29/21.
//

#include "valuearray.hpp"
#include "memory.hpp"


ValueArray::ValueArray(VM* vm) {
    this->count = 0;
    this->capacity = 0;
    this->vm = vm;
}

void ValueArray::writeValueArray(Value value){
    if (this->capacity == this->count) {
        size_t newCapa = grow_capacity(capacity);
        values = grow_array<Value>(this->values, this->capacity, newCapa, vm);
        capacity = newCapa;
    }
    
    this->values[this->count] = value;
    this->count++;
}

void ValueArray::freeValueArray(){
    free_array(this->values, this->capacity, vm);
}
