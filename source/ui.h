#ifndef KEYBOARD
#define KEYBOARD

#include "Notepad3DS/source/display.h"
#include "Notepad3DS/source/file.h"
#include "Notepad3DS/source/file_io.h"
#include <algorithm>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void keyboardInit(void);
void keyboardMain(uint32_t kDown, uint32_t kHeld);

bool promptSaveFile(void);
bool saveFile(std::string filename);

#endif /* KEYBOARD */
