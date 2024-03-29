#include "vm.hpp"
#include "debug.hpp"
#include "compiler.hpp"
#include "memory.hpp"
#include "object.hpp"
#include "table.hpp"
#include "flags.hpp"

//NATIVE FUNCTIONS====================================================>

bool VM::toStringNative(int argCount, Value *args) {
    args[-1] = ValueOP::obj_val(ValueOP::to_string(args[0], this));
    return true;
}

bool VM::interpolateNative(int argCount, Value *args) {
    if(!ValueOP::is_string(args[0])) {
        args[-1] = ValueOP::obj_val(ObjString::copyString(this, "Expected first element to be a string."));
        return false;
    }
    if(!ValueOP::is_native_subinstance(args[1], NATIVE_COLLECTION_INSTANCE)) {
        args[-1] = ValueOP::obj_val(ObjString::copyString(this, "Expected the second argument to be collection."));
        return false;
    }
    
    ObjString* format = ValueOP::as_string(args[0]);
    size_t n = format->chars.length();
    
    ObjCollectionInstance* input = ValueOP::as_native_subinstance<ObjCollectionInstance>(args[1]);
    size_t m = input->values.count;
    
    std::string interloped = "";
    int j = 0;
    int i = 0;
    while(i < n) {
        if(format->chars[i] == '$' && i != n - 2 && format->chars[i + 1] == '{' && format->chars[i + 2] == '}') {
            if(j >= m) {
                args[-1] = ValueOP::obj_val(ObjString::copyString(this, "Expected more arguments for interpolation."));
                return false;
            }
            
            ObjString* arg = ValueOP::to_string(input->values.values[j++], this);
            push_stack(ValueOP::obj_val(arg));
            std::string inserted(arg->chars);
            interloped += inserted;
            stack.pop_back();
            
            i += 3;
        } else {
            interloped += format->chars[i];
            i++;
        }
    }
    
    args[-1] = ValueOP::obj_val(ObjString::copyString(this, interloped));
    return true;
}

bool VM::clockNative(int argCount, Value *args) {
    args[-1] = ValueOP::number_val((double)clock() / CLOCKS_PER_SEC);
    return true;
}

bool VM::errNative(int argCount, Value* args) {
    args[-1] = ValueOP::obj_val(ObjString::copyString(this, "Error."));
    return false;
}

bool VM::runtimeErrNative(int argCount, Value *args) {
    ObjString* errMessage = ValueOP::as_string(args[0]);
    args[-1] = ValueOP::obj_val(ObjString::copyString(this, errMessage->chars));
    return false;
}

bool VM::getLineNative(int argCount, Value *args) {
    try {
        std::string get;
        getline(std::cin, get);
        args[-1] = ValueOP::obj_val(ObjString::copyString(this, get));
        return true;
    } catch(std::exception& e) {
        args[-1] = ValueOP::obj_val(ObjString::copyString(this, e.what()));
        return false;
    }
}

bool VM::hasFieldNative(int argCount, Value* args) {
    if(!ValueOP::is_instance(args[0])) {
        args[-1] = ValueOP::obj_val(ObjString::copyString(this, "Expected first argument to be a class instance."));
        return false;
    }
    
    if(!ValueOP::is_string(args[1])) {
        args[-1] = ValueOP::obj_val(ObjString::copyString(this, "Expected second argument to be a string."));
        return false;
    }
    
    ObjInstance* instance = ValueOP::as_instance(args[0]);
    Value dummy;
    args[-1] = ValueOP::bool_val(instance->fields.tableGet(args[1], &dummy));
    return true;
}

