#include "ui.h"
#include <strings.h>

#define BUFFER_SIZE 1025 // Notepad's line limit + \0

#define VERSION "SuperML"

PrintConsole topScreen, bottomScreen;
int scroll = 0;
bool fast_scroll = false;
std::string currentFilename;

void move_down(File &file);
void move_up(File &file);

unsigned int curr_line = 0;

File file;
static SwkbdState swkbd;

void keyboardInit(void) {
  consoleSelect(&bottomScreen);

  // Print some stuff on entry
  print_instructions();
  print_version();

  currentFilename = NEW_FN;

  swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 1, -1);
  swkbdSetValidation(&swkbd, SWKBD_ANYTHING, SWKBD_ANYTHING, 2);
  swkbdSetFeatures(&swkbd, SWKBD_DARKEN_TOP_SCREEN);

  update_screen(file, curr_line);
  show_logo();
}

void setupKeyboard(const char* hintText, const char* initialText) {
  swkbdSetHintText(&swkbd, hintText);
  swkbdSetInitialText(&swkbd, initialText);
}

void newFile(void) {
  File blankFile;
  file = blankFile;
  curr_line = 0;
  scroll = 0;
  currentFilename = NEW_FN;
  update_screen(file, curr_line);
}

static SwkbdCallbackResult validateYesNo(void* user, const char** ppMessage, const char* text, size_t textlen)
{
  *ppMessage = "Please enter one of \"y\", \"n\"";

  switch (text[0]) {
    case 'y':
    case 'Y':
      return SWKBD_CALLBACK_OK;
    case 'n':
    case 'N':
      return SWKBD_CALLBACK_CLOSE;
    default:
      return SWKBD_CALLBACK_CONTINUE;
  }
}

void keyboardMain(uint32_t kDown, uint32_t kHeld) {
  fast_scroll = false;

  static char swkbd_buf[BUFFER_SIZE];
  memset(swkbd_buf, '\0', BUFFER_SIZE);
  SwkbdButton button = SWKBD_BUTTON_NONE;
  bool entered_text = false;

  if (kDown) {
    swkbdSetInitialText(&swkbd, "");
  }

  if (kDown & KEY_A) { // Write/edit line
    char current_text[BUFFER_SIZE];
    auto line = file.lines.begin();

    // Get current line text
    if (curr_line < file.lines.size()) {
      advance(line, curr_line);

      if (curr_line == file.lines.size() - 1) {
        file.lines.push_back(std::vector<char>{'\n'});
      }

      size_t len = line->size();
      memcpy(current_text, line->data(), len);
      current_text[len] = '\0';
      swkbdSetInitialText(&swkbd, current_text);
    }

    // Get user input, modify file
    setupKeyboard("Line text goes here", current_text);
    if (swkbdInputText(&swkbd, swkbd_buf, sizeof(swkbd_buf)) != SWKBD_BUTTON_NONE) {
      std::vector<char> new_text = char_arr_to_vector(swkbd_buf);

      if (curr_line >= file.lines.size()) {
        // Empty line, add a new one.
        file.add_line(new_text);
      } else {
        file.edit_line(new_text, curr_line);
      }
      update_screen(file, curr_line);
    }
  } else if (kDown & KEY_B) { // Create file
    // Confirm creating a new file
    setupKeyboard("Open a new file? (y/n)", "");
    swkbdSetFilterCallback(&swkbd, validateYesNo, NULL);
    swkbdInputText(&swkbd, swkbd_buf, sizeof(swkbd_buf));

    // Create file or show an error
    if (swkbd_buf[0] == 'y') {
      newFile();
      status_message("New file created");
    } else status_message("Couldn't create new file");
  } else if (kDown & KEY_R) { // Select line with search text
    // Get term to search for
    setupKeyboard("Search term", "");
    swkbdInputText(&swkbd, swkbd_buf, sizeof(swkbd_buf));

    // Perform search
    int line = file.find(swkbd_buf);
    if (line < 0) {
      char error[2 * BUFFER_SIZE];
      sprintf(error, "Couldn't find %s in current file", swkbd_buf);
      status_message(error);
    } else {
      char msg[2 * BUFFER_SIZE];
      sprintf(msg, "Found %s at L%d", swkbd_buf, line);
      curr_line = line;
      if (curr_line > MAX_LINES) {
        scroll = curr_line - MAX_LINES;
      }
      update_screen(file, curr_line);
    }
  } else if (kHeld & KEY_L) {
    // If held, allows for jumping to end and start of file
    fast_scroll = true;
  } else if (kDown & KEY_X) { // Save file
    promptSaveFile();
  } else if (kDown & KEY_Y) {
    // Similar code to pressing X, see about refactoring
    // Open a file
    curr_line = 0;
    scroll = 0;
    // Clear buffer
    memset(swkbd_buf, '\0', BUFFER_SIZE);

    // Get file name

    swkbdSetHintText(&swkbd, "Input filename here.");
    button = swkbdInputText(&swkbd, swkbd_buf, sizeof(swkbd_buf));
    std::string filename = "";
    for (int i = 0; swkbd_buf[i] != '\0'; i++)
      filename.push_back(swkbd_buf[i]);

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

  if (entered_text && button != SWKBD_BUTTON_NONE) {
    std::vector<char> new_text = char_arr_to_vector(swkbd_buf);

    if (curr_line >= file.lines.size()) {
      // Empty line, add a new one.
      file.add_line(new_text);
    } else {
      file.edit_line(new_text, curr_line);
    }
    update_screen(file, curr_line);
  }
}

void move_down(File &file) {
  // Move a line down (towards bottom of screen)

  if (fast_scroll) {
    curr_line = file.lines.size();
    scroll = curr_line - MAX_LINES;
  } else {
    if ((curr_line - scroll >= MAX_LINES)) {
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

bool promptSaveFile(void) {
  char filename_buf[128];
  // Get filename
  setupKeyboard("Filename to save as", "");
  swkbdInputText(&swkbd, filename_buf, 128);
  std::string filename = filename_buf;

  // Write to file
  if (write_to_file(filename, file)) {
    currentFilename = filename_buf;
    status_message("Wrote to " + filename);
    return true;
  } else {
    status_message("Couldn't write to " + filename);
    return false;
  }
}
