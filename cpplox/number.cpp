#include "number.hpp"

Number Number::gen_whole_num(long long whole) {
    Number num;
    num.is_float = false;
    num.number.whole = whole;
    return num;
}

Number Number::gen_float_num(double decimal) {
    Number num;
    num.is_float = false;
    num.number.decimal = decimal;
    return num;
}

bool operator==(Number const& lhs, Number const& rhs) {
    if(lhs.is_float != rhs.is_float) return false;
    return lhs.is_float ? lhs.number.decimal == rhs.number.decimal : lhs.number.whole == rhs.number.whole;
}

bool operator< (Number const& lhs, Number const& rhs) {
    return (lhs.is_float ? lhs.number.decimal : rhs.number.whole) < (rhs.is_float ? rhs.number.decimal : rhs.number.whole);
}

bool operator> (Number const& lhs, Number const& rhs) {
    return rhs < lhs;
}

bool operator<= (Number const& lhs, Number const& rhs) {
    return !(lhs > rhs);
}

bool operator>= (Number const& lhs, Number const& rhs) {
    return !(lhs < rhs);
}

Number operator+ (Number const& lhs, Number const& rhs) {
    Number num;
    if(lhs.is_float || rhs.is_float) {
        num.is_float = true;
        num.number.decimal = (lhs.is_float ? lhs.number.decimal : lhs.number.whole) + (rhs.is_float ? rhs.number.decimal : rhs.number.whole);
    } else {
        num.is_float = false;
        num.number.whole = lhs.number.whole + rhs.number.whole;
    }
    
    return num;
}

Number operator- (Number const& lhs, Number const& rhs) {
    Number num;
    if(lhs.is_float || rhs.is_float) {
        num.is_float = true;
        num.number.decimal = (lhs.is_float ? lhs.number.decimal : lhs.number.whole) - (rhs.is_float ? rhs.number.decimal : rhs.number.whole);
    } else {
        num.is_float = false;
        num.number.whole = lhs.number.whole - rhs.number.whole;
    }
    
    return num;
}

Number operator* (Number const& lhs, Number const& rhs) {
    Number num;
    if(lhs.is_float || rhs.is_float) {
        num.is_float = true;
        num.number.decimal = (lhs.is_float ? lhs.number.decimal : lhs.number.whole) * (rhs.is_float ? rhs.number.decimal : rhs.number.whole);
    } else {
        num.is_float = false;
        num.number.whole = lhs.number.whole * rhs.number.whole;
    }
    
    return num;
}

Number operator- (Number const& rhs) {
    Number num = rhs;
    if(num.is_float) num.number.decimal = -num.number.decimal;
    else num.number.whole = -num.number.whole;
    
    return num;
}

Number operator/ (Number const& lhs, Number const& rhs) {
    Number num;
    num.is_float = true;
    num.number.decimal = ((lhs.is_float ? lhs.number.decimal : lhs.number.whole) * 1.0) /( rhs.is_float ? rhs.number.decimal : rhs.number.whole);
    return num;
}

std::ostream& operator<<(std::ostream& os, Number const& num) {
    os << (num.is_float ? num.number.decimal : num.number.whole);
    return os;
}

Number std::abs(Number const & n) {
    if (n < 0) return -n;
    else return n;
}
