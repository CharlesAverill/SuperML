#include "Notepad3DS/source/display.h"
#include "Notepad3DS/source/file.h"
#include "Notepad3DS/source/file_io.h"
#include <3ds.h>
#include <algorithm>
#include <iostream>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 1025 // Notepad's line limit + \0
#define MAX_BOTTOM_SIZE 28

#define VERSION "SuperML"

PrintConsole topScreen, bottomScreen;
int scroll = 0;
bool fast_scroll = false;
std::string currentFilename;

void move_down(File &file);
void move_up(File &file);

unsigned int curr_line = 0;

int main(int argc, char **argv) {
  gfxInitDefault();
  consoleInit(GFX_TOP, &topScreen);
  consoleInit(GFX_BOTTOM, &bottomScreen);
  consoleSelect(&bottomScreen);
  // Software keyboard thanks to fincs
  print_instructions();

  print_version(VERSION);

  File file; // Use as default file
  currentFilename = "(new file)";

  update_screen(file, curr_line);

  while (aptMainLoop()) {
    fast_scroll = false;

    hidScanInput();

    u32 kDown = hidKeysDown();
    u32 kHeld = hidKeysHeld();

    // if (kDown & KEY_START)
    //   break;

    static SwkbdState swkbd;
    static char mybuf[BUFFER_SIZE];
    SwkbdButton button = SWKBD_BUTTON_NONE;
    bool entered_text = false;

    swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 1, -1);
    swkbdSetValidation(&swkbd, SWKBD_ANYTHING, SWKBD_ANYTHING, 2);
    swkbdSetFeatures(&swkbd, SWKBD_DARKEN_TOP_SCREEN);

    if (kDown & KEY_A) {
      // Select current line for editing
      swkbdSetHintText(&swkbd, "Input text here.");
      // Iterator to find current selected line
      auto line = file.lines.begin();
      if (curr_line < file.lines.size()) {
        if (curr_line != 0)
          advance(line, curr_line);

        if (curr_line == file.lines.size() - 1) {
          file.lines.push_back(std::vector<char>{'\n'});
        }
        // Need a char array to output to keyboard
        char current_text[BUFFER_SIZE] = "";
        copy(line->begin(), line->end(), current_text);
        swkbdSetInitialText(&swkbd, current_text);
      }
      entered_text = true;
      button = swkbdInputText(&swkbd, mybuf, sizeof(mybuf));
    } else if (kDown & KEY_B) {
      // Create new file

      // Clear buffer
      memset(mybuf, '\0', BUFFER_SIZE);
      // Confirm creating a new file
      swkbdSetHintText(&swkbd,
                       "Are you sure you want to open a BLANK file? y/n");
      button = swkbdInputText(&swkbd, mybuf, sizeof(mybuf));
      if (mybuf[0] == 'y') {
        File blankFile;
        file = blankFile;
        curr_line = 0;
        scroll = 0;
        update_screen(file, curr_line);
        print_save_status("New file created");
      } else
        print_save_status("No new file created");
    } else if (kDown & KEY_R) {
      // find a thing

      // Clear buffer
      memset(mybuf, '\0', BUFFER_SIZE);
      // Get term to search for
      swkbdSetHintText(&swkbd, "Input search term here.");
      button = swkbdInputText(&swkbd, mybuf, sizeof(mybuf));
      int line = file.find(mybuf);
      if (line < 0)
        printf("Could not find %s", mybuf);
      else {
        printf("Found %s at %d", mybuf, line);
        curr_line = line;
        if (curr_line > MAX_BOTTOM_SIZE) {
          scroll = curr_line - MAX_BOTTOM_SIZE;
        }
        update_screen(file, curr_line);
      }
    } else if (kHeld & KEY_L) {
      // If held, allows for jumping to end and start of file
      fast_scroll = true;
    } else if (kDown & KEY_X) {
      // Save current file
      // Clear buffer
      memset(mybuf, '\0', BUFFER_SIZE);

      // Get file name

      swkbdSetHintText(&swkbd, "Input filename here.");
      button = swkbdInputText(&swkbd, mybuf, sizeof(mybuf));
      std::string filename = "";
      for (int i = 0; mybuf[i] != '\0'; i++)
        filename.push_back(mybuf[i]);

      // Write out characters to file
      bool success = write_to_file(filename, file);

      if (success) {
        status_message("Wrote to " + filename);
      } else {
        status_message("Couldn't write to " + filename);
      }
    } else if (kDown & KEY_Y) {
      // Similar code to pressing X, see about refactoring
      // Open a file
      curr_line = 0;
      scroll = 0;
      // Clear buffer
      memset(mybuf, '\0', BUFFER_SIZE);

      // Get file name

      swkbdSetHintText(&swkbd, "Input filename here.");
      button = swkbdInputText(&swkbd, mybuf, sizeof(mybuf));
      std::string filename = "";
      for (int i = 0; mybuf[i] != '\0'; i++)
        filename.push_back(mybuf[i]);
      
      if (!filename.empty()) {
        File oldfile = file;
        file = open_file(filename);

        // print functions here seem to crash the program
        if (file.read_success) {
          currentFilename = filename;
          update_screen(file, curr_line);
          status_message("Opened " + filename);
        } else {
          file = oldfile;
          update_screen(file, curr_line);
          status_message("Failed to open " + filename);
        }
      }
    }

    if ((kDown & KEY_DDOWN) | (kHeld & KEY_CPAD_DOWN)) {
      // Move a line down (towards bottom of screen)
      move_down(file);
    } else if ((kDown & KEY_DUP) | (kHeld & KEY_CPAD_UP)) {
      // Move a line up (towards top of screen)
      move_up(file);
    }

    if (entered_text) {
      if (button != SWKBD_BUTTON_NONE) {
        std::vector<char> new_text = char_arr_to_vector(mybuf);

        if (curr_line >= file.lines.size()) {
          // Empty line, add a new one.
          file.add_line(new_text);
        } else {
          file.edit_line(new_text, curr_line);
        }
        update_screen(file, curr_line);
      } else
        printf("swkbd event: %d\n", swkbdGetResult(&swkbd));
    }

    // Flush and swap framebuffers
    gfxFlushBuffers();
    gfxSwapBuffers();

    gspWaitForVBlank();
  }

  gfxExit();
  return 0;
}

void move_down(File &file) {
  // Move a line down (towards bottom of screen)

  if (fast_scroll) {
    curr_line = file.lines.size();
    scroll = curr_line - MAX_BOTTOM_SIZE;
  } else {
    if ((curr_line - scroll >= MAX_BOTTOM_SIZE)) {
      scroll++;
    }
    curr_line++;
  }

  // Add new empty lines as needed
  while (curr_line >= file.lines.size()) {
    file.lines.push_back(std::vector<char>{'\n'});
  }

  update_screen(file, curr_line);
}

void move_up(File &file) {
  // Move a line up (towards top of screen)
  if (curr_line != 0) {
    if (fast_scroll) {
      // Jump to the top
      curr_line = 0;
      scroll = 0;
    } else {
      curr_line--;
      if (curr_line - scroll <= 0 && scroll != 0) {
        scroll--;
      }
    }

    // Trim trailing empty lines after the current line
    while (file.lines.size() > curr_line + 1) {
      // Check if the last line is empty
      auto &lastLine = file.lines.back();
      if (lastLine.size() == 1 && lastLine[0] == '\n') {
        file.lines.pop_back();
      } else {
        break; // Stop if we hit a non-empty line
      }
    }
  }
  update_screen(file, curr_line);
}
