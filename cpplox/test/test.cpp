#include <gtest/gtest.h>
#include "../pch.pch"
#include "../scanner.hpp"
#include "../chunk.hpp"

class Scanner_Test : public testing::Test {
protected:
    Scanner scan;
};

TEST_F(Scanner_Test, constructor) {
    EXPECT_EQ(scan.start, 0);
    EXPECT_EQ(scan.current, 0);
    EXPECT_EQ(scan.line, 1);
}

TEST_F(Scanner_Test, left_paren) {
    scan.setSource("(");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_LEFT_PAREN);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, right_paren) {
    scan.setSource(")");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_RIGHT_PAREN);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, left_brace) {
    scan.setSource("{");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_LEFT_BRACE);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, right_brace) {
    scan.setSource("}");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_RIGHT_BRACE);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, left_brack) {
    scan.setSource("[");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_LEFT_BRACK);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, right_brack) {
    scan.setSource("]");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_RIGHT_BRACK);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, comma) {
    scan.setSource(",");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_COMMA);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, dot) {
    scan.setSource(".");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_DOT);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, minus) {
    scan.setSource("-");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_MINUS);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, plus) {
    scan.setSource("+");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_PLUS);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, semicolon) {
    scan.setSource(";");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_SEMICOLON);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, slash) {
    scan.setSource("/");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_SLASH);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, star) {
    scan.setSource("*");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_STAR);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, bang) {
    scan.setSource("!");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_BANG);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, bang_equal) {
    scan.setSource("!=");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_BANG_EQUAL);
    EXPECT_EQ(token.length, 2);
}

TEST_F(Scanner_Test, equal) {
    scan.setSource("=");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_EQUAL);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, equal_equal) {
    scan.setSource("==");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_EQUAL_EQUAL);
    EXPECT_EQ(token.length, 2);
}

TEST_F(Scanner_Test, greater) {
    scan.setSource(">");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_GREATER);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, greater_equal) {
    scan.setSource(">=");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_GREATER_EQUAL);
    EXPECT_EQ(token.length, 2);
}

TEST_F(Scanner_Test, lesser) {
    scan.setSource("<");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_LESS);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, lesser_equal) {
    scan.setSource("<=");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_LESS_EQUAL);
    EXPECT_EQ(token.length, 2);
}

TEST_F(Scanner_Test, question_mark) {
    scan.setSource("?");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_QUESTION_MARK);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, colon) {
    scan.setSource(":");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_COLON);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, identifier) {
    scan.setSource("test");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_IDENTIFIER);
    EXPECT_EQ(token.length, 4);
}

TEST_F(Scanner_Test, string) {
    scan.setSource("\"string\"");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_STRING);
    EXPECT_EQ(token.length, 8);
    EXPECT_EQ(token.source.substr(1, token.length - 2).compare("string"), 0);
}

TEST_F(Scanner_Test, number) {
    scan.setSource("123");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_NUMBER);
    EXPECT_EQ(token.length, 3);
    EXPECT_EQ(std::stoi(token.source), 123);
}

TEST_F(Scanner_Test, token_and) {
    scan.setSource("and");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_AND);
    EXPECT_EQ(token.length, 3);
}

TEST_F(Scanner_Test, token_class) {
    scan.setSource("class");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_CLASS);
    EXPECT_EQ(token.length, 5);
}

TEST_F(Scanner_Test, token_else) {
    scan.setSource("else");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_ELSE);
    EXPECT_EQ(token.length, 4);
}

TEST_F(Scanner_Test, token_false) {
    scan.setSource("false");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_FALSE);
    EXPECT_EQ(token.length, 5);
}

TEST_F(Scanner_Test, token_for) {
    scan.setSource("for");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_FOR);
    EXPECT_EQ(token.length, 3);
}

TEST_F(Scanner_Test, fun) {
    scan.setSource("fun");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_FUN);
    EXPECT_EQ(token.length, 3);
}

TEST_F(Scanner_Test, token_if) {
    scan.setSource("if");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_IF);
    EXPECT_EQ(token.length, 2);
}

TEST_F(Scanner_Test, nul) {
    scan.setSource("nul");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_NUL);
    EXPECT_EQ(token.length, 3);
}

TEST_F(Scanner_Test, token_or) {
    scan.setSource("or");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_OR);
    EXPECT_EQ(token.length, 2);
}

TEST_F(Scanner_Test, print) {
    scan.setSource("print");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_PRINT);
    EXPECT_EQ(token.length, 5);
}

TEST_F(Scanner_Test, token_return) {
    scan.setSource("return");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_RETURN);
    EXPECT_EQ(token.length, 6);
}

TEST_F(Scanner_Test, super) {
    scan.setSource("super");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_SUPER);
    EXPECT_EQ(token.length, 5);
}

TEST_F(Scanner_Test, token_this) {
    scan.setSource("this");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_THIS);
    EXPECT_EQ(token.length, 4);
}

