#ifndef input_hpp
#define input_hpp

#include "includes.hpp"

namespace Input {
void editorProcessKeypress();
void editorMoveCursor(int key);
std::string editorPrompt(const std::string& prompt, std::function<void(std::string&, int)>* callback);
}

#endif /* input_hpp */
