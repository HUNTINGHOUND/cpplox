#include "find.hpp"
#include "output.hpp"
#include "input.hpp"

void Find::editorFind() {
    int og_cx = E.cx;
    int og_cy = E.cy;
    int og_coloff = E.coloff;
    int og_rowoff = E.rowoff;
    
    std::function<void(std::string&, int)> callback = editorFindCallback;
    std::string query = Input::editorPrompt("Search: % (Use ESC/Arrows/Enter)", &callback);
    if(!query.empty()) return;
    
    E.cx = og_cx;
    E.cy = og_cy;
    E.coloff = og_coloff;
    E.rowoff = og_rowoff;
}

void Find::editorFindCallback(std::string& query, int key) {
    static int last_matchy = -1;
    static int last_matchx = -1;
    static int direction = 1;
    
    static int saved_hl_line;
    static std::vector<uint8_t> saved_hl;
    
    if(!saved_hl.empty()) {
        E.row[saved_hl_line].hl = saved_hl;
        saved_hl.clear();
    }
    
    
    if(key == '\r' || key == '\x1b') {
        last_matchy = -1;
        last_matchx = -1;
        direction = 1;
        return;
    } else if(key == ARROW_RIGHT || key == ARROW_DOWN) {
        direction = 1;
    } else if(key == ARROW_LEFT || key == ARROW_UP) {
        direction = -1;
    } else {
        last_matchy = -1;
        last_matchx = -1;
        direction = 1;
    }
    
    if(last_matchy == -1) direction = 1;
    int current = last_matchy;
    if(current == -1) current = 0;
    for(int i = 0; i <= E.numsrows; i++) {
        Erow* row = &E.row[current];
        std::string_view render = row->render;
        
        size_t pos;
        if(i == 0 && last_matchx != -1) {
            if(direction == 1) {
                if(last_matchx == row->render.size()) pos = std::string::npos;
                else {
                    pos = render.substr(last_matchx + 1, std::string::npos).find(query);
                    pos += pos == std::string::npos ? 0 : last_matchx + 1;
                }
            } else {
                if(last_matchx == 0) pos = std::string::npos;
                else pos = render.substr(0, last_matchx + query.size() - 1).rfind(query);
            }
        } else {
            if(direction == 1) pos = row->render.find(query);
            else pos = row->render.rfind(query);
        }
        
        if(pos != std::string::npos) {
            last_matchy = current;
            last_matchx = pos;
            E.cy = current;
            E.cx = Output::editorRowRxToCx(*row, pos);
            E.rowoff = E.numsrows;
            
            saved_hl_line = current;
            saved_hl = row->hl;
            
            std::fill(row->hl.begin() + pos, row->hl.begin() + pos + query.size(), HL_MATCH);
            break;
        }
        current += direction;
        if(current == -1) current = E.numsrows - 1;
        else if(current == E.numsrows) current = 0;
    }
}