TEST_F(Scanner_Test, token_true) {
    scan.setSource("true");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_TRUE);
    EXPECT_EQ(token.length, 4);
}

TEST_F(Scanner_Test, var) {
    scan.setSource("var");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_VAR);
    EXPECT_EQ(token.length, 3);
}

TEST_F(Scanner_Test, token_const) {
    scan.setSource("const");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_CONST);
    EXPECT_EQ(token.length, 5);
}

TEST_F(Scanner_Test, token_while) {
    scan.setSource("while");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_WHILE);
    EXPECT_EQ(token.length, 5);
}

TEST_F(Scanner_Test, token_continue) {
    scan.setSource("continue");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_CONTINUE);
    EXPECT_EQ(token.length, 8);
}

TEST_F(Scanner_Test, token_break) {
    scan.setSource("break");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_BREAK);
    EXPECT_EQ(token.length, 5);
}

TEST_F(Scanner_Test, token_switch) {
    scan.setSource("switch");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_SWITCH);
    EXPECT_EQ(token.length, 6);
}

TEST_F(Scanner_Test, token_case) {
    scan.setSource("case");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_CASE);
    EXPECT_EQ(token.length, 4);
}

TEST_F(Scanner_Test, token_default) {
    scan.setSource("default");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_DEFAULT);
    EXPECT_EQ(token.length, 7);
}

TEST_F(Scanner_Test, token_del) {
    scan.setSource("delete");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_DEL);
    EXPECT_EQ(token.length, 6);
}

TEST_F(Scanner_Test, import) {
    scan.setSource("import");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_IMPORT);
    EXPECT_EQ(token.length, 6);
}

TEST_F(Scanner_Test, error) {
    scan.setSource("@");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_ERROR);
    EXPECT_EQ(token.source.compare("Unexpected character."), 0);
}

TEST_F(Scanner_Test, eof) {
    scan.setSource("");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_EOF);
    EXPECT_EQ(token.length, 0);
}

TEST_F(Scanner_Test, line) {
    scan.setSource("line1\nline2\nline3");
    
    Token line1 = scan.scanToken(), line2 = scan.scanToken(), line3 = scan.scanToken();
    EXPECT_EQ(line1.type, TOKEN_IDENTIFIER);
    EXPECT_EQ(line2.type, TOKEN_IDENTIFIER);
    EXPECT_EQ(line3.type, TOKEN_IDENTIFIER);
    EXPECT_EQ(line1.line, 1);
    EXPECT_EQ(line2.line, 2);
    EXPECT_EQ(line3.line, 3);
}

TEST_F(Scanner_Test, spaces) {
    scan.setSource("first     second     third\n   fourth");
    
    Token one = scan.scanToken(), two = scan.scanToken(), three = scan.scanToken(), four = scan.scanToken();
    EXPECT_EQ(one.type, TOKEN_IDENTIFIER);
    EXPECT_EQ(two.type, TOKEN_IDENTIFIER);
    EXPECT_EQ(three.type, TOKEN_IDENTIFIER);
    EXPECT_EQ(four.type, TOKEN_IDENTIFIER);
    
    EXPECT_EQ(one.source.compare("first"), 0);
    EXPECT_EQ(two.source.compare("second"), 0);
    EXPECT_EQ(three.source.compare("third"), 0);
    EXPECT_EQ(four.source.compare("fourth"), 0);
}


class Chunk_test : public testing::Test {
protected:
    Chunk chunk;
};

TEST_F(Chunk_test, constructor) {
    EXPECT_EQ(chunk.count, 0);
    EXPECT_EQ(chunk.lineCount, 0);
}

TEST_F(Chunk_test, test_write_chunk) {
    chunk.writeChunk(OP_NUL, 1);
    chunk.writeChunk(OP_ADD, 2);
    
    EXPECT_EQ(chunk.code[0], OP_NUL);
    EXPECT_EQ(chunk.code[1], OP_ADD);
}

TEST_F(Chunk_test, test_lines) {
    chunk.writeChunk(OP_NUL, 1);
    chunk.writeChunk(OP_NUL, 1);
    chunk.writeChunk(OP_NUL, 2);
    chunk.writeChunk(OP_NUL, 2);
    chunk.writeChunk(OP_NUL, 3);
    chunk.writeChunk(OP_NUL, 4);
    
    EXPECT_EQ(chunk.lineCount, 4);
    EXPECT_EQ(chunk.getLine(0), 1);
    EXPECT_EQ(chunk.getLine(1), 1);
    EXPECT_EQ(chunk.getLine(2), 2);
    EXPECT_EQ(chunk.getLine(3), 2);
    EXPECT_EQ(chunk.getLine(4), 3);
    EXPECT_EQ(chunk.getLine(5), 4);
}

TEST_F(Chunk_test, test_add_constant) {
    
}

int main(int argc, char *argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
