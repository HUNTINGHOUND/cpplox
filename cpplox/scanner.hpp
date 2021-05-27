

//TODO: Finish Documentation

#ifndef scanner_hpp
#define scanner_hpp

#include <string>

enum TokenType {
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_PLUS,
    TOKEN_SEMICOLON, TOKEN_SLASH, TOKEN_STAR,
    
    TOKEN_BANG, TOKEN_BANG_EQUAL,
    TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER, TOKEN_GREATER_EQUAL,
    TOKEN_LESS, TOKEN_LESS_EQUAL, TOKEN_QUESTION_MARK,
    TOKEN_COLON,
    
    TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,
    
    TOKEN_AND, TOKEN_CLASS, TOKEN_ELSE, TOKEN_FALSE,
    TOKEN_FOR, TOKEN_FUN, TOKEN_IF, TOKEN_NUL, TOKEN_OR,
    TOKEN_PRINT, TOKEN_RETURN, TOKEN_SUPER, TOKEN_THIS,
    TOKEN_TRUE, TOKEN_VAR, TOKEN_CONST, TOKEN_WHILE,
    TOKEN_CONTINUE, TOKEN_BREAK, TOKEN_SWITCH, TOKEN_CASE,
    TOKEN_DEFAULT, TOKEN_DEL,
    
    TOKEN_ERROR,
    TOKEN_EOF
};

struct Token {
    TokenType type;
    std::string source;
    int length;
    int line;
    
    Token(TokenType type, const std::string& source, int start, int length, int line);
    static Token createToken(const std::string& text);
};

class Scanner {
    
    char advance();
    bool isAtEnd();
    Token makeToken(TokenType type);
    Token errorToken(const std::string& message);
    bool match(char expected);
    void skipWhitespace();
    char peek();
    char peekNext();
    Token string();
    Token number();
    Token identifier();
    TokenType identifierType();
    TokenType checkKeyword(int start, int length,
                           const std::string& rest, TokenType type);
    
    
public:
    std::string source;
    int start;
    int current;
    int line;
    
    Scanner();
    Token scanToken();
    void setSource(const std::string& src);
};

#endif /* scanner_hpp */
