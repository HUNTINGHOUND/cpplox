#include <iostream>
#include "value.hpp"
#include "memory.hpp"
#include "object.hpp"

ValueArray::ValueArray() {
    this->count = 0;
    this->capacity = 0;
}

void ValueArray::writeValueArray(Value value){
    if (this->capacity == this->count) {
        size_t newCapa = grow_capacity(capacity);
        values = grow_array<Value>(this->values, this->capacity, newCapa);
        capacity = newCapa;
    }
    
    this->values[this->count] = value;
    this->count++;
}

void ValueArray::freeValueArray(){
    free_array(this->values, this->capacity);
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
        case VAL_OBJ:
            return as_obj(a) == as_obj(b);
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
        case OBJ_FUNCTION:
            printFunction(as_function(value));
            break;
        case OBJ_NATIVE:
            std::cout << "<native fn>";
            break;
        case OBJ_CLOSURE:
            printFunction(as_closure(value)->function);
            break;
        case OBJ_UPVALUE:
            std::cout << "upvalue";
            break;
    }
}

void Value::printFunction(ObjFunction *function) {
    if(function->name == nullptr) {
        std::cout << "<script>";
        return;
    }
    std::cout << "<fn " << function->name->chars << ">";
}

bool Value::is_empty(Value value) {
    return value.type == VAL_EMPTY;
}

Value Value::empty_val() {
    Value val;
    val.type = VAL_EMPTY;
    return val;
}

uint32_t Value::hashDouble(double value) {
    union {
        double value;
        uint32_t ints[2];
    } cast;
    
    cast.value = (value) + 1.0;
    return cast.ints[0] + cast.ints[1];
}

uint32_t Value::hashValue(Value value) {
    switch(value.type) {
        case VAL_BOOL: return as_bool(value) ? 3 : 4;
        case VAL_NUL: return 8;
        case VAL_NUMBER: return hashDouble(as_number(value));
        case VAL_OBJ: return as_string(value)->hash;
        case VAL_EMPTY: return 0;
    }
}

bool Value::is_function(Value value) {
    return isObjType(value, OBJ_FUNCTION);
}

ObjFunction* Value::as_function(Value value) {
    return (ObjFunction*)as_obj(value);
}

bool Value::is_native(Value value) {
    return isObjType(value, OBJ_NATIVE);
}

ObjNative* Value::as_native(Value value) {
    return (ObjNative*)as_obj(value);
}

bool Value::is_closure(Value value) {
    return isObjType(value, OBJ_CLOSURE);
}

ObjClosure* Value::as_closure(Value value) {
    return (ObjClosure*)as_obj(value);
}
