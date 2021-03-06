#include "compiler.hpp"
#include <iostream>

#ifdef DEBUG_PRINT_CODE
#include "debug.hpp"
#endif


ParseRule rules[42] = {
  [TOKEN_LEFT_PAREN]    = {&Compiler::grouping, nullptr,   PREC_NONE},
  [TOKEN_RIGHT_PAREN]   = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_LEFT_BRACE]    = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_RIGHT_BRACE]   = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_COMMA]         = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_DOT]           = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_MINUS]         = {&Compiler::unary, &Compiler::binary, PREC_TERM},
  [TOKEN_PLUS]          = {nullptr,     &Compiler::binary, PREC_TERM},
  [TOKEN_SEMICOLON]     = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_SLASH]         = {nullptr,     &Compiler::binary, PREC_FACTOR},
  [TOKEN_STAR]          = {nullptr,     &Compiler::binary, PREC_FACTOR},
  [TOKEN_BANG]          = {&Compiler::unary,     nullptr,   PREC_NONE},
  [TOKEN_BANG_EQUAL]    = {nullptr,     &Compiler::binary,   PREC_EQUALITY},
  [TOKEN_EQUAL]         = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_EQUAL_EQUAL]   = {nullptr,     &Compiler::binary,   PREC_EQUALITY},
  [TOKEN_GREATER]       = {nullptr,     &Compiler::binary,   PREC_COMPARISON},
  [TOKEN_GREATER_EQUAL] = {nullptr,     &Compiler::binary,   PREC_COMPARISON},
  [TOKEN_LESS]          = {nullptr,     &Compiler::binary,   PREC_COMPARISON},
  [TOKEN_LESS_EQUAL]    = {nullptr,     &Compiler::binary,   PREC_COMPARISON},
  [TOKEN_QUESTION_MARK] = {nullptr,     &Compiler::condition, PREC_CONDITIONAL},
  [TOKEN_COLON]         = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_IDENTIFIER]    = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_STRING]        = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_NUMBER]        = {&Compiler::number, nullptr, PREC_NONE},
  [TOKEN_AND]           = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_CLASS]         = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_ELSE]          = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_FALSE]         = {&Compiler::literal,     nullptr,   PREC_NONE},
  [TOKEN_FOR]           = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_FUN]           = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_IF]            = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_NUL]           = {&Compiler::literal,     nullptr,   PREC_NONE},
  [TOKEN_OR]            = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_PRINT]         = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_RETURN]        = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_SUPER]         = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_THIS]          = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_TRUE]          = {&Compiler::literal,     nullptr,   PREC_NONE},
  [TOKEN_VAR]           = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_WHILE]         = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_ERROR]         = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_EOF]           = {nullptr,     nullptr,   PREC_NONE},
};

Compiler::Compiler(const std::string& source) : scanner(source), parser(&scanner){
}

Parser::Parser(Scanner* scanner) : current(TOKEN_NUL, "", 0, 0, 0), previous(TOKEN_NUL, "", 0, 0, 0){
    this->scanner = scanner;
}

void Parser::errorAt(Token* token, const std::string& message) {
    if(panicMode) return;
    panicMode = true;
    std::cerr << "[line " << token->line << "] Error";
    
    if (token->type == TOKEN_EOF) {
        std::cerr << " at end";
    } else if (token->type == TOKEN_ERROR) {
        //nothing
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->source.c_str());
    }
    
    std::cerr << ": " << message << std::endl;
    hadError = true;
}

void Parser::errorAtCurrent(const std::string& message) {
    errorAt(&this->current, message);
}

void Parser::error(const std::string& message) {
    errorAt(&this->previous, message);
}

void Parser::advance() {
    this->previous = this->current;
    
    for (;;) {
        this->current = scanner->scanToken();
        if (this->current.type != TOKEN_ERROR) break;
        
        errorAtCurrent(this->current.source);
    }
}

void Parser::consume(TokenType type, const std::string& message) {
    if (current.type == type) {
        advance();
        return;
    }
    
    errorAtCurrent(message);
}

Chunk* Compiler::currentChunk() {
    return compilingChunk;
}

void Compiler::emitByte(uint8_t byte) {
    currentChunk()->writeChunk(byte, parser.previous.line);
}