bool VM::getFieldNative(int argCount, Value *args) {
    if(!ValueOP::is_instance(args[0])) {
        args[-1] = ValueOP::obj_val(ObjString::copyString(this, "Expected first argument to be a class instance."));
        return false;
    }
    
    if(!ValueOP::is_string(args[1])) {
        args[-1] = ValueOP::obj_val(ObjString::copyString(this, "Expected second argument to be a string."));
        return false;
    }
    
    
    ObjInstance* instance = ValueOP::as_instance(args[0]);
    Value field;
    if(instance->fields.tableGet(args[1], &field)) {
        args[-1] = field;
        return true;
    } else {
        args[-1] = ValueOP::obj_val(ObjString::copyString(this, "Instance does not contain given field."));
        return false;
    }
}

bool VM::setFieldNative(int argCount, Value *args) {
    ObjInstance* instance = ValueOP::as_instance(args[0]);
    instance->fields.tableSet(args[1], args[2]);
    args[-1] = args[2];
    return true;
}

//====================================================================>

VM::VM() : strings(this), globalNames(this), globalValues(this){
    current = nullptr;
    currentClass = nullptr;
    objects = nullptr;
    openUpvalues = nullptr;
    bytesAllocated = 0;
    nextGC = 1024 * 1024;
    marker = true;
    
    initString = nullptr;
    initString = ObjString::copyString(this, "init");
    
    defineNative("clock", &VM::clockNative, 0);
    defineNative("error", &VM::errNative, 0);
    defineNative("runtimeError", &VM::runtimeErrNative, 1);
    defineNative("getLine", &VM::getLineNative, 0);
    defineNative("hasField", &VM::hasFieldNative, 2);
    defineNative("getField", &VM::getFieldNative, 2);
    defineNative("setField", &VM::setFieldNative, 3);
    defineNative("interpolate", &VM::interpolateNative, 2);
    defineNative("toString", &VM::toStringNative, 1);
    defineNative("isFloat", &VM::isFloatNative, 1);
    defineNative("isWhole", &VM::isWholeNative, 1);
    
    defineNativeClass("Collection", NATIVE_COLLECTION);
}

void VM::resetStacks() {
    std::deque<Value>().swap(stack);
    std::deque<CallFrame>().swap(frames);
    openUpvalues = nullptr;
}

uint8_t VM::read_byte(CallFrame* frame) {
    return *frame->ip++;
}

Value VM::read_constant(CallFrame* frame) {
    return getFrameFunction(frame)->chunk.constants.values[read_byte(frame)];
}

ObjString* VM::read_string(CallFrame *frame) {
    return ValueOP::as_string(read_constant(frame));
}


CallFrame::CallFrame(Obj* function, uint8_t* ip, size_t slots) {
    this->function = function;
    this->ip = ip;
    this->slots = slots;
}

template <typename T, typename U>
InterpretResult VM::binary_op(Value (*valuetype)(T),std::function<T (U, U)> func) {
    
    if(!ValueOP::is_number(peek(0)) || !ValueOP::is_number(peek(1))) {
        runtimeError("Operands must be numbers.");
        return INTERPRET_RUNTIME_ERROR;
    }
    Number b = ValueOP::as_number(stack.back());
    stack.pop_back();
    Number a = ValueOP::as_number(stack.back());
    stack.pop_back();
    Value v = std::invoke(*valuetype, func(a,b));
    push_stack(v);
    return INTERPRET_OK;
}

void VM::freeVM() {
    initString = nullptr;
    freeObjects(this);
}

InterpretResult VM::interpret(const std::string& source) {
    Scanner scanner;
    Parser parser(&scanner);
    
    Compiler compiler(this, TYPE_SCRIPT, nullptr, &scanner, &parser, EXECUTION_PATH);
    ObjFunction* function = compiler.compile(source);
    
    if(function == nullptr) return INTERPRET_COMPILE_ERROR;
    
    push_stack(ValueOP::obj_val(function));
    callValue(ValueOP::obj_val(function), 0);
    
    return run();
}


