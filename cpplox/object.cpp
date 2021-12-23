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

ObjFunction* ObjFunction::newFunction(VM* vm, FunctionType type) {
    ObjFunction* function = allocate_obj<ObjFunction>(OBJ_FUNCTION, vm);
    
    function->arity = 0;
    function->defaults = 0;
    function->upvalueCount = 0;
    function->funcType = type;
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

ObjNativeClassMethod* ObjNativeClassMethod::newNativeClassMethod(NativeClassMethod method, int arity, VM* vm) {
    ObjNativeClassMethod* newMethod = Obj::allocate_obj<ObjNativeClassMethod>(OBJ_NATIVE_CLASS_METHOD, vm);
    newMethod->method = method;
    newMethod->arity = arity;
    return newMethod;
};

ObjNativeClass* ObjNativeClass::newNativeClass(ObjString* name, VM* vm, ObjType subType) {
    ObjNativeClass* nativeClass = (ObjNativeClass *) ObjClass::newClass(name, vm);
    nativeClass->type = OBJ_NATIVE_CLASS;
    nativeClass->subType = subType;
    return nativeClass;
};

NativeClassRes NativeClassRes::genError(const std::string& errorMessage, bool propertyMissing) {
    NativeClassRes erro{true, propertyMissing, std::move(errorMessage)};
    return erro;
}

NativeClassRes NativeClassRes::genResponse(Value returnVal, bool isVoid) {
    NativeClassRes res{false, false, "", isVoid, std::move(returnVal)};
    return res;
}

void ObjNativeClass::addMethod(const std::string& name, NativeClassMethod method, int arity, VM* vm) {
    vm->push_stack(ValueOP::obj_val(ObjString::copyString(vm, name.c_str(), name.size())));
    vm->push_stack(ValueOP::obj_val(ObjNativeClassMethod::newNativeClassMethod(method, arity, vm)));
    methods.tableSet(vm->peek(1), vm->peek(0));
    vm->stack.pop_back();
    vm->stack.pop_back();
};

NativeClassRes ObjCollectionClass::invokeMethod(ObjString* name, int argCount, Value* args) {
    Value seek_method, name_val = ValueOP::obj_val(name);
    if(methods.tableGet(name_val, &seek_method)) {
        ObjNativeClassMethod* method_wrapper = static_cast<ObjNativeClassMethod*>(ValueOP::as_obj(seek_method));
        NativeClassMethod native_method = method_wrapper->method;
        CollectionClassMethod collection_method = static_cast<CollectionClassMethod>(native_method);
        NativeClassRes res = std::invoke(collection_method, *this, argCount, args);
        return res;
        
    } else {
        return NativeClassRes::genError("Property Not Found.", true);
    }
}

NativeClassRes ObjCollectionClass::indexAccess(int argCount, Value *args) {
    if(argCount != 1)
        return NativeClassRes::genError("Expected 1 argument, got " + std::to_string(argCount) + " instead.");
    if(!ValueOP::is_number(args[0]))
        return NativeClassRes::genError("Expected number as argument for collection random access.");
    
    long index = ValueOP::as_number(args[0]);
    if(index < 0) index = values.count - index;
    if(std::abs(index) == values.count)
        return NativeClassRes::genError("Out of range random accees");
    
    return NativeClassRes::genResponse(values.values[index], false);
}

NativeClassRes ObjCollectionClass::addValue(int argCount, Value* args) {
    if(argCount != 1)
        return NativeClassRes::genError("Expected 1 argument, got " + std::to_string(argCount) + " instead.");
    values.writeValueArray(args[0]);
    return NativeClassRes::genResponse(ValueOP::empty_val(), true);
}

NativeClassRes ObjCollectionClass::initializer(int argCount, Value* args) {
    
}

ObjCollectionClass* ObjCollectionClass::newCollectionClass(ObjString *name, VM *vm) {
    ObjCollectionClass* collection = static_cast<ObjCollectionClass*>(ObjNativeClass::newNativeClass(name, vm, OBJ_NATIVE_COLLECTION));
    collection->values = ValueArray(vm);
    
    collection->addMethod("addValue", static_cast<NativeClassMethod>(&ObjCollectionClass::addValue), 1, vm);
    collection->addMethod("indexAccess", static_cast<NativeClassMethod>(&ObjCollectionClass::indexAccess), 1, vm);
    
    
    return collection;
}

