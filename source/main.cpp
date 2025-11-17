#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <math.h>
#include <string>

#include "keyboard.h"

int main(int argc, char** argv) {
    gfxInitDefault();
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Prepare();

    C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    C3D_RenderTarget* bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    initKeyboard();

    std::string buffer = "let () =\n"
                         "  print_endline \"hello world!\";\n"
                         "  print_int 42";
    size_t cursor = 0;

    keyboardFont = C2D_FontLoadSystem(CFG_REGION_USA);
    PrintConsole bottomConsole;

    C2D_TextBuf measureBuf = C2D_TextBufNew(4096);
    C2D_TextBuf drawBuf    = C2D_TextBufNew(4096);
    globalKeyBuf = C2D_TextBufNew(100);
    const float topMaxWidth = 380.0f;
    const float lineHeight = 14.0f;

    while (aptMainLoop()) {
        hidScanInput();
        touchPosition touch;
        hidTouchRead(&touch);

        u32 down = hidKeysDown();

        if (down & KEY_START) break;

        // Detect screen taps for keyboard
        if (down & KEY_TOUCH) {
            int idx = keyAt(touch.px, touch.py);
            if (idx != -1) {
                Key& k = keyboard[idx];
                
                if (isShift(k)) {
                    // Toggle shift
                    shiftActive = !shiftActive;
                } else {
                    // Insert character
                    char c = getKeyChar(k);
                    if (c != 0) {  // getKeyChar returns 0 for shift key
                        buffer.insert(buffer.begin() + cursor, c);
                        cursor++;
                        
                        // Un-toggle shift after typing
                        shiftActive = false;
                    }
                }
            }
        }

        // Cursor movement
        if (down & KEY_LEFT && cursor > 0) cursor--;
        if (down & KEY_RIGHT && cursor < buffer.size()) cursor++;

        // Prepare frame
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

        // --- TOP SCREEN (text buffer) ---
        C2D_TargetClear(top, C2D_Color32(255,255,255,255));
        C2D_SceneBegin(top);

        C2D_TextBufClear(measureBuf);
        C2D_TextBufClear(drawBuf);
        C2D_TextBufClear(globalKeyBuf);

        float x = 10.0f;
        float y = 10.0f;
        size_t pos = 0;
        while (pos < buffer.size()) {
            // find how many characters fit on this line by trying increasingly larger substrings
            size_t len = 0;
            float used = 0.0f;

            // try to grow the line until it would overflow or hit newline
            for (size_t j = pos; j < buffer.size(); ++j) {
                if (buffer[j] == '\n') { len = j - pos + 1; break; }

                // prepare a small temporary null-terminated substring (or use C++ substr)
                std::string candidate = buffer.substr(pos, j - pos + 1);
                C2D_Text parsed;
                C2D_TextParse(&parsed, measureBuf, candidate.c_str());
                float w, h;
                C2D_TextGetDimensions(&parsed, 0.6f, 0.6f, &w, &h);

                if (w > topMaxWidth) {
                    if (j == pos) {
                        // single char too wide: force at least one char to avoid infinite loop
                        len = 1;
                    }
                    break;
                } else {
                    len = j - pos + 1;
                    used = w;
                }
            }

            std::string line = buffer.substr(pos, len);
            C2D_Text text;
            C2D_TextParse(&text, drawBuf, line.c_str());
            C2D_DrawText(&text, C2D_WithColor, x, y, 0.0f, 0.6f, 0.6f, C2D_Color32(0,0,0,255));

            pos += len;
            y += lineHeight;
        }

        // --- BOTTOM SCREEN (keyboard) ---
        C2D_TargetClear(bottom, C2D_Color32(200,200,200,255));
        C2D_SceneBegin(bottom);

        drawKeyboard();

        C3D_FrameEnd(0);
    }

    C2D_TextBufDelete(measureBuf);
    C2D_TextBufDelete(drawBuf);
    C2D_TextBufDelete(globalKeyBuf);
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    return 0;
}
