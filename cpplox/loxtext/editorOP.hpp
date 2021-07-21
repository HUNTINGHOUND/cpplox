#ifndef editorOP_hpp
#define editorOP_hpp

#include "includes.hpp"

namespace EditorOP {
void editorInsertChar(int c);
void editorDelChar();
void editorInsertNewLine();
void editorInsertNewRow(int at, std::string chars);
}

#endif /* editorOP_hpp */
