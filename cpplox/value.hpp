//TODO: Finish Documentation

#ifndef value_h
#define value_h

#include <vector>
#include "object.hpp"

enum ValueType{
    VAL_BOOL,
    VAL_NUL,
    VAL_NUMBER,
    VAL_OBJ,
    VAL_EMPTY
};

class Value {
    
    static uint32_t hashDouble(double value);
    
public:
    ValueType type;
    bool isConst = false;
    union {
        bool boolean;
        double number;
        Obj* obj;
    } as;
    
    static bool is_bool(Value value);
    static bool is_nul(Value value);
    static bool is_number(Value value);
    static bool is_obj(Value value);
    static bool is_string(Value value);
    static bool is_empty(Value value);
    static bool isObjType(Value value, ObjType type);
    
    static bool as_bool(Value value);
    static double as_number(Value value);
    static Obj* as_obj(Value value);
    static ObjString* as_string(Value value);
    static char* as_c_string(Value value);

    static Value bool_val(bool value);
    static Value nul_val();
    static Value number_val(double value);
    static Value obj_val(Obj* obj);
    static Value empty_val();
    
    static bool valuesEqual(Value a, Value b);
    
    static ObjType obj_type(Value value);
    static void printObject(Value value);
    static void printValue(Value value);
    static uint32_t hashValue(Value value);
};


class ValueArray {
public:
    int count;
    std::vector<Value> values;
    
    ValueArray();
    void writeValueArray(Value value);
    void freeValueArray();
    
};


#endif /* value_h */
