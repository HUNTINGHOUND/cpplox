#include "vm.hpp"
#include "debug.hpp"
#include "compiler.hpp"
#include "flags.hpp"
#include "memory.hpp"
#include "object.hpp"
#include <iostream>
#include <cstdio>

//NATIVE FUNCTIONS====================================================>

bool VM::clockNative(int argCount, Value *args) {
    args[-1] = Value::number_val((double)clock() / CLOCKS_PER_SEC);
    return true;
}

bool VM::errNative(int argCount, Value* args) {
    args[-1] = Value::obj_val(ObjString::copyString(this, "Error.", 6));
    return false;
}

bool VM::runtimeErrNative(int argCount, Value *args) {
    ObjString* errMessage = Value::as_string(args[0]);
    args[-1] = Value::obj_val(ObjString::copyString(this, errMessage->chars, errMessage->length));
    return false;
}

bool VM::getLineNative(int argCount, Value *args) {
    try {
        std::string get;
        getline(std::cin, get);
        args[-1] = Value::obj_val(ObjString::copyString(this, get.c_str(), get.length()));
        return true;
    } catch(std::exception& e) {
        args[-1] = Value::obj_val(ObjString::copyString(this, e.what(), (int)strlen(e.what())));
        return false;
    }
}

//====================================================================>

VM::VM() : strings(), globalNames(), globalValues(){
    std::deque<Value>().swap(stack);
    objects = nullptr;
    
    defineNative("clock", &VM::clockNative, 0);
    defineNative("error", &VM::errNative, 0);
    defineNative("runtimeError", &VM::runtimeErrNative, 1);
    defineNative("getLine", &VM::getLineNative, 0);
}

uint8_t VM::read_byte(CallFrame* frame) {
    return *frame->ip++;
}

Value VM::read_constant(CallFrame* frame) {
    return frame->function->chunk.constants.values[read_byte(frame)];
}

CallFrame::CallFrame(ObjFunction* function, uint8_t* ip, size_t slots) {
    this->function = function;
    this->ip = ip;
    this->slots = slots;
}

template <typename T, typename U>
InterpretResult VM::binary_op(Value (*valuetype)(T),std::function<T (U, U)> func) {
    
    if(!Value::is_number(peek(0)) || !Value::is_number(peek(1))) {
        runtimeError("Operands must be numbers.");
        return INTERPRET_RUNTIME_ERROR;
    }
    double b = Value::as_number(stack.back());
    stack.pop_back();
    double a = Value::as_number(stack.back());
    stack.pop_back();
    Value v = std::invoke(*valuetype, func(a,b));
    stack.push_back(v);
    return INTERPRET_OK;
}

void VM::freeVM() {
    strings.freeTable();
    globalNames.freeTable();
    globalValues.freeValueArray();
    freeObjects(this);
}

InterpretResult VM::interpret(const std::string& source) {
    Scanner scanner;
    Parser parser(&scanner);
    
    Compiler compiler(this, TYPE_SCRIPT, nullptr, &scanner, &parser);
    ObjFunction* function = compiler.compile(source);
    
    if(function == nullptr) return INTERPRET_COMPILE_ERROR;
    
    stack.push_back(Value::obj_val(function));
    callValue(Value::obj_val(function), 0);
    
    return run();
}


