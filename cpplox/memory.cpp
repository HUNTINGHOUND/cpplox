#include "memory.hpp"
#include "chunk.hpp"
#include "debug.hpp"
#include "compiler.hpp"
#include "valuearray.hpp"
#include "flags.hpp"


#ifdef NAN_BOXING
#include "nanvalue.hpp"
#else
#include "value.hpp"
#endif

void freeObjects(VM* vm) {
    Obj* object = vm->objects;
    while (object != nullptr) {
        Obj* next = object->next;
        freeObject(object, vm);
        object = next;
    }
}

void freeObject(Obj* object, VM* vm) {
    if(DEBUG_LOG_GC) {
        std::cout << (void*)object << " free type ";
    }
    
    switch (object->type) {
        case OBJ_STRING:
            mem_deallocate<ObjString>((ObjString*)object, sizeof(ObjString), vm);
            if(DEBUG_LOG_GC) std::cout << "OBJ_STRING" << std::endl;
            break;
        case OBJ_FUNCTION:
            mem_deallocate<ObjFunction>((ObjFunction*)object, sizeof(ObjFunction), vm);
            if(DEBUG_LOG_GC) std::cout << "OBJ_FUNCTION" << std::endl;
            break;
        case OBJ_NATIVE:
            mem_deallocate<ObjNative>((ObjNative*)object, sizeof(ObjNative), vm);
            if(DEBUG_LOG_GC) std::cout << "OBJ_NATIVE" << std::endl;
            break;
        case OBJ_UPVALUE:
            mem_deallocate<ObjUpvalue>((ObjUpvalue*)object, sizeof(ObjUpvalue), vm);
            if(DEBUG_LOG_GC) std::cout << "OBJ_UPVALUE" << std::endl;
            break;
        case OBJ_CLOSURE:
            mem_deallocate<ObjClosure>((ObjClosure*)object, sizeof(ObjClosure), vm);
            if(DEBUG_LOG_GC) std::cout << "OBJ_CLOSURE" << std::endl;
            break;
        case OBJ_CLASS:
            mem_deallocate<ObjClass>((ObjClass*)object, sizeof(ObjClass), vm);
            if(DEBUG_LOG_GC) std::cout << "OBJ_CLASS" << std::endl;
            break;
        case OBJ_INSTANCE:
            mem_deallocate<ObjInstance>((ObjInstance*)object, sizeof(ObjInstance), vm);
            if(DEBUG_LOG_GC) std::cout << "OBJ_INSTANCE" << std::endl;
            break;
        case OBJ_BOUND_METHOD:
            mem_deallocate<ObjBoundMethod>((ObjBoundMethod*)object, sizeof(ObjBoundMethod), vm);
            if(DEBUG_LOG_GC) std::cout << "OBJ_BOUND_METHOD" << std::endl;
            break;
    }
}

void GarbageCollector::collectGarbage(VM* vm) {
    if(DEBUG_LOG_GC) {
        std::cout << "-- gc begin" << std::endl;
    }
    
    size_t before = vm->bytesAllocated;
    
    markRoots(vm);
    traceReferences(vm);
    vm->strings.removeWhite(vm);
    sweep(vm);
    
    vm->nextGC = vm->bytesAllocated * GC_HEAP_GROW_FACTOR;
    
    vm->marker = !vm->marker;
    
    if(DEBUG_LOG_GC) {
        std::cout << "-- gc end" << std::endl;
        std::cout << "   collected " << before - vm->bytesAllocated << " bytes (from " << before << " to " << vm->bytesAllocated << ") next at " << vm->nextGC << std::endl;
    }
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
    for(int i = 0; i < table->entries.size(); i++) {
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
    
    if(DEBUG_LOG_GC) {
        std::cout << (void*)object << " mark ";
        ValueOP::printValue(ValueOP::obj_val(object));
        std::cout << std::endl;
    }
}

void traceReferences(VM* vm) {
    while (vm->grayStack.size() > 0) {
        Obj* object = vm->grayStack.back();
        vm->grayStack.pop_back();
        blackenObject(object, vm);
    }
}

void blackenObject(Obj* object, VM* vm) {
    if(DEBUG_LOG_GC) {
        std::cout << (void*)object << " blacken ";
        ValueOP::printValue(ValueOP::obj_val(object));
        std::cout << std::endl;
    }
    
    switch (object->type) {
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
    for(int i = 0; i < table->entries.size(); i++) {
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
