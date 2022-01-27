#ifndef util_hpp
#define util_hpp

#include "pch.pch"

std::string readFile(const char* path);

template <typename T>
struct is_built_in : std::bool_constant<std::is_integral<T>::value || std::is_floating_point<T>::value> {};

template <typename T>
inline constexpr auto is_built_in_v = is_built_in<T>::value;

#endif /* util_hpp */
