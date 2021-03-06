

//TODO: Finish Documentation

#ifndef value_h
#define value_h

#include <vector>

enum ValueType{
    VAL_BOOL,
    VAL_NUL,
    VAL_NUMBER
};

class Value {
public:
    ValueType type;
    union {
        bool boolean;
        double number;
    } as;
    
    static bool is_bool(Value value);
    static bool is_nul(Value value);
    static bool is_number(Value value);

    static bool as_bool(Value value);
    static double as_number(Value value);

    static Value bool_val(bool value);
    static Value nul_val();
    static Value number_val(double value);
    static bool valuesEqual(Value a, Value b);
    
};


class ValueArray {
public:
    int count;
    std::vector<Value> values;
    
    ValueArray();
    void writeValueArray(Value value);
    void freeValueArray();
    
    static void printValue(Value value);
};


#endif /* value_h */
