#include "memory.hpp"
#include "object.hpp"

#ifdef NAN_BOXING
#include "nanvalue.hpp"
#else
#include "value.hpp"
#endif

#ifdef NAN_BOXING

Value ValueOP::number_val(double num) {
    Value value;
    memcpy(&value, &num, sizeof(double));
    return value;
}
Value ValueOP::nul_val() {
    return (Value)((uint64_t)(qnan | tag_nul));
}
Value ValueOP::empty_val() {
    return (Value)qnan;
}
Value ValueOP::true_val() {
    return (Value)((uint64_t)(qnan | tag_true));
}
Value ValueOP::false_val() {
    return (Value)((uint64_t)(qnan | tag_false));
}
Value ValueOP::bool_val(bool value) {
    return value ? true_val() : false_val();
}
Value ValueOP::obj_val(Obj* obj) {
    return (Value)(sign_bit | qnan | (int64_t)(uintptr_t)(obj));
}

//==================================================

double ValueOP::as_number(Value value) {
    double num;
    memcpy(&num, &value, sizeof(Value));
    return num;
}
bool ValueOP::as_bool(Value value) {
    return value == true_val();
}
Obj* ValueOP::as_obj(Value value) {
    return (Obj*)(uintptr_t)(value & ~(sign_bit | qnan));
}

//==================================================

bool ValueOP::is_number(Value value) {
    return (value & qnan) != qnan;
}
bool ValueOP::is_nul(Value value) {
    return value == nul_val();
}
bool ValueOP::is_bool(Value value) {
    return (value | 1) == true_val();
}
bool ValueOP::is_obj(Value value) {
    return ((value & (qnan | sign_bit)) == (qnan | sign_bit));
}
bool ValueOP::is_empty(Value value) {
    return value == qnan;
}
bool ValueOP::isConst(Value value) {
    return ((value >> 49) & 1) == 1;
}

//===================================================

void ValueOP::setConst(bool isCon, Value value) {
    if(isCon) value |= const_tag;
    else value &= (~const_tag);
}

#else

Value ValueOP::bool_val(bool value) {
    Value val;
    val.type = VAL_BOOL;
    val.as.boolean = value;
    return val;
}

Value ValueOP::nul_val() {
    Value val;
    val.type = VAL_NUL;
    val.as.number = 0;
    return val;
}

Value ValueOP::number_val(Number value) {
    Value val;
    val.type = VAL_NUMBER;
    val.as.number = value;
    return val;
}

bool ValueOP::as_bool(Value value) {
    return value.as.boolean;
}

Number ValueOP::as_number(Value value) {
    return value.as.number;
}

bool ValueOP::is_bool(Value value) {
    return value.type == VAL_BOOL;
}

bool ValueOP::is_nul(Value value) {
    return value.type == VAL_NUL;
}

bool ValueOP::is_number(Value value) {
    return value.type == VAL_NUMBER;
}

bool ValueOP::is_whole_number(Value value) {
    return !as_number(value).is_float;
}

bool ValueOP::is_obj(Value value) {
    return value.type == VAL_OBJ;
}

Obj* ValueOP::as_obj(Value value) {
    return value.as.obj;
}

Value ValueOP::obj_val(Obj* obj) {
    Value val;
    val.type = VAL_OBJ;
    val.as.obj = obj;
    return val;
}

bool ValueOP::is_empty(Value value) {
    return value.type == VAL_EMPTY;
}

Value ValueOP::empty_val() {
    Value val;
    val.type = VAL_EMPTY;
    return val;
}

bool ValueOP::isConst(Value value) {
    return value.isConst;
}

void ValueOP::setConst(bool isCon, Value value) {
    value.isConst = isCon;
}
#endif


void ValueOP::printValue(Value value) {
#ifdef NAN_BOXING
    if (is_bool(value)) {
        std::cout << (is_bool(value) ? "true" : "false");
    } else if (is_nul(value)) {
        std::cout << "nul";;
    } else if (is_number(value)) {
        std::cout << as_number(value);;
    } else if (is_obj(value)) {
        printObject(value);
    }
    
#else
    switch (value.type) {
        case VAL_BOOL:
            std::cout << (is_bool(value) ? "true" : "false");
            break;
        case VAL_NUL:
            std::cout << "nul";
            break;
        case VAL_NUMBER: {
            std::cout << as_number(value);
            break;
        }
        case VAL_OBJ:
            printObject(value);
            break;
        case VAL_EMPTY:
            std::cout << "empty";
            break;
    }
#endif
}

bool ValueOP::valuesEqual(Value a, Value b) {
#ifdef NAN_BOXING
    if (is_number(a) && is_number(b)) {
        return as_number(a) == as_number(b);
    }
    
    return a == b;
#else
    if (a.type != b.type) return false;
    
    switch (a.type) {
        case VAL_BOOL:
            return ValueOP::as_bool(a) == ValueOP::as_bool(b);
        case VAL_NUL:
            return true;
        case VAL_NUMBER:
            return ValueOP::as_number(a) == ValueOP::as_number(b);
        case VAL_OBJ:
            return as_obj(a) == as_obj(b); // covers string as all strings are interned
        default:
            return false; //unreachable
    }
#endif
}

