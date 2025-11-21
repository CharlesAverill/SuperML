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

  aptSetHomeAllowed(false);

  keybaordInit();

  Result rc = romfsInit();
	if (rc)
		printf("romfsInit: %08lX\n", rc);

  show_logo();

  while (aptMainLoop()) {
    gspWaitForVBlank();
    // gfxFlushBuffers();
    gfxSwapBuffers();
    hidScanInput();
    u32 kDown = hidKeysDown();
    u32 kHeld = hidKeysHeld();

    keyboardMain(kDown, kHeld);
    if (kDown & KEY_START) {
      break;
    } else if (kDown & KEY_SELECT) {
      clear_top_screen();
      consoleSelect(&topScreen);
      interpreterMain(currentFilename);
    }

    // Flush and swap framebuffers
  }

  gfxExit();
  return 0;
}

