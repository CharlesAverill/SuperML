#include <3ds.h>
#include "globals.h"
#include "ui.h"
#include "lang/interpreter.h"

void stepCallback(State state) {

}

int main(int argc, char **argv) {
  gfxInitDefault();
  consoleInit(GFX_TOP, &topScreen);
  consoleInit(GFX_BOTTOM, &bottomScreen);

  // aptSetHomeAllowed(false);

  keyboardInit();

  Result rc = romfsInit();
	if (rc)
		printf("romfsInit: %08lX\n", rc);

  while (aptMainLoop()) {
    gspWaitForVBlank();
    // gfxFlushBuffers();
    gfxSwapBuffers();
    hidScanInput();
    u32 kDown = hidKeysDown();
    u32 kHeld = hidKeysHeld();

    keyboardMain(kDown, kHeld);
    if (kDown & KEY_SELECT) {
      bool do_run = true;
      if (currentFilename == NEW_FN) {
        do_run = promptSaveFile();
      }

      if (do_run) {
        // clear_top_screen();
        interpreterMain(currentFilename);
      }
    }
  }

  gfxExit();
  return 0;
}