bool ValueOP::is_string(Value value) {
    return isObjType(value, OBJ_STRING);
}

bool ValueOP::isObjType(Value value, ObjType type) {
    return is_obj(value) && as_obj(value)->type == type;
}

ObjType ValueOP::obj_type(Value value) {
    return as_obj(value)->type;
}

ObjString* ValueOP::as_string(Value value) {
    return (ObjString*)as_obj(value);
}

std::string* ValueOP::as_std_string(Value value) {
    return &((ObjString*)as_obj(value))->chars;
}

void ValueOP::printObject(Value value) {
    switch(obj_type(value)) {
        case OBJ_BOUND_METHOD:
            printFunction(get_value_function(value));
            break;
        case OBJ_STRING:
            std::cout << *as_std_string(value);
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
        case OBJ_CLASS:
            std::cout << as_class(value)->name->chars;
            break;
        case OBJ_INSTANCE:
            std::cout << as_instance(value)->_class->name->chars << " instance";
            break;
        case OBJ_NATIVE_CLASS_METHOD:
            std::cout << "Native class method.";
            break;
        case OBJ_NATIVE_CLASS: {
            ObjNativeClass* _class = as_native_class(value);
            switch (_class->subType) {
                case NATIVE_COLLECTION:
                    std::cout << "Collection Class";
                    break;
            }
            break;
        }
        case OBJ_NATIVE_INSTANCE: {
            ObjNativeInstance* instance = as_native_instance(value);
            switch (instance->subType) {
                case NATIVE_COLLECTION_INSTANCE: {
                    ObjCollectionInstance* collection = static_cast<ObjCollectionInstance*>(instance);
                    std::cout << "{";
                    for(int i = 0; i < collection->values.count; i++) {
                        printValue(collection->values.values[i]);
                        if(i != collection->values.count - 1) std::cout << ", ";
                    }
                    std::cout << "}";
                }
            }
        }
    }
}

void ValueOP::printFunction(ObjFunction *function) {
    if(function->name == nullptr) {
        std::cout << "<script>";
        return;
    }
    std::cout << "<fn " << function->name->chars << ">";
}


uint32_t ValueOP::hashNumber(Number value) {
    union {
        double decimal;
        long long whole;
        uint32_t ints[2];
    } cast;
    
    if(value.is_float) cast.decimal = value.number.decimal + 1.0;
    else cast.whole = value.number.whole + 1;
    return cast.ints[0] + cast.ints[1];
}

uint32_t ValueOP::hashValue(Value value) {
#ifdef NAN_BOXING
    if(is_bool(value)) {
        return as_bool(value) ? 3 : 4;
    } else if (is_nul(value)) {
        return 8;
    } else if (is_obj(value)) {
        return as_string(value)->hash;
    } else {
        return 0;
    }
#else
    switch(value.type) {
        case VAL_BOOL: return as_bool(value) ? 3 : 4;
        case VAL_NUL: return 8;
        case VAL_NUMBER: return hashNumber(as_number(value));
        case VAL_OBJ: return as_string(value)->hash;
        case VAL_EMPTY: return 0;
    }
#endif
}

bool ValueOP::is_function(Value value) {
    return isObjType(value, OBJ_FUNCTION);
}

ObjFunction* ValueOP::as_function(Value value) {
    return (ObjFunction*)as_obj(value);
}

bool ValueOP::is_native(Value value) {
    return isObjType(value, OBJ_NATIVE);
}

ObjNative* ValueOP::as_native(Value value) {
    return (ObjNative*)as_obj(value);
}

bool ValueOP::is_closure(Value value) {
    return isObjType(value, OBJ_CLOSURE);
}

ObjClosure* ValueOP::as_closure(Value value) {
    return (ObjClosure*)as_obj(value);
}

bool ValueOP::is_class(Value value) {
    return isObjType(value, OBJ_CLASS);
}

ObjClass* ValueOP::as_class(Value value) {
    return (ObjClass*)as_obj(value);
}

bool ValueOP::is_instance(Value value) {
    return isObjType(value, OBJ_INSTANCE);
}

ObjInstance* ValueOP::as_instance(Value value) {
    return (ObjInstance*)as_obj(value);
}

bool ValueOP::is_bound_method(Value value) {
    return isObjType(value, OBJ_BOUND_METHOD);
}

ObjBoundMethod* ValueOP::as_bound_method(Value value) {
    return ((ObjBoundMethod*)ValueOP::as_obj(value));
}

ObjFunction* ValueOP::get_value_function(Value value) {
    Obj* obj = as_obj(value);
    if(obj->type == OBJ_FUNCTION) {
        return as_function(value);
    } else if(obj->type == OBJ_CLOSURE) {
        return as_closure(value)->function;
    } else if(obj->type == OBJ_BOUND_METHOD) {
        ObjBoundMethod* bound = as_bound_method(value);
        if(bound->method->type == OBJ_FUNCTION) {
            return (ObjFunction*)(bound->method);
        } else {
            ObjClosure* closure = (ObjClosure*)(bound->method);
            return closure->function;
        }
    } else {
        return nullptr;
    }
}

