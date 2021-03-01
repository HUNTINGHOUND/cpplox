#include <iostream>
#include "value.hpp"
#include "memory.hpp"

ValueArray::ValueArray() {
    this->count = 0;
}

void ValueArray::writeValueArray(Value value){
    if (this->values.capacity() == this->count) {
        size_t oldCapacity = this->values.capacity();
        grow_array<Value>(this->values, oldCapacity, grow_capacity(oldCapacity));
    }
    
    this->values[this->count] = value;
    this->count++;
}

void ValueArray::freeValueArray(){
    free_array(this->values, this->values.capacity());
    ValueArray();
}

void ValueArray::printValue(Value value) {
    switch (value.type) {
        case VAL_BOOL:
            std::cout << (as_bool(value) ? "true" : "false");
            break;
        case VAL_NUL:
            std::cout << "nul";
            break;
        case VAL_NUMBER:
            std::cout << as_number(value);
            break;
    }
}

Value bool_val(bool value) {
    Value val;
    val.type = VAL_BOOL;
    val.as.boolean = value;
    return val;
}

Value nul_val() {
    Value val;
    val.type = VAL_NUL;
    val.as.number = 0;
    return val;
}

Value number_val(double value) {
    Value val;
    val.type = VAL_NUMBER;
    val.as.number = value;
    return val;
}

bool as_bool(Value value) {
    return value.as.boolean;
}

double as_number(Value value) {
    return value.as.number;
}

bool is_bool(Value value) {
    return value.type == VAL_BOOL;
}

bool is_nul(Value value) {
    return value.type == VAL_NUL;
}

bool is_number(Value value) {
    return value.type == VAL_NUMBER;
}
