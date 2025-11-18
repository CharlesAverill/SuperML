#include <3ds.h>
#include "globals.h"
#include "keyboard.h"
#include "lang/interpreter.h"

void stepCallback(State* state) {

}

int main(int argc, char **argv) {
  gfxInitDefault();
  consoleInit(GFX_TOP, &topScreen);
  consoleInit(GFX_BOTTOM, &bottomScreen);

  keybaordInit();

  while (aptMainLoop()) {
    hidScanInput();
    u32 kDown = hidKeysDown();
    u32 kHeld = hidKeysHeld();

    keyboardMain(kDown, kHeld);

    if (kDown & KEY_SELECT) {
      clear_screen();
      consoleSelect(&topScreen);
      interpreterMain(getFile());
    }

    // Flush and swap framebuffers
    gfxFlushBuffers();
    gfxSwapBuffers();

    gspWaitForVBlank();
  }

  gfxExit();
  return 0;
}

