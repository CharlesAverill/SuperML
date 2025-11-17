#ifndef KEYBOARD
#define KEYBOARD

#include <vector>
#include <string>
#include <citro2d.h>
#include "globals.h"

struct Key {
    std::string label;
    std::string shiftLabel;
    float x, y, w, h;
};

extern std::vector<Key> keyboard;
extern C2D_TextBuf globalKeyBuf;
extern C2D_Font keyboardFont;
extern bool shiftActive;

void initKeyboard(void);
void drawKeyboard(void);
int keyAt(int x, int y);
char getKeyChar(const Key& k);
bool isShift(const Key& k);

#endif /* KEYBOARD */
