#ifndef memory_h
#define memory_h

#include <vector>

//central allocation method that helps keep track of memory usage
template <typename T>
extern void reallocate(std::vector<T>& array, size_t oldsize, size_t newsize);
/*
 oldSize           newSize           Operation
 0                 Non-zero          Allocate new block.
 Non-zero          0                 Free block.
 Non-zero          < oldSize         Shrink block.
 Non-zero          > oldSize         Expand block.
 */

//return the expanded capacity size
inline size_t grow_capacity(size_t capacity) {
    return capacity < 8 ? 8 : capacity * 2;
}

//grow an array
template <typename V>
inline void grow_array(std::vector<V>& array, size_t oldCount, size_t newCount) {
    reallocate<V>(array, oldCount, newCount);
}

//free an array
template <typename V>
inline void free_array(std::vector<V>& array, size_t oldCount) {
    reallocate<V>(array, oldCount, 0);
}

#endif /* memory_h */
