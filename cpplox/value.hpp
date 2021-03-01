

//TODO: Finish Documentation

#ifndef value_h
#define value_h

#include <vector>

enum ValueType{
    VAL_BOOL,
    VAL_NUL,
    VAL_NUMBER
};

struct Value {
    ValueType type;
    union {
        bool boolean;
        double number;
    } as;
};

bool is_bool(Value value);
bool is_nul(Value value);
bool is_number(Value value);

bool as_bool(Value value);
double as_number(Value value);

Value bool_val(bool value);
Value nul_val();
Value number_val(double value);

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
