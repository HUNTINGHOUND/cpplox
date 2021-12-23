#include "valuearray.hpp"
#include "memory.hpp"

ValueArray::ValueArray() {
    count = 0;
    vm = nullptr;
}

ValueArray::ValueArray(VM* vm) {
    count = 0;
    this->vm = vm;
}

void ValueArray::writeValueArray(Value value){
    values.push_back(value);
    count++;
}