void Compiler::endCompiler() {
    emitReturn();
#ifdef DEBUG_PRINT_CODE
    if(!parser.hadError) {
        Disassembler::disassembleChunk(currentChunk(), "code");
    }
#endif
}

void Compiler::condition() {
    parsePrecedence(PREC_CONDITIONAL);
    
    parser.consume(TOKEN_COLON, "Expect ':' after conditional operator");
    
    parsePrecedence(PREC_ASSIGNMENT);
}

void Compiler::emitReturn() {
    emitByte(OP_RETURN);
}

void Compiler::emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

void Compiler::number() {
    double value = atof(parser.previous.source.c_str());
    emitConstant(Value::number_val(value));
}

void Compiler::emitConstant(Value value) {
    emitBytes(OP_CONSTANT, makeConstant(value));
}

uint8_t Compiler::makeConstant(Value value) {
    int constant = currentChunk()->addConstant(value);
    if (constant > UINT8_MAX) {
        parser.error("Too many constants in one chunk.");
        return 0;
    }
    
    return (uint8_t)constant;
}

void Compiler::grouping() {
    expression();
    parser.consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression");
}

void Compiler::unary() {
    TokenType operatorType = parser.previous.type;
    
    parsePrecedence(PREC_UNARY);
    
    switch (operatorType) {
        case TOKEN_BANG:
            emitByte(OP_NOT);
            break;
        case TOKEN_MINUS:
            emitByte(OP_NEGATE);
            break;
            
        default:
            return; //unreachable
    }
}

void Compiler::binary() {
    TokenType operatorType = parser.previous.type;
    
    ParseRule* rule = ParseRule::getRule(operatorType);
    parsePrecedence((Precedence)(rule->precedence + 1));
    
    switch(operatorType) {
        case TOKEN_BANG_EQUAL:
            emitBytes(OP_EQUAL, OP_NOT);
            break;
        case TOKEN_EQUAL_EQUAL:
            emitByte(OP_EQUAL);
            break;
        case TOKEN_GREATER:
            emitByte(OP_GREATER);
            break;
        case TOKEN_GREATER_EQUAL:
            emitBytes(OP_LESS, OP_NOT);
            break;
        case TOKEN_LESS:
            emitByte(OP_LESS);
            break;
        case TOKEN_LESS_EQUAL:
            emitBytes(OP_GREATER, OP_NOT);
            break;
        case TOKEN_PLUS:
            emitByte(OP_ADD);
            break;
        case TOKEN_MINUS:
            emitByte(OP_SUBTRACT);
            break;
        case TOKEN_STAR:
            emitByte(OP_MULTIPLY);
            break;
        case TOKEN_SLASH:
            emitByte(OP_DIVIDE);
            break;
            
        default:
            return; //unreachable
    }
}


void Compiler::parsePrecedence(Precedence precedence) {
    parser.advance();
    ParseFn prefixRule = ParseRule::getRule(parser.previous.type)->prefix;
    if (prefixRule == nullptr) {
        parser.error("Expect Expression.");
        return;
    }
    
    std::invoke(prefixRule, *this);
    
    while (precedence <= ParseRule::getRule(parser.current.type)->precedence) {
        parser.advance();
        ParseFn infixRule = ParseRule::getRule(parser.previous.type)->infix;
        std::invoke(infixRule, *this);
    }
}

void Compiler::literal() {
    switch(parser.previous.type) {
        case TOKEN_FALSE:
            emitByte(OP_FALSE);
            break;
        case TOKEN_NUL:
            emitByte(OP_NUL);
            break;
        case TOKEN_TRUE:
            emitByte(OP_TRUE);
            break;
            
        default:
            return; //unreachable
    }
}

void Compiler::expression() {
    parsePrecedence(PREC_ASSIGNMENT);
}

bool Compiler::compile(Chunk* chunk) {
    compilingChunk = chunk;
    
    parser.hadError = false;
    parser.panicMode = false;
    
    parser.advance();
    expression();
    parser.consume(TOKEN_EOF, "Expect end of expression.");
    endCompiler();
    return !parser.hadError;
}

ParseRule*  ParseRule::getRule(TokenType type) {
    return &rules[type];
}
