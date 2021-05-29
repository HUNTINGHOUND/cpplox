

#ifndef nanvalue_h
#define nanvalue_h

#include <vector>
#include <string.h>

class Obj;
class ObjString;
class ObjFunction;
class ObjNative;
class ObjClosure;
class ObjClass;
class ObjInstance;
class ObjBoundMethod;
enum ObjType : short;
class VM;


const uint64_t qnan = 0x7ffc000000000000;
const uint64_t tag_nul = 1;
const uint64_t tag_false = 2;
const uint64_t tag_true = 3;
const uint64_t sign_bit = 0x8000000000000000;
const uint64_t const_tag = 0x2000000000000;

using Value = uint64_t;


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

Value true_val();
Value false_val();
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





#endif /* nanvalue_h */
