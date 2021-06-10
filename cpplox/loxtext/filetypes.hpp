#ifndef filetypes_h
#define filetypes_h

#include "includes.hpp"
#include "terminal.hpp"



const int hl_highlight_numbers = (1 << 0);
const int hl_highlight_strings = (1 << 1);

std::vector<std::string> C_HL_extensions{".c", ".h", ".cpp"};
std::vector<std::string> C_HL_keywords{
    "switch", "if", "while", "for", "break", "continue", "return", "else", "struct", "union", "typedef", "static", "enum",
    "class", "case",
    
    "int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|", "void|"
};

std::vector<EditorSyntax> HLDB {
    EditorSyntax{
        C_HL_extensions,
        C_HL_keywords,
        "c/c++",
        "//",
        hl_highlight_numbers | hl_highlight_strings
    }
};

#endif /* filetypes_h */
