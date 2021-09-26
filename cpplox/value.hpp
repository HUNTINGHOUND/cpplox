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
class ObjCollection;
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
bool is_collection(Value value);
bool isObjType(Value value, ObjType type);
bool isConst(Value value);

bool as_bool(Value value);
double as_number(Value value);
Obj* as_obj(Value value);
ObjString* as_string(Value value);
char* as_c_string(Value value);
ObjFunction* as_function(Value value);
ObjNative* as_native(Value value);
ObjClosure* as_closure(Value value);
ObjClass* as_class(Value value);
ObjInstance* as_instance(Value value);
ObjBoundMethod* as_bound_method(Value value);
ObjCollection* as_collection(Value value);

ObjString* to_string(Value value, VM* vm);

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
