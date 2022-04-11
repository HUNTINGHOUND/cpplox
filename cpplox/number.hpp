#ifndef number_hpp
#define number_hpp

#include "pch.pch"
#include "util.hpp"

struct Number {
    bool is_float;
    union {
        long long whole;
        double decimal;
    } number;
    
    Number()=default;
    template<typename T>
    Number(T val) {
        static_assert(is_built_in_v<T>, "Must be built-in type to initialize Number");
        if(std::is_floating_point_v<T>) {
            is_float = true;
            number.decimal = static_cast<double>(val);
        } else {
            is_float = false;
            number.whole = static_cast<long long>(val);
        }
    }
    
    static Number gen_whole_num(long long whole);
    static Number gen_float_num(double decimal);
    
    template <typename T>
    static T cast_to(Number const & t) {
        static_assert(is_built_in_v<T>, "Must cast to built in type");
        return (T)(t.is_float ? t.number.decimal : t.number.whole);
    }
};

std::ostream& operator<<(std::ostream& os, Number const& num);

bool operator== (Number const& lhs, Number const& rhs);
bool operator< (Number const& lhs, Number const& rhs);
bool operator> (Number const& lhs, Number const& rhs);
bool operator<= (Number const& lhs, Number const& rhs);
bool operator>= (Number const& lhs, Number const& rhs);

Number operator+ (Number const& lhs, Number const& rhs);
Number operator- (Number const& lhs, Number const& rhs);
Number operator* (Number const& lhs, Number const& rhs);
Number operator/ (Number const& lhs, Number const& rhs);
Number operator- (Number const& rhs);

namespace std {
Number abs(Number const & n);
};

#endif /* number_hpp */
