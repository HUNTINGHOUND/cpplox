#ifndef memory_h
#define memory_h

#include <vector>
#include "vm.hpp"

//central allocation method that helps keep track of memory usage
void* reallocate(void* pointer, size_t oldsize, size_t newsize);
/*
 oldSize           newSize           Operation
 0                 Non-zero          Allocate new block.
 Non-zero          0                 Free block.
 Non-zero          < oldSize         Shrink block.
 Non-zero          > oldSize         Expand block.
 */

//Free objects from heap space
void freeObjects(VM* vm);

void freeObject(Obj* object);


//return the expanded capacity size
inline size_t grow_capacity(size_t capacity) {
    return capacity < 8 ? 8 : capacity * 2;
}

//grow an array
template <typename V>
inline V* grow_array(V* pointer, size_t oldCount, size_t newCount) {
    return (V*)reallocate(pointer, sizeof(V) * (oldCount), sizeof(V) * newCount);
}

//free an array
template <typename V>
inline void free_array(V* pointer, size_t oldCount) {
    reallocate(pointer, sizeof(V) * (oldCount), 0);
}

template<typename T>
inline T* allocate(int count) {
    return (T*)reallocate(nullptr, 0, sizeof(T) * count);
}


#endif /* memory_h */
