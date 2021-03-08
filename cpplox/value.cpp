#include <iostream>
#include "value.hpp"
#include "memory.hpp"

ValueArray::ValueArray() {
    this->count = 0;
}

void ValueArray::writeValueArray(Value value){
    if (this->values.capacity() == this->count) {
        size_t oldCapacity = this->values.capacity();
        grow_array<Value>(this->values, grow_capacity(oldCapacity));
    }
    
    this->values[this->count] = value;
    this->count++;
}

void ValueArray::freeValueArray(){
    free_array(this->values);
    ValueArray();
}

void Value::printValue(Value value) {
    switch (value.type) {
        case VAL_BOOL:
            std::cout << (Value::as_bool(value) ? "true" : "false");
            break;
        case VAL_NUL:
            std::cout << "nul";
            break;
        case VAL_NUMBER:
            std::cout << Value::as_number(value);
            break;
        case VAL_OBJ:
            printObject(value);
            break;
    }
}

Value Value::bool_val(bool value) {
    Value val;
    val.type = VAL_BOOL;
    val.as.boolean = value;
    return val;
}

Value Value::nul_val() {
    Value val;
    val.type = VAL_NUL;
    val.as.number = 0;
    return val;
}

Value Value::number_val(double value) {
    Value val;
    val.type = VAL_NUMBER;
    val.as.number = value;
    return val;
}

bool Value::as_bool(Value value) {
    return value.as.boolean;
}

double Value::as_number(Value value) {
    return value.as.number;
}

bool Value::is_bool(Value value) {
    return value.type == VAL_BOOL;
}

bool Value::is_nul(Value value) {
    return value.type == VAL_NUL;
}

bool Value::is_number(Value value) {
    return value.type == VAL_NUMBER;
}

bool Value::valuesEqual(Value a, Value b) {
    if (a.type != b.type) return false;
    
    switch (a.type) {
        case VAL_BOOL:
            return Value::as_bool(a) == Value::as_bool(b);
        case VAL_NUL:
            return true;
        case VAL_NUMBER:
            return Value::as_number(a) == Value::as_number(b);
        case VAL_OBJ: {
            ObjString* aString = as_string(a);
            ObjString* bString = as_string(b);
            return (aString->length == bString->length) &&
            (memcmp(aString->chars, bString->chars, aString->length) == 0);
        }
        default:
            return false; //unreachable
    }
}

bool Value::is_obj(Value value) {
    return value.type == VAL_OBJ;
}

bool Value::is_string(Value value) {
    return isObjType(value, OBJ_STRING);
}

bool Value::isObjType(Value value, ObjType type) {
    return is_obj(value) && as_obj(value)->type == type;
}

Obj* Value::as_obj(Value value) {
    return value.as.obj;
}

Value Value::obj_val(Obj* obj) {
    Value val;
    val.type = VAL_OBJ;
    val.as.obj = obj;
    return val;
}

ObjType Value::obj_type(Value value) {
    return value.as.obj->type;
}

ObjString* Value::as_string(Value value) {
    return (ObjString*)as_obj(value);
}

char* Value::as_c_string(Value value) {
    return ((ObjString*)as_obj(value))->chars;
}

void Value::printObject(Value value) {
    switch(obj_type(value)) {
        case OBJ_STRING:
            std::cout << as_c_string(value);
            break;
    }
}