ObjFunction* ValueOP::get_obj_function(Obj* obj) {
    if(obj->type == OBJ_FUNCTION) {
        return (ObjFunction*)obj;
    } else if(obj->type == OBJ_CLOSURE) {
        return ((ObjClosure*)obj)->function;
    } else if(obj->type == OBJ_BOUND_METHOD) {
        ObjBoundMethod* bound = (ObjBoundMethod*)obj;
        if(bound->method->type == OBJ_FUNCTION) {
            return (ObjFunction*)(bound->method);
        } else {
            ObjClosure* closure = (ObjClosure*)(bound->method);
            return closure->function;
        }
    } else {
        return nullptr;
    }
}

bool ValueOP::is_native_class(Value value) {
    return as_obj(value)->type == OBJ_NATIVE_CLASS;
}

bool ValueOP::is_native_subclass(Value value, NativeClassType type) {
    return is_native_class(value) && as_native_class(value)->subType == type;
}

ObjNativeClass* ValueOP::as_native_class(Value value) {
    return static_cast<ObjNativeClass*>(as_obj(value));
}

ObjString* ValueOP::to_string(Value value, VM* vm) {
    switch (value.type) {
        case VAL_BOOL: {
            return ObjString::copyString(vm, value.as.boolean ? "true" : "false");
        }
        case VAL_NUL: {
            return ObjString::copyString(vm, "nul");
        }
        case VAL_NUMBER: {
            std::stringstream ss;
            ss << value.as.number;
            return ObjString::copyString(vm, ss.str());
        }
        case VAL_OBJ: {
            return object_to_string(value, vm);
        }
        default: {
            return ObjString::copyString(vm, "");
        }
    }
}

ObjString* ValueOP::object_to_string(Value value, VM* vm) {
    switch(obj_type(value)) {
        case OBJ_FUNCTION:
        case OBJ_BOUND_METHOD:
            return function_to_string(get_value_function(value), vm);
        case OBJ_STRING:
            return as_string(value);
        case OBJ_NATIVE:
            return ObjString::copyString(vm, "<native fn>");
        case OBJ_CLOSURE:
            return function_to_string(get_value_function(value), vm);
        case OBJ_UPVALUE:
            return ObjString::copyString(vm, "upvalue");
        case OBJ_CLASS:
            return ObjString::copyString(vm, as_class(value)->name->chars);
        case OBJ_INSTANCE: {
            std::string buffer(as_instance(value)->_class->name->chars);
            buffer += " instance";
            return ObjString::copyString(vm, buffer);
        }
        case OBJ_NATIVE_CLASS: {
            ObjNativeClass* _class = ValueOP::as_native_class(value);
            switch(_class->subType) {
                case NATIVE_COLLECTION:
                    return ObjString::copyString(vm, "Collection Class");
                default:
                    return ObjString::copyString(vm, "Native Class");
            }
        }
        case OBJ_NATIVE_INSTANCE: {
            ObjNativeInstance* instance = ValueOP::as_native_instance(value);
            switch (instance->subType) {
                case NATIVE_COLLECTION_INSTANCE: {
                    ObjCollectionInstance* collection = static_cast<ObjCollectionInstance*>(instance);
                    std::string buffer;
                    for(int i = 0; i < collection->values.count; i++) {
                        vm->push_stack(ValueOP::obj_val(to_string(collection->values.values[i], vm)));
                        buffer += as_string(vm->peek(0))->chars;
                        vm->stack.pop_back();
                    }
                    
                    return ObjString::copyString(vm, buffer);
                }
                default:
                    return ObjString::copyString(vm, "Native Instance");
            }
        }
        default: {
            Obj* object = as_obj(value);
            std::stringstream ss;
            ss << ((void *)object);
            return ObjString::copyString(vm, ss.str());
        }
    }
}

ObjString* ValueOP::function_to_string(ObjFunction* function, VM* vm) {
    if(function->name == nullptr) {
        return ObjString::copyString(vm, "<script>");
    }
    
    std::string buffer = "<fn ";
    buffer += std::string(function->name->chars);
    buffer += ">";
    return ObjString::copyString(vm, buffer.c_str());
}

ObjNativeInstance* ValueOP::as_native_instance(Value value) {
    return (ObjNativeInstance*)as_obj(value);
}

bool ValueOP::is_native_instance(Value value) {
    return as_obj(value)->type == OBJ_NATIVE_INSTANCE;
}

bool ValueOP::is_native_subinstance(Value value, NativeInstanceType type) {
    return is_native_instance(value) && as_native_instance(value)->subType == type;
}

bool ValueOP::is_native_method(Value value) {
    return as_obj(value)->type == OBJ_NATIVE_CLASS_METHOD;
}

ObjNativeClassMethod* ValueOP::as_native_class_method(Value value) {
    return static_cast<ObjNativeClassMethod*>(as_obj(value));
}
