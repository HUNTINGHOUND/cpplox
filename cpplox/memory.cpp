#include "memory.hpp"
#include "value.hpp"
#include "chunk.hpp"
#include "debug.hpp"
#include "compiler.hpp"
#include <iostream>

void* reallocate(void* pointer, size_t oldsize, size_t newsize, VM* vm) {
    vm->byteAllocated += newsize - oldsize;
    
    if(newsize > oldsize) {
#ifdef DEBUG_STRESS_GC
        GarbageCollector::collectGarbage(vm);
#endif
    }
    
    if(vm->byteAllocated > vm->nextGC) {
        GarbageCollector::collectGarbage(vm);
    }
    
    if (newsize == 0) {
        if(oldsize != 0) {
            free(pointer);
        }
        return nullptr;
    }
    
    void* result;
    
    if(oldsize == 0) {
        result = malloc(newsize);
    } else {
        result = realloc(pointer, newsize);
    }
    return result;
}

void freeObjects(VM* vm) {
    Obj* object = vm->objects;
    while (object != nullptr) {
        Obj* next = object->next;
        freeObject(object, vm);
        object = next;
    }
}

void freeObject(Obj* object, VM* vm) {
#ifdef DEBUG_LOG_GC
    std::cout << (void*)object << "free type " << object->type << std::endl;
#endif
    
    switch (object->type) {
        case OBJ_STRING: {
            ObjString* string = (ObjString*) object;
            reallocate(object, sizeof(ObjString) + string->length + 1, 0, vm);
            break;
        }
        case OBJ_FUNCTION: {
            ObjFunction* function = (ObjFunction*)object;
            function->chunk.freeChunk();
            reallocate(object, sizeof(ObjFunction), 0, vm);
        }
        case OBJ_NATIVE:
            reallocate(object, sizeof(ObjNative), 0, vm);
            break;
        case OBJ_UPVALUE:
            reallocate(object, sizeof(ObjUpvalue), 0, vm);
            break;
        case OBJ_CLOSURE: {
            ObjClosure* closure = (ObjClosure*) object;
            free_array<ObjUpvalue*>(closure->upvalues, closure->upvalueCount);
            reallocate(object, sizeof(ObjClosure), 0, vm);
            break;
        }
    }
}

void GarbageCollector::collectGarbage(VM* vm) {
#ifdef DEBUG_LOG_GC
    std::cout << "-- gc begin" << std::endl;
    size_t before = vm->bytesAllocated;
#endif
    
    markRoots(vm);
    traceReferences(vm);
    vm->strings.removeWhite();
    sweep(vm);
    
    vm->nextGC = vm->byteAllocated * GC_HEAP_GROW_FACTOR;
    
#ifdef DEBUG_LOG_GC
    std::cout << "-- gc end" << std::endl;
    std::cout << "   collected " << before - vm->bytesAllocated << " bytes (from " << before << " to " << vm->bytesAllocated << ") next at " << vm->nextGC << std::endl;
#endif
}

void GarbageCollector::markRoots(VM* vm) {
    for (auto slot = vm->stack.begin(); slot < vm->stack.end(); slot++) {
        markValue(vm, *slot);
    }
    
    for(int i = 0; i < vm->frames.size(); i++) {
        markObject(vm, vm->frames[i].function);
    }
    
    for(ObjUpvalue* upvalue = vm->openUpvalues; upvalue != nullptr; upvalue = upvalue->next) {
        markObject(vm, (Obj*)upvalue);
    }
    
    markGlobal(&vm->globalNames, vm);
    vm->current->markCompilerRoots();
}

void markGlobal(Table* table, VM* vm) {
    for(int i = 0; i < table->capacity; i++) {
        Entry* entry = &table->entries[i];
        if(Value::is_obj(entry->key)) {
            markObject(vm, Value::as_obj(entry->key));
        }
        markValue(vm, vm->globalValues.values[(int)Value::as_number(entry->value)]);
    }
}

void markValue(VM* vm, Value value) {
    if(!Value::is_obj(value)) return;
    markObject(vm, Value::as_obj(value));
}

void markObject(VM* vm, Obj* object) {
    if(object == nullptr) return;
    if(object->isMarked) return;
    object->isMarked = true;
    
    vm->grayStack.push_back(object);
    
#ifdef DEBUG_LOG_GC
    std::cout << (void*)object << " mark ";
    Value::printValue(Value::obj_val(object));
    std::cout << std::endl;
#endif
}

void traceReferences(VM* vm) {
    while (vm->grayStack.size() > 0) {
        Obj* object = vm->grayStack.back();
        vm->grayStack.pop_back();
        blackenObject(object, vm);
    }
}

void blackenObject(Obj* object, VM* vm) {
#ifdef DEBUG_LOB_GC
    std::cout << (void*)object << " blacken ";
    Value::printValue(Value::obj_val(object));
    std::cout << std::endl;
#endif
    
    switch (object->type) {
        case OBJ_CLOSURE: {
            ObjClosure* closure = (ObjClosure*)object;
            markObject(vm, (Obj*)closure->function);
            for(int i = 0; i < closure->upvalueCount; i++) {
                markObject(vm, (Obj*)closure->upvalues[i]);
            }
            break;
        }
        case OBJ_FUNCTION: {
            ObjFunction* function = (ObjFunction*)object;
            markObject(vm, (Obj*)function->name);
            markArray(vm, &function->chunk.constants);
            break;
        }
        case OBJ_UPVALUE:
            markValue(vm, ((ObjUpvalue*)object)->closed);
            break;
        case OBJ_NATIVE:
        case OBJ_STRING:
            break;
    }
}

void markArray(VM* vm, ValueArray* array) {
    for(int i = 0; i < array->count; i++) {
        markValue(vm, array->values[i]);
    }
}

void sweep(VM* vm) {
    Obj* previous = nullptr;
    Obj* object = vm->objects;
    while(object != nullptr) {
        if(object->isMarked) {
            object->isMarked = false;
            previous = object;
            object = object->next;
        } else {
            Obj* unreached = object;
            object = object->next;
            if (previous != nullptr) {
                previous->next = object;
            } else {
                vm->objects = object;
            }
            
            freeObject(unreached, vm);
        }
    }
}
