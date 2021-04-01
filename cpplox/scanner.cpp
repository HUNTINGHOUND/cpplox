#include "scanner.hpp"


Token::Token(TokenType type, const std::string& source, int start, int length, int line) {
    this->type = type;
    this->source = source.substr(start, length);
    this->length = length;
    this->line = line;
}

bool Scanner::isAtEnd() {
    return this->current == this->source.length();
}

Token Scanner::makeToken(TokenType type) {
    return Token(type, source, start, current - start, line);
}

Token Scanner::errorToken(const std::string& message) {
    return Token(TOKEN_ERROR, message, 0, message.length(), this->line);
}

Scanner::Scanner(const std::string& source) {
    this->source = source;
    start = 0;
    current = 0;
    line = 1;
}

char Scanner::advance() {
    current++;
    return source[current - 1];
}

bool Scanner::match(char expected) {
    if (isAtEnd()) return false;
    if (source[current] != expected) return false;
    
    current++;
    return true;
}

char Scanner::peek() {
    return source[current];
}

char Scanner::peekNext() {
    if(isAtEnd()) return '\0';
    return source[current + 1];
}

void Scanner::skipWhitespace() {
    for(;;) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
                
            case '\n':
                line++;
                advance();
                break;
                
            case '/':
                if (peekNext() == '/') {
                    while(peek() != '\n' && !isAtEnd()) advance();
                } else
                    return;
                
                break;
            
            default:
                return;
        }
    }
}

Token Scanner::string() {
    while (peek() != '\"' && !isAtEnd()) {
        if(peek() == '\n') line++;
        advance();
    }
    
    if (isAtEnd()) {
        return errorToken("Unterminated string.");
    }
    
    advance();
    return makeToken(TOKEN_STRING);
}

Token Scanner::number() {
    while(isdigit(peek())) advance();
    
    if (peek() == '.' && isdigit(peekNext())) {
        advance();
        
        while (isdigit(peek())) advance();
    }
    
    return makeToken(TOKEN_NUMBER);
}

TokenType Scanner::checkKeyword(int start, int length,
                                const std::string& rest, TokenType type) {
    
    if (current - this->start == start + length && this->source.substr(start + this->start, length).compare(rest) == 0)
        return type;
    
    return TOKEN_IDENTIFIER;
}

TokenType Scanner::identifierType() {
    switch(source[start]) {
        case 'a': return checkKeyword(1, 2, "nd", TOKEN_AND);
        case 'b': return checkKeyword(1, 4, "reak", TOKEN_BREAK);
        case 'c':
            if (current - start > 1) {
                switch(source[start + 1]) {
                    case 'l': return checkKeyword(2, 3, "ass", TOKEN_CLASS);
                    case 'o':
                        if(current - start > 2) {
                            switch (source[start + 2]) {
                                case 'n':
                                    if(current - start > 3) {
                                        switch (source[start + 3]) {
                                            case 's':
                                                return checkKeyword(4, 1, "t", TOKEN_CONST);
                                            case 't':
                                                return checkKeyword(4, 4, "inue", TOKEN_CONTINUE);
                                        }
                                    }
                            }
                        }
                }
            }
        case 'e': return checkKeyword(1, 3, "lse", TOKEN_ELSE);
        case 'f':
            if (current - start > 1) {
                switch(source[start + 1]) {
                    case 'a': return checkKeyword(2, 3, "lse", TOKEN_FALSE);
                    case 'o': return checkKeyword(2, 1, "r", TOKEN_FOR);
                    case 'u': return checkKeyword(2, 1, "n", TOKEN_FUN);
                }
            }
        case 'i': return checkKeyword(1, 1, "f", TOKEN_IF);
        case 'n': return checkKeyword(1, 2, "il", TOKEN_NUL);
        case 'o': return checkKeyword(1, 1, "r", TOKEN_OR);
        case 'p': return checkKeyword(1, 4, "rint", TOKEN_PRINT);
        case 'r': return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
        case 's': return checkKeyword(1, 4, "uper", TOKEN_SUPER);
        case 't':
            if (current - start > 1) {
                switch(source[start + 1]) {
                    case 'h': return checkKeyword(2, 2, "is", TOKEN_THIS);
                    case 'r': return checkKeyword(2, 2, "ue", TOKEN_TRUE);
                }
            }
        case 'v': return checkKeyword(1, 2, "ar", TOKEN_VAR);
        case 'w': return checkKeyword(1, 4, "hile", TOKEN_WHILE);
    }
    
    return TOKEN_IDENTIFIER;
}

Token Scanner::identifier() {
    while (isalpha(peek()) || isdigit(peek())) advance();
    
    return makeToken(identifierType());
}

Token Scanner::scanToken() {
    
    skipWhitespace();
    
    this->start = this->current;
    
    if(isAtEnd()) return makeToken(TOKEN_EOF);
    
    
    
    char c = advance();
    
    if (isalpha(c)) return identifier();
    if (isdigit(c)) return number();
    
    switch(c) {
        case '(': return makeToken(TOKEN_LEFT_PAREN);
        case ')': return makeToken(TOKEN_RIGHT_PAREN);
        case '{': return makeToken(TOKEN_LEFT_BRACE);
        case '}': return makeToken(TOKEN_RIGHT_BRACE);
        case ';': return makeToken(TOKEN_SEMICOLON);
        case ',': return makeToken(TOKEN_COMMA);
        case '.': return makeToken(TOKEN_DOT);
        case '-': return makeToken(TOKEN_MINUS);
        case '+': return makeToken(TOKEN_PLUS);
        case '/': return makeToken(TOKEN_SLASH);
        case '*': return makeToken(TOKEN_STAR);
        case '?': return makeToken(TOKEN_QUESTION_MARK);
        case ':': return makeToken(TOKEN_COLON);
        case '!':
            return makeToken(
                        match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=':
              return makeToken(
                        match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<':
              return makeToken(
                        match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>':
              return makeToken(
                        match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
            
        case '"':
            return string();
    }
    
    return errorToken("Unexpected character.");
}
