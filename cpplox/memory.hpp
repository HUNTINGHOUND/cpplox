#ifndef memory_h
#define memory_h

#include "pch.pch"
#include "vm.hpp"
#include "object.hpp"
#include "flags.hpp"

class ValueArray;

//Free objects from heap space
void freeObjects(VM* vm);

void freeObject(Obj* object, VM* vm);

//return the expanded capacity size
inline size_t grow_capacity(size_t capacity) {
    return capacity < 8 ? 8 : capacity * 2;
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

//central allocation method that helps keep track of memory usage
template<typename V>
V* mem_allocate(size_t newsize, VM* vm) {
    vm->bytesAllocated += newsize;
    
    if(DEBUG_STRESS_GC) {
        GarbageCollector::collectGarbage(vm);
    }
    
    if(vm->bytesAllocated > vm->nextGC) {
        GarbageCollector::collectGarbage(vm);
    }
    
    V* result = new V;
    return result;
}

//Central free method to help keep track of memory usage
template<typename V>
void mem_deallocate(V* pointer, size_t oldsize, VM* vm) {
    vm->bytesAllocated -= oldsize;
    
    delete pointer;
}

template<typename T>
inline T* allocate(size_t count, VM* vm) {
    return mem_allocate<T>(sizeof(T) * count, vm);
}

#endif /* memory_h */
