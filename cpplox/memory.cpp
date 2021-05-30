#include "memory.hpp"
#include "chunk.hpp"
#include "debug.hpp"
#include "compiler.hpp"
#include "valuearray.hpp"


#ifdef NAN_BOXING
#include "nanvalue.hpp"
#else
#include "value.hpp"
#endif

void* reallocate(void* pointer, size_t oldsize, size_t newsize, VM* vm) {
    vm->bytesAllocated += newsize - oldsize;
    
    if(newsize > oldsize) {
#ifdef DEBUG_STRESS_GC
        GarbageCollector::collectGarbage(vm);
#endif
    }
    
    if(vm->bytesAllocated > vm->nextGC) {
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
    std::cout << (void*)object << " free type " << object->type << std::endl;
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
            break;
        }
        case OBJ_NATIVE:
            reallocate(object, sizeof(ObjNative), 0, vm);
            break;
        case OBJ_UPVALUE:
            reallocate(object, sizeof(ObjUpvalue), 0, vm);
            break;
        case OBJ_CLOSURE: {
            ObjClosure* closure = (ObjClosure*) object;
            free_array<ObjUpvalue*>(closure->upvalues, closure->upvalueCount, vm);
            reallocate(object, sizeof(ObjClosure), 0, vm);
            break;
        }
        case OBJ_CLASS: {
            ObjClass* _class = (ObjClass*)object;
            _class->methods.freeTable();
            reallocate(object, sizeof(ObjClass),0,vm);
            break;
        }
        case OBJ_INSTANCE: {
            ObjInstance* instance = (ObjInstance*)object;
            instance->fields.freeTable();
            reallocate(object, sizeof(ObjInstance), 0, vm);
            break;
        }
        case OBJ_BOUND_METHOD: {
            reallocate(object, sizeof(ObjBoundMethod), 0, vm);
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
    vm->strings.removeWhite(vm);
    sweep(vm);
    
    vm->nextGC = vm->bytesAllocated * GC_HEAP_GROW_FACTOR;
    
    vm->marker = !vm->marker;
    
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
    
    for(ObjUpvalue* upvalue = vm->openUpvalues; upvalue != nullptr; upvalue = upvalue->nextUp) {
        markObject(vm, (Obj*)upvalue);
    }
    
    markGlobal(&vm->globalNames, vm);
    if(vm->current != nullptr) vm->current->markCompilerRoots();
    markObject(vm, (Obj*)vm->initString);
}

void markGlobal(Table* table, VM* vm) {
    for(int i = 0; i < table->capacity; i++) {
        Entry* entry = &table->entries[i];
        if(ValueOP::is_obj(entry->key)) {
            markObject(vm, ValueOP::as_obj(entry->key));
            markValue(vm, vm->globalValues.values[(int)ValueOP::as_number(entry->value)]);
        }
    }
}

void markValue(VM* vm, Value value) {
    if(!ValueOP::is_obj(value)) return;
    markObject(vm, ValueOP::as_obj(value));
}

void markObject(VM* vm, Obj* object) {
    if(object == nullptr) return;
    if(object->mark == vm->marker) return;
    object->mark = vm->marker;
    
    vm->grayStack.push_back(object);
    
#ifdef DEBUG_LOG_GC
    std::cout << (void*)object << " mark ";
    ValueOP::printValue(ValueOP::obj_val(object));
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
#ifdef DEBUG_LOG_GC
    std::cout << (void*)object << " blacken ";
    ValueOP::printValue(ValueOP::obj_val(object));
    std::cout << std::endl;
#endif
    
    switch (object->type) {
        case OBJ_COLLECTION: {
            ObjCollection* collection = (ObjCollection*)object;
            for(int i = 0; i < collection->size; i++) {
                markValue(vm, collection->values[i]);
            }
            markTable(vm, &collection->methods);
        }
        case OBJ_BOUND_METHOD: {
            ObjBoundMethod* bound = (ObjBoundMethod*)object;
            markValue(vm, bound->receiver);
            markObject(vm, bound->method);
            break;
        }
        case OBJ_INSTANCE: {
            ObjInstance* instance = (ObjInstance*)object;
            markObject(vm, (Obj*)instance->_class);
            markTable(vm, &instance->fields);
        }
        case OBJ_CLASS: {
            ObjClass* _class = (ObjClass*)object;
            markObject(vm, (Obj*)_class->name);
            markTable(vm, &_class->methods);
            break;
        }
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

void markTable(VM* vm, Table* table) {
    for(int i = 0; i < table->capacity; i++) {
        Entry* entry = &table->entries[i];
        markValue(vm, entry->key);
        markValue(vm, entry->value);
    }
}

void sweep(VM* vm) {
    Obj* previous = nullptr;
    Obj* object = vm->objects;
    while(object != nullptr) {
        if(object->mark == vm->marker) {
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