InterpretResult VM::run() {
    
    CallFrame* frame = &frames.back();
    
    for(;;) {
        
#ifdef DEBUG_TRACE_EXECUTION
        std::cout << "         ";
        for (auto it = stack.begin(); it != stack.end(); it++) {
            std::cout << "[";
            Value::printValue(*it);
            std::cout << "]";
        }
        std::cout << std::endl;
        
        uint8_t* start = frame->function->chunk.code;
        Disassembler::disassembleInstruction(&frame->function->chunk, this, (int)(frame->ip - start));
        
#endif
        
        
        uint8_t instruction;
        switch(instruction = read_byte(frame)) {
            case OP_CONDITIONAL: {
                Value b = stack.back();
                stack.pop_back();
                Value a = stack.back();
                stack.pop_back();
                Value condition = stack.back();
                stack.pop_back();
                
                if(Value::as_bool(condition)) {
                    stack.push_back(a);
                } else {
                    stack.push_back(b);
                }
                
                break;
            }
            case OP_EQUAL: {
                Value b = stack.back();
                stack.pop_back();
                Value a = stack.back();
                stack.pop_back();
                stack.push_back(Value::bool_val(Value::valuesEqual(a,b)));
                break;
            }
            case OP_GREATER: {
                binary_op<bool, double>(Value::bool_val, std::greater<double>());
                break;
            }
            case OP_LESS: {
                binary_op<bool, double>(Value::bool_val, std::less<double>());
                break;
            }
            case OP_ADD: {
                if (Value::is_string(peek(0)) && Value::is_string(peek(1))) {
                    concatenate();
                } else if (Value::is_number(peek(0)) && Value::is_number(peek(1))) {
                    binary_op<double,double>(Value::number_val, std::plus<double>());
                } else {
                    runtimeError(
                                 "Operands must be two numbers or two strings.");
                    
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_DIVIDE: {
                binary_op<double,double>(Value::number_val, std::divides<double>());
                break;
            }
            case OP_MULTIPLY: {
                binary_op<double,double>(Value::number_val, std::multiplies<double>());
                break;
            }
            case OP_SUBTRACT: {
                binary_op<double,double>(Value::number_val, std::minus<double>());
                break;
            }
            case OP_NOT: {
                Value v = Value::bool_val(isFalsey(stack.back()));
                stack.pop_back();
                stack.push_back(v);
                break;
            }
            case OP_NEGATE: {
                if (!Value::is_number(peek(0))) {
                    runtimeError("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                
                stack.back() = Value::number_val(-Value::as_number(stack.back()));
                break;
            }
            case OP_CONSTANT: {
                Value constant = read_constant(frame);
                stack.push_back(constant);
                Value::printValue(constant);
                std::cout << std::endl;
                break;
            }
            case OP_RETURN: {
                Value result = stack.back();
                stack.pop_back();
                
                frames.pop_back();
                if(frames.size() == 0) {
                    stack.pop_back();
                    return INTERPRET_OK;
                }
                
                while(stack.size() != frame->slots) {
                    stack.pop_back();
                }
                stack.push_back(result);
                
                frame = &frames.back();
                break;
            }
            case OP_NUL:
                stack.push_back(Value::nul_val());
                break;
            case OP_TRUE:
                stack.push_back(Value::bool_val(true));
                break;
            case OP_FALSE:
                stack.push_back(Value::bool_val(false));
                break;
            case OP_PRINT: {
                Value::printValue(stack.back());
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
                if (Value::is_empty(value)) {
                    runtimeError("Undefined variable.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                stack.push_back(value);
                break;
            }
            case OP_SET_GLOBAL: {
                uint8_t index = read_byte(frame);
                if (Value::is_empty(globalValues.values[index])) {
                    runtimeError("Undefined variable.");;
                    return INTERPRET_RUNTIME_ERROR;
                }
                globalValues.values[index] = peek(0);
                break;
            }
            case OP_GET_LOCAL: {
                uint8_t slot = read_byte(frame);
                stack.push_back(stack[frame->slots + slot]);
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
                stack.push_back(peek(0));
                break;
            case OP_CALL: {
                int argCount = read_byte(frame);
                if(!callValue(peek(argCount), argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &frames.back();
                break;
            }
        }
    }
}


void VM::concatenate() {
    ObjString* b = Value::as_string(stack.back());
    stack.pop_back();
    
    ObjString* a = Value::as_string(stack.back());
    stack.pop_back();
    
    int length = a->length + b->length;
    char* newString = allocate<char>(length + 1);
    memcpy(newString, a->chars, a->length);
    memcpy(newString+ a->length, b->chars, b->length);
    newString[length] = '\0';
    
    ObjString* result = ObjString::copyString(this, newString, length);
    
    
    stack.push_back(Value::obj_val(result));
}

bool VM::isFalsey(Value value) {
    return Value::is_nul(value) || (Value::is_bool(value) && !Value::as_bool(value));
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
    
    for (int i = frames.size() - 1; i >= 0; i--) {
        CallFrame* frame = &frames[i];
        ObjFunction* function = frame->function;
        
        size_t instruction = frame->ip - function->chunk.code - 1;
        std::cerr << "[line " << function->chunk.getLine(instruction) << "] in ";
        
        if(function->name == nullptr) {
            std::cerr << "script" << std::endl;
        } else {
            std::cerr << function->name->chars << "()" << std::endl;
        }
    }
    
    stack = std::deque<Value>();
}

uint16_t VM::read_short(CallFrame* frame) {
    frame->ip += 2;
    return (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]);
}

bool VM::callValue(Value callee, int argCount) {
    if(Value::is_obj(callee)) {
        switch (Value::obj_type(callee)) {
            case OBJ_FUNCTION:
                return call(Value::as_function(callee), argCount);
            case OBJ_NATIVE: {
                ObjNative* native = Value::as_native(callee);
                
                if(argCount != native->arity) {
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
                    runtimeError(Value::as_string(stack[stack.size() - argCount - 1])->chars);
                    return false;
                }
            }
                
            default:
                break;
        }
    }
    
    runtimeError("Can only call functions and classes.");
    return false;
}

bool VM::call(ObjFunction *function, int argCount) {
    if(argCount != function->arity) {
        runtimeError("Expected %d arguments but got %d.", function->arity, argCount);
        return false;
    }
    
    if(frames.size() == frames.max_size()) {
        runtimeError("Stack overflow.");
        return false;
    }
    
    frames.push_back(CallFrame(function, function->chunk.code, stack.size() - argCount - 1));
    return true;
}

void VM::defineNative(const std::string& name, NativeFn function, int arity) {
    stack.push_back(Value::obj_val(ObjString::copyString(this, name.c_str(), (int)strlen(name.c_str()))));
    stack.push_back(Value::obj_val(ObjNative::newNative(function,arity)));
    globalValues.writeValueArray(stack[1]);
    globalNames.tableSet(stack[0], Value::number_val(globalValues.count - 1));
    stack.pop_back();
    stack.pop_back();
}