InterpretResult VM::run() {
    
    CallFrame* frame = &frames.back();
    
    for(;;) {
        
        if(DEBUG_TRACE_EXECUTION) {
            std::cout << "         ";
            for (auto it = stack.begin(); it != stack.end(); it++) {
                std::cout << "[";
                ValueOP::printValue(*it);
                std::cout << "]";
            }
            std::cout << std::endl;
            
            uint8_t* start = &getFrameFunction(frame)->chunk.code[0];
            Disassembler::disassembleInstruction(&getFrameFunction(frame)->chunk, this, (int)(frame->ip - start));
        }
        
        
        
        uint8_t instruction;
        switch(instruction = read_byte(frame)) {
            case OP_CONDITIONAL: {
                Value b = stack.back();
                stack.pop_back();
                Value a = stack.back();
                stack.pop_back();
                Value condition = stack.back();
                stack.pop_back();
                
                if(ValueOP::as_bool(condition)) {
                    push_stack(a);
                } else {
                    push_stack(b);
                }
                
                break;
            }
            case OP_EQUAL: {
                Value b = stack.back();
                stack.pop_back();
                Value a = stack.back();
                stack.pop_back();
                push_stack(ValueOP::bool_val(ValueOP::valuesEqual(a,b)));
                break;
            }
            case OP_GREATER: {
                binary_op<bool, Number>(ValueOP::bool_val, std::greater<Number>());
                break;
            }
            case OP_LESS: {
                binary_op<bool, Number>(ValueOP::bool_val, std::less<Number>());
                break;
            }
            case OP_ADD: {
                if (ValueOP::is_string(peek(0)) && ValueOP::is_string(peek(1))) {
                    concatenate();
                } else if (ValueOP::is_number(peek(0)) && ValueOP::is_number(peek(1))) {
                    binary_op<Number,Number>(ValueOP::number_val, std::plus<Number>());
                } else if (ValueOP::is_native_subinstance(peek(0), NATIVE_COLLECTION_INSTANCE) && ValueOP::is_native_subinstance(peek(1), NATIVE_COLLECTION_INSTANCE)) {
                    appendCollection();
                } else {
                    runtimeError("Operands must be two numbers, two strings, or two collections.");
                    
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_DIVIDE: {
                binary_op<Number,Number>(ValueOP::number_val, std::divides<Number>());
                break;
            }
            case OP_MULTIPLY: {
                binary_op<Number,Number>(ValueOP::number_val, std::multiplies<Number>());
                break;
            }
            case OP_SUBTRACT: {
                binary_op<Number,Number>(ValueOP::number_val, std::minus<Number>());
                break;
            }
            case OP_NOT: {
                Value v = ValueOP::bool_val(isFalsey(stack.back()));
                stack.pop_back();
                push_stack(v);
                break;
            }
            case OP_NEGATE: {
                if (!ValueOP::is_number(peek(0))) {
                    runtimeError("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                
                stack.back() = ValueOP::number_val(-ValueOP::as_number(stack.back()));
                break;
            }
            case OP_CONSTANT: {
                Value constant = read_constant(frame);
                push_stack(constant);
                break;
            }
            case OP_RETURN: {
                Value result = stack.back();
                stack.pop_back();
                
                closeUpvalues(&stack[frame->slots]);
                
                frames.pop_back();
                if(frames.size() == 0) {
                    stack.pop_back();
                    return INTERPRET_OK;
                }
                
                while(stack.size() != frame->slots) {
                    stack.pop_back();
                }
                push_stack(result);
                
                frame = &frames.back();
                break;
            }
            case OP_NUL:
                push_stack(ValueOP::nul_val());
                break;
            case OP_TRUE:
                push_stack(ValueOP::bool_val(true));
                break;
            case OP_FALSE:
                push_stack(ValueOP::bool_val(false));
                break;
            case OP_PRINT: {
                if(ValueOP::is_instance(stack.back())) {
                    ObjString* name = ObjString::copyString(this, "toString");
                    if(invoke(name, 0, false)) {
                        frame->ip--;
                        frame = &frames.back();
                        break;
                    }
                }
                
                ValueOP::printValue(stack.back());
                stack.pop_back();
                std::cout << std::endl;
                break;
            }
            case OP_POP:
                stack.pop_back();
                break;
            case OP_DEFINE_GLOBAL: {
                globalValues.values[read_byte(frame)] = stack.back();
                stack.pop_back();
                break;
            }
            case OP_GET_GLOBAL: {
                Value value = globalValues.values[read_byte(frame)];
                if (ValueOP::is_empty(value)) {
                    runtimeError("Undefined variable.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                push_stack(value);
                break;
            }
            case OP_SET_GLOBAL: {
                uint8_t index = read_byte(frame);
                if (ValueOP::is_empty(globalValues.values[index])) {
                    runtimeError("Undefined variable.");;
                    return INTERPRET_RUNTIME_ERROR;
                }
                globalValues.values[index] = peek(0);
                break;
            }
            case OP_GET_LOCAL: {
                uint8_t slot = read_byte(frame);
                push_stack(stack[frame->slots + slot]);
                break;
            }
            case OP_SET_LOCAL: {
                uint8_t slot = read_byte(frame);
                stack[frame->slots + slot] = peek(0);
                break;
            }
            case OP_JUMP_IF_FALSE: {
                uint16_t offset = read_short(frame);
                if (isFalsey(peek(0))) frame->ip += offset;
                break;
            }
            case OP_JUMP_IF_EMPTY: {
                uint16_t offset = read_short(frame);
                if (ValueOP::is_empty(peek(0))) frame->ip += offset;
                break;
            }
            case OP_JUMP: {
                uint16_t offset = read_short(frame);
                frame->ip += offset;
                break;
            }
            case OP_LOOP: {
                uint16_t offset = read_short(frame);
                frame->ip -= offset;
                break;
            }
            case OP_DUP:
                push_stack(peek(0));
                break;
            case OP_CALL: {
                int argCount = read_byte(frame);
                if(!callValue(peek(argCount), argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &frames.back();
                break;
            }
            case OP_CLOSURE: {
                ObjFunction* function = ValueOP::as_function(read_constant(frame));
                ObjClosure* closure = ObjClosure::newClosure(function, this);
                push_stack(ValueOP::obj_val(closure));
                for(int i = 0; i < closure->upvalueCount; i++) {
                    uint8_t isLocal = read_byte(frame);
                    uint8_t index = read_byte(frame);
                    if (isLocal) {
                        closure->upvalues[i] = captureUpvalue(frame->slots + index);
                    } else {
                        closure->upvalues[i] = ((ObjClosure*)frame->function)->upvalues[index];
                    }
                }
                break;
            }
            case OP_GET_UPVALUE: {
                uint8_t slot = read_byte(frame);
                push_stack(*((ObjClosure*)frame->function)->upvalues[slot]->location);
                break;
            }
            case OP_SET_UPVALUE: {
                uint8_t slot = read_byte(frame);
                *((ObjClosure*)frame->function)->upvalues[slot]->location = peek(0);
                break;
            }
            case OP_CLOSE_UPVALUE: {
                closeUpvalues(&stack.back());
                stack.pop_back();
                break;
            }
            case OP_CLASS: {
                push_stack(ValueOP::obj_val(ObjClass::newClass(read_string(frame), this)));
                break;
            }
            case OP_GET_PROPERTY: {
                if(!ValueOP::is_instance(peek(0))) {                    
                    runtimeError("Cannot reference property of non-instances.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                
                ObjInstance* instance = ValueOP::as_instance(peek(0));
                ObjString* name = read_string(frame);
                
                Value value;
                if (instance->fields.tableGet(ValueOP::obj_val(name), &value)) {
                    stack.pop_back();
                    push_stack(value);
                    break;
                }
                
                if(!bindMethod(instance->_class, name)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                
                break;
            }
            case OP_SET_PROPERTY: {
                if(!ValueOP::is_instance(peek(1))) {
                    runtimeError("Cannot reference property of non-instances.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                
                ObjInstance* instance = ValueOP::as_instance(peek(1));
                instance->fields.tableSet(ValueOP::obj_val(read_string(frame)), peek(0));
                
                Value value = stack.back();
                stack.pop_back();
                stack.pop_back();
                push_stack(value);
                break;
            }
            case OP_DEL: {
                if(!ValueOP::is_instance(peek(0))) {
                    runtimeError("Cannot reference property of non-instances.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                
                ObjInstance* instance = ValueOP::as_instance(peek(0));
                ObjString* name = read_string(frame);
                if(instance->fields.tableDelete(ValueOP::obj_val(name))) {
                    stack.pop_back();
                    break;
                }
                
                runtimeError("Undefined Property '%s'.", name->chars.c_str());
                return INTERPRET_RUNTIME_ERROR;
            }
            case OP_METHOD: {
                defineMethod(read_string(frame));
                break;
            }
            case OP_INVOKE: {
                ObjString* method = read_string(frame);
                int argCount = read_byte(frame);
                if(!invoke(method, argCount, true)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                
                frame = &frames.back();
                break;
            }
            case OP_INHERIT: {
                ObjClass* superclass = ValueOP::as_class(peek(1));
                if(!superclass) {
                    runtimeError("Superclass must be a class.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                
                ObjClass* subclass = ValueOP::as_class(peek(0));
                
                subclass->methods.tableAddAll(&superclass->methods);
                stack.pop_back();
                break;
            }
            case OP_GET_SUPER: {
                ObjString* name = read_string(frame);
                ObjClass* superclass = ValueOP::as_class(stack.back());
                stack.pop_back();
                
                if (!bindMethod(superclass, name)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                
                break;
            }
            case OP_SUPER_INVOKE: {
                ObjString* method = read_string(frame);
                int argCount = read_byte(frame);
                ObjClass* superclass = ValueOP::as_class(stack.back());
                stack.pop_back();
                
                if(!invokeFromClass(superclass, method, argCount, true)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                
                frame = &frames.back();
                break;
            }
            case OP_RANGE: {
                if(!ValueOP::is_number(peek(0)) || !ValueOP::is_number(peek(1)) || !ValueOP::is_number(peek(2))) {
                    runtimeError("Range start, stop, step must be numbers.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                
                double step = ValueOP::as_number(stack.back()).is_float ?
                    ValueOP::as_number(stack.back()).number.decimal : ValueOP::as_number(stack.back()).number.whole;
                stack.pop_back();
                
                double end = ValueOP::as_number(stack.back()).is_float ?
                    ValueOP::as_number(stack.back()).number.decimal : ValueOP::as_number(stack.back()).number.whole;
                stack.pop_back();
                
                double start = ValueOP::as_number(stack.back()).is_float ?
                    ValueOP::as_number(stack.back()).number.decimal : ValueOP::as_number(stack.back()).number.whole;
                stack.pop_back();
                
                Value collection_idx;
                globalNames.tableGet(ValueOP::obj_val(ObjString::copyString(this, "Collection")), &collection_idx);
                
                ObjCollectionClass* collection_class = ValueOP::as_native_subclass<ObjCollectionClass>(globalValues.values[(long)(ValueOP::as_number(collection_idx).is_float ? ValueOP::as_number(collection_idx).number.decimal : ValueOP::as_number(collection_idx).number.whole)]);
                ObjCollectionInstance* collection = ObjCollectionInstance::newCollectionInstance(collection_class, this);
                push_stack(ValueOP::obj_val(collection));
                for(double i = start; (end > start) ? i < end : i > end; i += step) {
                    Value tobeinserted = ValueOP::number_val(i);
                    collection_class->addValue(collection, 1, &tobeinserted);
                }
                
                break;
            }
            default:
                runtimeError("Invalid bytecode instruction.");
                return INTERPRET_RUNTIME_ERROR;
        }
    }
}

bool VM::invoke(ObjString *name, int argCount, bool interrupt) {
    Value receiver = peek(argCount);
    
    if(!ValueOP::is_instance(receiver) && !ValueOP::is_native_instance(receiver)) {
        runtimeError("Only instances have methods.");
        return false;
    }
    
    ObjInstance* instance = ValueOP::as_instance(receiver);
    
    Value value;
    if (instance->fields.tableGet(ValueOP::obj_val(name), &value)) {
        stack.end()[-argCount - 1] = value;
        return callValue(value, argCount);
    }
    
    return invokeFromClass(instance->_class, name, argCount, interrupt);
}

bool VM::invokeFromClass(ObjClass *_class, ObjString *name, int argCount, bool interrupt) {
    Value method;
    if (!_class->methods.tableGet(ValueOP::obj_val(name), &method)) {
        if(interrupt) runtimeError("Undefined property '%s'.", name->chars.c_str());
        return false;
    }
    
    if(ValueOP::is_native_method(method)) {
        ObjNativeClass* native_class = static_cast<ObjNativeClass*>(_class);
        NativeClassRes res = native_class->invokeMethod(name, ValueOP::as_native_instance(peek(argCount)), argCount, &stack[stack.size() - 1] - argCount + 1);
        if(res.hasErr) {
            if(interrupt) runtimeError(res.propertyMissing ? "Undefined property." : res.errorMessage);
            return false;
        }
        
        stack[stack.size() - 1 - argCount] = res.isVoid ? ValueOP::nul_val() : res.returnVal;
        for(int i = 0; i < argCount; i++) {
            stack.pop_back();
        }
        return true;
    }
    
    return call(ValueOP::as_obj(method), ValueOP::get_value_function(method), argCount);
}


void VM::concatenate() {
    ObjString* b = ValueOP::as_string(peek(0));
    ObjString* a = ValueOP::as_string(peek(1));
    
    std::string newString = a->chars + b->chars;
    
    ObjString* result = ObjString::copyString(this, newString.c_str());
    
    stack.pop_back();
    stack.pop_back();
    
    
    push_stack(ValueOP::obj_val(result));
}


bool VM::isFalsey(Value value) {
    return ValueOP::is_nul(value) || (ValueOP::is_bool(value) && !ValueOP::as_bool(value));
}

Value VM::peek(int distance) {
    return *(stack.end() - distance - 1);
}

void VM::runtimeError(const std::string& format, ... ) {
    va_list args;
    va_start(args, format);
    std::cerr << "Runtime Error: ";
    vfprintf(stderr, format.c_str(), args);
    va_end(args);
    std::cerr << std::endl;
    
    for (int i = (int)frames.size() - 1; i >= 0; i--) {
        CallFrame* frame = &frames[i];
        ObjFunction* function = getFrameFunction(frame);
        
        size_t instruction = frame->ip - (&function->chunk.code[0]) - 1;
        std::cerr << "[line " << function->chunk.getLine(instruction) << "] in ";
        
        if(function->name == nullptr) {
            std::cerr << "script" << std::endl;
        } else {
            std::cerr << function->name->chars << "()" << std::endl;
        }
    }
    
    resetStacks();
}

uint16_t VM::read_short(CallFrame* frame) {
    frame->ip += 2;
    return (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]);
}

bool VM::callValue(Value callee, int argCount) {
    if(ValueOP::is_obj(callee)) {
        switch (ValueOP::obj_type(callee)) {
            case OBJ_BOUND_METHOD: {
                ObjBoundMethod* bound = ValueOP::as_bound_method(callee);
                
                auto it = stack.end();
                it[-argCount - 1] = bound->receiver;
                
                return call((Obj*)bound->method, ValueOP::get_value_function(callee), argCount);
            }
            case OBJ_NATIVE: {
                ObjNative* native = ValueOP::as_native(callee);
                
                if(native->arity != -1 && argCount != native->arity) {
                    runtimeError("Expected %d arguments but got %d.", native->arity, argCount);
                    return false;
                }
                
                bool res = std::invoke(native->function, *this, argCount, &stack[stack.size() - 1] - argCount + 1);
                if(res) {
                    for(int i = 0; i < argCount; i++) {
                        stack.pop_back();
                    }
                    return true;
                } else {
                    runtimeError(ValueOP::as_string(stack[stack.size() - argCount - 1])->chars);
                    return false;
                }
            }
            case OBJ_CLOSURE:
                return callClosure(ValueOP::as_closure(callee), argCount);
            case OBJ_FUNCTION:
                return callFunction(ValueOP::as_function(callee), argCount);
            case OBJ_CLASS: {
                ObjClass* _class = ValueOP::as_class(callee);
                Value* back = &stack.back();
                back[-argCount] = ValueOP::obj_val(ObjInstance::newInstance(_class, this));
                
                if (_class->initializer) {
                    return call(_class->initializer, ValueOP::get_obj_function(_class->initializer), argCount);
                } else if(argCount != 0) {
                    runtimeError("Expected 0 argument but got %d.", argCount);
                    return false;
                }
                
                return true;
            }
            case OBJ_NATIVE_CLASS: {
                ObjNativeClass* _class = ValueOP::as_native_class(callee);
                Value* back = &stack.back();
                switch(_class->subType) {
                    case NATIVE_COLLECTION:
                        back[-argCount] = ValueOP::obj_val(ObjCollectionInstance::newCollectionInstance(static_cast<ObjCollectionClass*>(_class), this));
                        break;
                    default:
                        // should never be reached;
                        runtimeError("Invalid native class");
                        return false;
                }
                
                if(_class->hasInitializer) {
                    _class->invokeMethod(ObjString::copyString(this, "init"), ValueOP::as_native_instance(back[-argCount]), argCount, &stack[stack.size() - 1] - argCount + 1);
                    for(int i = 0; i < argCount; i++) stack.pop_back();
                } else if(argCount != 0) {
                    runtimeError("Expected 0 argument, but got %d.", argCount);
                    return false;
                }
                
                return true;
            }
            default:
                break;
        }
    }
    
    runtimeError("Can only call functions and classes.");
    return false;
}

bool VM::callClosure(ObjClosure *closure, int argCount) {
    return call((Obj*)closure, closure->function, argCount);
}

bool VM::callFunction(ObjFunction *function, int argCount) {
    return call((Obj*)function, function, argCount);
}

bool VM::call(Obj* callee, ObjFunction* function, int argCount) {
    
    if(argCount < function->arity - function->defaults) {
        runtimeError("Expected at least %d arguments but got %d.", function->arity - function->defaults, argCount);
        return false;
    }
    if(argCount > function->arity) {
        runtimeError("Expected at most %d arguments but got %d.", function->arity, argCount);
        return false;
    }
    if(frames.size() == frames.max_size()) {
        runtimeError("Stack overflow.");
        return false;
    }
    
    if(argCount > function->arity - function->defaults) {
        for(int i = 0; i < argCount - (function->arity - function->defaults); i++) {
            push_stack(ValueOP::empty_val());
        }
    }
    

    frames.push_back(CallFrame(callee, &function->chunk.code[0],
                               stack.size() - argCount - (argCount - (function->arity - function->defaults)) - 1));
    return true;
}

void VM::defineNative(std::string&& name, NativeFn function, int arity) {
    push_stack(ValueOP::obj_val(ObjString::copyString(this, std::move(name))));
    push_stack(ValueOP::obj_val(ObjNative::newNative(function,arity, this)));
    globalValues.writeValueArray(stack[1]);
    globalNames.tableSet(stack[0], ValueOP::number_val(globalValues.count - 1));
    stack.pop_back();
    stack.pop_back();
}

void VM::defineNativeClass(std::string&& name, NativeClassType type) {
    ObjString* obj_name = ObjString::copyString(this, std::move(name));
    push_stack(ValueOP::obj_val(obj_name));
    switch (type) {
        case NATIVE_COLLECTION:
            push_stack(ValueOP::obj_val(ObjCollectionClass::newCollectionClass(obj_name, this)));
            break;
            
        default:
            // should never reach
            break;
    }
    
    globalValues.writeValueArray(stack[1]);
    globalNames.tableSet(stack[0], ValueOP::number_val(globalValues.count - 1));
    stack.pop_back();
    stack.pop_back();
}

ObjUpvalue* VM::captureUpvalue(size_t localIndex) {
    ObjUpvalue* prevUpvalue = nullptr;
    ObjUpvalue* upvalue = openUpvalues;
    
    while (upvalue != nullptr && upvalue->location > &stack[localIndex]) {
        prevUpvalue = upvalue;
        upvalue = upvalue->nextUp;
    }
    
    if (upvalue != nullptr && upvalue->location == &stack[localIndex])
        return upvalue;
    
    ObjUpvalue* createdUpvalue = ObjUpvalue::newUpvalue(&stack[localIndex], this);
    createdUpvalue->next = upvalue;
    if(prevUpvalue == nullptr)
        openUpvalues = createdUpvalue;
    else
        prevUpvalue->next = createdUpvalue;
    
    return createdUpvalue;
}

void VM::closeUpvalues(Value *last) {
    while (openUpvalues != nullptr && openUpvalues->location >= last) {
        ObjUpvalue* upvalue = openUpvalues;
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        openUpvalues = upvalue->nextUp;
    }
}

ObjFunction* VM::getFrameFunction(CallFrame *frame) {
    if(frame->function->type == OBJ_FUNCTION) {
        return (ObjFunction*)frame->function;
    } else {
        return ((ObjClosure*)frame->function)->function;
    }
}

void VM::defineMethod(ObjString *name) {
    Value method = peek(0);
    ObjClass* _class = ValueOP::as_class(peek(1));
    _class->methods.tableSet(ValueOP::obj_val(name), method);
    if(name == initString) _class->initializer = ValueOP::as_obj(method);
    stack.pop_back();
    
}

bool VM::bindMethod(ObjClass *_class, ObjString *name) {
    Value method;
    if(!_class->methods.tableGet(ValueOP::obj_val(name), &method)) {
        runtimeError("Undefined property '%s'.", name->chars.c_str());
        return false;
    }
    
    ObjBoundMethod* bound = ObjBoundMethod::newBoundMethod(peek(0), ValueOP::as_obj(method), this);
    
    stack.pop_back();
    push_stack(ValueOP::obj_val(bound));
    return true;
}

void VM::push_stack(Value value) {
    stack.push_back(value);
}

void VM::appendCollection() {
    ObjCollectionInstance* collection1 = ValueOP::as_native_subinstance<ObjCollectionInstance>(peek(1));
    ObjCollectionInstance* collection2 = ValueOP::as_native_subinstance<ObjCollectionInstance>(peek(0));
    
    ObjCollectionInstance* newcollection = ObjCollectionInstance::newCollectionInstance(static_cast<ObjCollectionClass*>(collection1->_class), this);
    push_stack(ValueOP::obj_val(newcollection));
    for(int i = 0; i < collection1->values.count; i++) newcollection->values.writeValueArray(collection1->values.values[i]);
    for(int i = 0; i < collection2->values.count; i++) newcollection->values.writeValueArray(collection2->values.values[i]);
}

bool VM::isFloatNative(int argCount, Value *args) {
    if (!ValueOP::is_number(args[0])) {
        args[-1] = ValueOP::obj_val(ObjString::copyString(this, "isFloat only accept Number"));
        return false;
    }
    
    Number a = ValueOP::as_number(args[0]);
    args[-1] = ValueOP::bool_val(a.is_float);
    return true;
}

bool VM::isWholeNative(int argCount, Value *args) {
    if (!ValueOP::is_number(args[0])) {
        args[-1] = ValueOP::obj_val(ObjString::copyString(this, "isWhole only accept Number"));
        return false;
    }
    
    Number a = ValueOP::as_number(args[0]);
    args[-1] = ValueOP::bool_val(!a.is_float);
    return true;
}
