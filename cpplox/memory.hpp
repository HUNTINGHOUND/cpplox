#ifndef memory_h
#define memory_h

#include <vector>
#include <iostream>
#include "vm.hpp"
#include "object.hpp"

class ValueArray;

//central allocation method that helps keep track of memory usage
void* reallocate(void* pointer, size_t oldsize, size_t newsize, VM* vm);
/*
 oldSize           newSize           Operation
 0                 Non-zero          Allocate new block.
 Non-zero          0                 Free block.
 Non-zero          < oldSize         Shrink block.
 Non-zero          > oldSize         Expand block.
 */

//Free objects from heap space
void freeObjects(VM* vm);

void freeObject(Obj* object, VM* vm);


//return the expanded capacity size
inline size_t grow_capacity(size_t capacity) {
    return capacity < 8 ? 8 : capacity * 2;
}

//grow an array
template <typename V>
inline V* grow_array(V* pointer, size_t oldCount, size_t newCount, VM* vm) {
    return (V*)reallocate(pointer, sizeof(V) * (oldCount), sizeof(V) * newCount, vm);
}

//free an array
template <typename V>
inline void free_array(V* pointer, size_t oldCount, VM* vm) {
    reallocate(pointer, sizeof(V) * (oldCount), 0, vm);
}

template<typename T>
inline T* allocate(size_t count, VM* vm) {
    return (T*)reallocate(nullptr, 0, sizeof(T) * count, vm);
}

void markValue(VM* vm, Value value);
void markObject(VM* vm, Obj* object);
void markGlobal(Table* table, VM* vm);
void markCompilerRoots(Compiler* compiler);
void markTable(VM* vm, Table* table);
void traceReferences(VM* vm);
void blackenObject(Obj* object, VM* vm);
void markArray(VM* vm, ValueArray* array);
void sweep(VM* vm);

class GarbageCollector {
    static void markRoots(VM* vm);
public:
    static void collectGarbage(VM* vm);
};

#endif /* memory_h */
