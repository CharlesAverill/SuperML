#ifndef KEYBOARD
#define KEYBOARD

#include <stdint.h>
#include <algorithm>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include "Notepad3DS/source/display.h"
#include "Notepad3DS/source/file.h"
#include "Notepad3DS/source/file_io.h"

void keybaordInit(void);
void keyboardMain(uint32_t kDown, uint32_t kHeld);

File* getFile(void);

#endif /* KEYBOARD */
