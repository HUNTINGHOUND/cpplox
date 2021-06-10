#include "startEditor.hpp"
#include "includes.hpp"
#include "terminal.hpp"
#include "input.hpp"
#include "output.hpp"
#include "fileio.hpp"



int startEditor(std::string& filename) {
    Terminal::enableRawMode();
    Terminal::initEditor();
    if(!filename.empty()) {
        FileIO::editorOpen(filename);
    }
    
    Output::editorSetStatusMessage("HELP: Ctrl-Q = quit | Ctrl-S = save | Ctrl-F = find", std::vector<std::string>{});
    
    while(true) {
        Output::editorRefreashScreen();
        Input::editorProcessKeypress();
    }
    
    return 0;
}
