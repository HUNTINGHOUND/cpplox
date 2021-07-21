//
//  editorOP.cpp
//  loxtext
//
//  Created by Morgan Xu on 6/2/21.
//


#include "editorOP.hpp"
#include "output.hpp"

void EditorOP::editorInsertNewRow(int at, std::string chars) {
    if(at < 0 || at > E.numsrows) return;
    
    E.row.insert(E.row.begin() + at, Erow{{},chars, "", at, false});
    E.numsrows++;
    
    for(int i = at + 1; i < E.numsrows; i++) E.row[i].idx++;
    
    Output::editorUpdateRows(E.row[at]);
    
    E.dirty++;
}

void EditorOP::editorInsertChar(int c) {
    if(E.cy == E.numsrows) {
        editorInsertNewRow(E.numsrows, "");
    }
    
    Output::editorRowInsertChar(E.row[E.cy], E.cx, c);
    E.cx++;
}

void EditorOP::editorDelChar() {
    if(E.cy == E.numsrows) return;
    if(E.cx == 0 && E.cy == 0) return;
    
    Erow* row = &E.row[E.cy];
    if(E.cx > 0) {
        Output::editorRowDelChar(*row, E.cx - 1);
        E.cx--;
    } else {
        E.cx = E.row[E.cy - 1].chars.size();
        Output::editorRowAppendString(E.row[E.cy - 1], row->chars);
        E.row.erase(E.row.begin() + E.cy);
        E.numsrows--;
        for(int i = E.cy; i < E.numsrows; i++) E.row[i].idx--;
        E.cy--;
    }
}



void EditorOP::editorInsertNewLine() {
    if(E.cx == 0) {
        editorInsertNewRow(E.cy, "");
    } else {
        Erow* row = &E.row[E.cy];
        
        if(E.cx != row->chars.size()) {
            editorInsertNewRow(E.cy + 1, row->chars.substr(E.cx, row->chars.size() - E.cx));
            row = &E.row[E.cy];
            row->chars.erase(E.cx, row->chars.size() - E.cx);
        } else {
            editorInsertNewRow(E.cy + 1, "");
        }
        
        Output::editorUpdateRows(E.row[E.cy]);
        Output::editorUpdateRows(E.row[E.cy + 1]);
    }
    
    E.cy++;
    E.cx = 0;
    
}

