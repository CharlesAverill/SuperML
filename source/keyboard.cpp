#include "keyboard.h"
#include <string.h>

std::vector<Key> keyboard;
C2D_TextBuf globalKeyBuf;
C2D_Font keyboardFont;
bool shiftActive = false;
Key* shiftKey = nullptr;

void initKeyboard(void) {
    keyboard.clear();

    const char* row0 = "`1234567890-=";
    const char* row0Shift = "~!@#$%^&*()_+";
    
    const char* row1 = "qwertyuiop[]\\";
    const char* row1Shift = "QWERTYUIOP{}|";
    
    const char* row2 = "asdfghjkl;'";
    const char* row2Shift = "ASDFGHJKL:\"";
    
    const char* row3 = "zxcvbnm,./";
    const char* row3Shift = "ZXCVBNM<>?";

    const float horizontalMargin = WIN_W * 0.05f;
    const float availableWidth = WIN_W - 2 * horizontalMargin;
    const float interKey = WIN_W * 0.007f;

    const int maxKeys = 13;
    const float keyW = (availableWidth - interKey * (maxKeys - 1)) / maxKeys;
    const float keyH = WIN_H * 0.12f;

    const float startY = WIN_H * 0.06f;
    const float rowSpacing = keyH * 1.3f;

    auto addRow = [&](const char* row, const char* rowShift, float yOffset) {
        int len = strlen(row);
        float rowWidth = len * keyW + (len - 1) * interKey;
        float x0 = (WIN_W - rowWidth) / 2.0f;

        for (int i = 0; i < len; i++) {
            keyboard.push_back({
                std::string(1, row[i]),
                std::string(1, rowShift[i]),
                x0 + i * (keyW + interKey),
                startY + yOffset,
                keyW,
                keyH
            });
        }
    };

    // Row placement
    addRow(row0, row0Shift, 0.0f);
    addRow(row1, row1Shift, rowSpacing);
    addRow(row2, row2Shift, rowSpacing * 2);
    
    // Row 3 with shift key
    int len3 = strlen(row3);
    float shiftW = keyW * 1.5f;  // Shift key is wider
    float row3Width = len3 * keyW + (len3 - 1) * interKey;
    float x0 = (WIN_W - row3Width - shiftW - interKey) / 2.0f;
    
    // Shift key on the left
    keyboard.push_back({
        "Sh",
        "Sh",
        x0,
        startY + rowSpacing * 3,
        shiftW,
        keyH
    });
    shiftKey = &(keyboard.back());
    
    // Regular keys
    for (int i = 0; i < len3; i++) {
        keyboard.push_back({
            std::string(1, row3[i]),
            std::string(1, row3Shift[i]),
            x0 + shiftW + interKey + i * (keyW + interKey),
            startY + rowSpacing * 3,
            keyW,
            keyH
        });
    }

    // Space bar
    float spaceW = availableWidth * 0.75f;
    float spaceX = (WIN_W - spaceW) / 2.0f;
    float spaceY = startY + rowSpacing * 4;

    keyboard.push_back({
        " ",
        " ",
        spaceX,
        spaceY,
        spaceW,
        keyH
    });
}

int keyAt(int x, int y) {
    for (int i = 0; i < (int)keyboard.size(); i++) {
        Key &k = keyboard[i];
        if (x >= k.x && x <= k.x + k.w &&
            y >= k.y && y <= k.y + k.h) {
            return i;
        }
    }
    return -1;
}

char getKeyChar(const Key& k) {
    if (isShift(k)) return 0;
    return shiftActive ? k.shiftLabel[0] : k.label[0];
}

bool isShift(const Key& k) {
    return &k == shiftKey;
}

void drawKeyLabel(const Key& k, float scale, u32 color) {
    C2D_Text txt;

    std::string s = shiftActive ? k.shiftLabel : k.label;
    
    C2D_TextFontParse(&txt, keyboardFont, globalKeyBuf, s.c_str());

    float tw, th;
    C2D_TextGetDimensions(&txt, scale, scale, &tw, &th);

    float cx = k.x + (k.w - tw) * 0.5f;
    float cy = k.y + (k.h - th) * 0.5f;

    C2D_DrawText(&txt, 0, cx, cy, 0, scale, scale);
}

void drawKeyboard(void) {
    C2D_TextBufClear(globalKeyBuf);
    
    for (auto &k : keyboard) {
        // Highlight shift key when active
        u32 bgColor = (isShift(k) && shiftActive) ? 
            C2D_Color32(180, 200, 255, 255) : 
            C2D_Color32(230, 230, 230, 255);
            
        C2D_DrawRectSolid(k.x, k.y, 0, k.w, k.h, bgColor);
        C2D_DrawRectSolid(k.x, k.y, 0, k.w, 2, C2D_Color32(180, 180, 180, 255));
        C2D_DrawRectSolid(k.x, k.y + k.h - 2, 0, k.w, 2, C2D_Color32(180, 180, 180, 255));

        drawKeyLabel(k, 0.7f, C2D_Color32(0, 0, 0, 255));
    }
}
