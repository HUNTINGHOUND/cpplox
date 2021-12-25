//TODO: Finish Documentation

#ifndef value_h
#define value_h

#include "pch.pch"

class Obj;
class ObjString;
class ObjFunction;
class ObjNative;
class ObjClosure;
class ObjClass;
class ObjInstance;
class ObjBoundMethod;
class ObjNativeClass;
class ObjCollectionClass;
class ObjNativeInstance;
class ObjCollectionInstance;
class ObjNativeClassMethod;
enum ObjType : short;
class VM;

#ifdef NAN_BOXING

using Value = uint64_t;

#else


enum ValueType{
    VAL_BOOL,
    VAL_NUL,
    VAL_NUMBER,
    VAL_OBJ,
    VAL_EMPTY
};

class Value {
public:
    ValueType type;
    bool isConst = false;
    union {
        bool boolean;
        double number;
        Obj* obj;
    } as;
    
};

#endif

namespace ValueOP {
uint32_t hashDouble(double value);

bool is_bool(Value value);
bool is_nul(Value value);
bool is_number(Value value);
bool is_obj(Value value);
bool is_string(Value value);
bool is_empty(Value value);
bool is_function(Value value);
bool is_native(Value value);
bool is_closure(Value value);
bool is_class(Value value);
bool is_instance(Value value);
bool is_bound_method(Value value);
bool is_native_class(Value value);
bool is_native_subclass(Value value, ObjType type);
bool is_native_instance(Value value);
bool is_native_subinstance(Value value, ObjType type);
bool is_native_method(Value value);
bool isObjType(Value value, ObjType type);
bool isConst(Value value);

bool as_bool(Value value);
double as_number(Value value);
Obj* as_obj(Value value);
ObjString* as_string(Value value);
std::string* as_std_string(Value value);
ObjFunction* as_function(Value value);
ObjNative* as_native(Value value);
ObjClosure* as_closure(Value value);
ObjClass* as_class(Value value);
ObjInstance* as_instance(Value value);
ObjBoundMethod* as_bound_method(Value value);
ObjNativeClass* as_native_class(Value value);
template <typename T>
T* as_native_subclass(Value value) {
    return (T*)as_native_class(value);
}
ObjNativeInstance* as_native_instance(Value value);
template <typename T>
T* as_native_subinstance(Value value) {
    return (T*)as_native_instance(value);
}
ObjNativeClassMethod* as_native_class_method(Value value);

ObjString* to_string(Value value, VM* vm);
ObjString* object_to_string(Value object, VM* vm);
ObjString* function_to_string(ObjFunction* function, VM* vm);

Value bool_val(bool value);
Value nul_val();
Value number_val(double value);
Value obj_val(Obj* obj);
Value empty_val();

bool valuesEqual(Value a, Value b);

ObjType obj_type(Value value);
void printObject(Value value);
void printValue(Value value);
void printFunction(ObjFunction* function);
uint32_t hashValue(Value value);

ObjFunction* get_value_function(Value value);
ObjFunction* get_obj_function(Obj* obj);
void setConst(bool isCon, Value value);
}

#endif /* value_h */
