#include "vm.hpp"
#include "debug.hpp"
#include "compiler.hpp"
#include "flags.hpp"
#include <iostream>
#include <cstdio>
#include <cstdarg>

uint8_t VM::read_byte() {
    return *ip++;
}

Value VM::read_constant() {
    return chunk->constants.values[read_byte()];
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

VM::VM() {
    std::deque<Value>().swap(stack);
}

void VM::freeVM() {
    
}

InterpretResult VM::interpret(const std::string& source) {
    Chunk chunk;
    Compiler compiler(source);
    
    if(!compiler.compile(&chunk)) {
        chunk.freeChunk();
        return INTERPRET_COMPILE_ERROR;
    }
    
    this->chunk = &chunk;
    this->ip = &this->chunk->code[0];
    
    InterpretResult result = run();
    
    chunk.freeChunk();
    return result;
}


InterpretResult VM::run() {
    
    for(;;) {
        
#ifdef DEBUG_TRACE_EXECUTION
        std::cout << "         ";
        for (auto it = stack.begin(); it != stack.end(); it++) {
            std::cout << "[";
            if(Value::is_nul(*it)) {
                std::cout << "nul]";
            } else if (Value::is_bool(*it)) {
                std::cout << (Value::as_bool(*it) ? "true" : "false") << "]";
            } else if (Value::is_number(*it)) {
                std::cout << Value::as_number(*it) << "]";
            } else {
                //should not happen
                std::cerr << "unrecognizable value in stack";
            }
        }
        std::cout << std::endl;
        
        uint8_t* start = &this->chunk->code[0];
        Disassembler::disassembleInstruction(this->chunk, (int)(this->ip - start));
        
#endif
        
        
        uint8_t instruction;
        switch(instruction = read_byte()) {
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
                binary_op<double,double>(Value::number_val, std::plus<double>());
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
                Value constant = read_constant();
                stack.push_back(constant);
                ValueArray::printValue(constant);
                std::cout << std::endl;
                break;
            }
            case OP_RETURN: {
                ValueArray::printValue(stack.back());
                stack.pop_back();
                std::cout << std::endl;
                return INTERPRET_OK;
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
        }
    }
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
    vfprintf(stderr, format.c_str(), args);
    va_end(args);
    std::cerr << std::endl;
    
    size_t instruction = ip - &chunk->code[0] - 1;
    int line = chunk->getLine(instruction);
    std::cerr << "[line " << line << "] in script" << std::endl;
    
    stack = std::deque<Value>();
}
