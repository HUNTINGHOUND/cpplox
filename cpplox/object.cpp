#include "object.hpp"
#include "memory.hpp"
#include "vm.hpp"
#include "table.hpp"
#include "chunk.hpp"
#include "flags.hpp"
#include "debug.hpp"


uint32_t ObjString::hashString(const char* key, size_t length) {
    uint32_t hash = 2166136261u;
    
    for (int i = 0; i < length; i++) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }
    
    return hash;
}


template<typename T>
T* Obj::allocate_obj(ObjType objectType, VM* vm) {
    Obj* object = (Obj* )mem_allocate<T>(sizeof(T), vm);
    object->type = objectType;
    object->mark = !vm->marker;
    
    object->next = vm->objects;
    vm->objects = object;
    if(DEBUG_LOG_GC) {
        std::cout << (void*)object << " allocate " << sizeof(T) << " for " << objectType << std::endl;
    }
    
    return (T*)object;
}

ObjString* ObjString::makeString(VM* vm, size_t length, uint32_t hash) {
    ObjString* string = Obj::allocate_obj<ObjString>(OBJ_STRING, vm);
    string->hash = hash;
    
    vm->stack.push_back(ValueOP::obj_val(string));
    
    vm->strings.tableSet(ValueOP::obj_val(string), ValueOP::nul_val());
    
    vm->stack.pop_back();
    
    return string;
}

ObjString* ObjString::copyString(VM* vm, const char* chars, size_t length) {
    uint32_t hash = hashString(chars, length);
    ObjString* interned = vm->strings.tableFindString(chars, length,
                                                      hash);
    
    if (interned != nullptr) {
        return interned;
    }
    
    ObjString* string = makeString(vm, length, hash);
    string->chars = std::string(chars, length);
    
    return string;
}

ObjFunction* ObjFunction::newFunction(VM* vm) {
    ObjFunction* function = allocate_obj<ObjFunction>(OBJ_FUNCTION, vm);
    
    function->arity = 0;
    function->defaults = 0;
    function->upvalueCount = 0;
    function->name = nullptr;
    function->chunk = Chunk(vm);
    return function;
}

ObjNative* ObjNative::newNative(NativeFn function, int arity, VM* vm) {
    ObjNative* native = allocate_obj<ObjNative>(OBJ_NATIVE, vm);
    native->function = function;
    native->arity = arity;
    return native;
}

ObjClosure* ObjClosure::newClosure(ObjFunction* function, VM* vm) {
    ObjClosure* closure = allocate_obj<ObjClosure>(OBJ_CLOSURE, vm);
    closure->function = function;
    closure->upvalues = std::vector<ObjUpvalue*>(function->upvalueCount, nullptr);
    closure->upvalueCount = function->upvalueCount;
    return closure;
}

ObjUpvalue* ObjUpvalue::newUpvalue(Value* slot, VM* vm) {
    ObjUpvalue* upvalue = allocate_obj<ObjUpvalue>(OBJ_UPVALUE, vm);
    upvalue->nextUp = nullptr;
    upvalue->location = slot;
    upvalue->closed = ValueOP::nul_val();
    return upvalue;
}

ObjClass* ObjClass::newClass(ObjString* name, VM* vm) {
    ObjClass* _class = Obj::allocate_obj<ObjClass>(OBJ_CLASS, vm);
    _class->name = name;
    _class->methods = Table(vm);
    _class->initializer = nullptr;
    return _class;
}

ObjInstance* ObjInstance::newInstance(ObjClass *_class, VM* vm) {
    ObjInstance* instance = allocate_obj<ObjInstance>(OBJ_INSTANCE, vm);
    instance->_class = _class;
    instance->fields = Table(vm);
    return instance;
}

ObjBoundMethod* ObjBoundMethod::newBoundMethod(Value receiver, Obj *method, VM* vm) {
    ObjBoundMethod* bound = allocate_obj<ObjBoundMethod>(OBJ_BOUND_METHOD, vm);
    bound->method = method;
    bound->receiver = receiver;
    return bound;
}

ObjCollection* ObjCollection::newCollection(Value *values, size_t size, size_t capacity, VM* vm) {
    ObjCollection* collection = allocate_obj<ObjCollection>(OBJ_COLLECTION, vm);
    collection->values = ValueArray(vm);
    collection->vm = vm;
    collection->methods = Table(vm);
    return collection;
}

CollectionResponse ObjCollection::invokeCollectionMethods(ObjString* method, std::vector<Value>& arguments) {
    CollectionResponse response;
    response.hasErr = false;
    if(method->chars.compare("addBack") == 0) {
        if(arguments.size() != 1) {
            response.hasErr = true;
            response.propertyMissing = false;
            
            response.errorMessage = "addBack method expects 1 argument.";
            return response;
        } else {
            response.isVoid = true;
            addBack(arguments[0]);
        }
    } else if (method->chars.compare("deleteBack") == 0) {
        if(arguments.size() != 0) {
            response.hasErr = true;
            response.propertyMissing = false;
            
            response.errorMessage = "deleteBack method expects 0 argument.";
            return response;
        } else {
            response.isVoid = true;
            deleteBack();
        }
    } else if (method->chars.compare("swap") == 0) {
        if(arguments.size() != 2) {
            response.hasErr = true;
            response.propertyMissing = false;
            
            response.errorMessage = "swap method expects 2 argument.";
            return response;
        } else {
            
            double dummy;
            if(!ValueOP::is_number(arguments[0]) || modf(ValueOP::as_number(arguments[0]), &dummy) != 0.0) {
                response.hasErr = true;
                response.propertyMissing = false;
                
                response.errorMessage = "Swap index argument must be an integer";
                return response;
            }
            
            if(values.count <= ValueOP::as_number(arguments[0])) {
                response.hasErr = true;
                response.propertyMissing = false;
                
                response.errorMessage = "Out of bound access to collection object.";
                return response;
            }
            
            
            
            response.isVoid = true;
            swap(arguments[0], arguments[1]);
        }
    } else if (method->chars.compare("getSize") == 0) {
        if(arguments.size() != 0) {
            response.hasErr = true;
            response.propertyMissing = false;
            
            response.errorMessage = "getSize method expects 0 argument.";
            return response;
        } else {
            response.isVoid = false;
            response.returnVal = ValueOP::number_val(getSize());
        }
    } else {
        response.hasErr = true;
        response.propertyMissing = true;
        
        response.errorMessage = "Collection class does not contain property '";
        response.errorMessage + method->chars;
        response.errorMessage.pop_back();
        response.errorMessage += "'.";
    }
    
    return response;
}


void ObjCollection::addBack(Value value) {
    values.writeValueArray(value);
}

void ObjCollection::deleteBack() {
    values.count--;
}

int ObjCollection::getSize() {
    return (int)values.count;
}

void ObjCollection::swap(Value index, Value value) {
    values.values[(int)ValueOP::as_number(index)] = value;
}
