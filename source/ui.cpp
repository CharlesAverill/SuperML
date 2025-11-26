#include "ui.h"
#include <strings.h>

#define BUFFER_SIZE 1025 // Notepad's line limit + \0

#define VERSION "SuperML"

PrintConsole topScreen, bottomScreen;
unsigned int scroll = 0;
bool fast_scroll = false;
bool unsavedChanges = false;
std::string currentFilename;
unsigned int cursor_pos = 0;

void move_down(File &file);
void move_up(File &file);
void move_left(File &file);
void move_right(File &file);

unsigned int current_file_line = 0;

File file;
SwkbdState swkbd;

void normalKeyboardInit(void) {
  swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 1, -1);
  swkbdSetValidation(&swkbd, SWKBD_ANYTHING, 0, 2);
  swkbdSetFeatures(&swkbd, SWKBD_DARKEN_TOP_SCREEN);
}

void intKeyboardInit(void) {
  swkbdInit(&swkbd, SWKBD_TYPE_NUMPAD, 1, -1);
  swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, 2);
  swkbdSetFeatures(&swkbd, SWKBD_DARKEN_TOP_SCREEN);
}

static SwkbdCallbackResult validateFloat(void *user, const char **ppMessage,
                                         const char *text, size_t textlen) {
  *ppMessage = "Please enter a valid floating-\npoint number";

  if (textlen == 0)
    return SWKBD_CALLBACK_CONTINUE;

  bool seenDigit = false;
  bool seenDot = false;

  for (size_t i = 0; i < textlen; i++) {
    char c = text[i];

    if (i == 0 && (c == '+' || c == '-')) {
      // leading sign is OK
      continue;
    }

    if (c >= '0' && c <= '9') {
      seenDigit = true;
      continue;
    }

    if (c == '.') {
      if (seenDot) {
        // second dot → reject
        return SWKBD_CALLBACK_CONTINUE;
      }
      seenDot = true;
      continue;
    }

    // Any other character is invalid
    return SWKBD_CALLBACK_CONTINUE;
  }

  // Must contain at least one digit
  if (!seenDigit)
    return SWKBD_CALLBACK_CONTINUE;

  return SWKBD_CALLBACK_OK;
}

void floatKeyboardInit(void) {
  swkbdInit(&swkbd, SWKBD_TYPE_QWERTY, 1, -1);
  swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, 2);
  swkbdSetFeatures(&swkbd, SWKBD_DARKEN_TOP_SCREEN);
  swkbdSetFilterCallback(&swkbd, validateFloat, NULL);
}

void keyboardInit(void) {
  consoleSelect(&bottomScreen);

  // Print some stuff on entry
  print_instructions();
  print_version();

  currentFilename = NEW_FN;

  normalKeyboardInit();

  update_screen(file, current_file_line);
  show_logo();
}

void setupKeyboard(const char *hintText, const char *initialText) {
  swkbdSetHintText(&swkbd, hintText);
  swkbdSetInitialText(&swkbd, initialText);
}

void newFile(void) {
  File blankFile;
  file = blankFile;
  current_file_line = 0;
  scroll = 0;
  currentFilename = NEW_FN;
  unsavedChanges = true;
  update_screen(file, current_file_line);
}

static SwkbdCallbackResult validateYesNo(void *user, const char **ppMessage,
                                         const char *text, size_t textlen) {
  *ppMessage = "Please enter one of \"y\", \"n\"";

  if (textlen < 1)
    return SWKBD_CALLBACK_CONTINUE;

  switch (text[0]) {
  case 'y':
  case 'Y':
  case 'n':
  case 'N':
    return SWKBD_CALLBACK_OK;
  default:
    return SWKBD_CALLBACK_CONTINUE;
  }
}

bool promptYesNo() {
  static char buf[BUFFER_SIZE];
  memset(buf, '\0', BUFFER_SIZE);
  swkbdSetFilterCallback(&swkbd, validateYesNo, NULL);
  swkbdInputText(&swkbd, buf, BUFFER_SIZE);
  return buf[0] == 'y' || buf[0] == 'Y';
}

void promptSaveChanges() {
  setupKeyboard("Save unsaved changes? (y/n)", "");
  if (promptYesNo()) {
  }
  // 	if (currentFilename == NEW_FN)
  // 		promptSaveFile();
  //   else
  // 		saveFile(currentFilename);
  // }
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
    if (current_file_line < file.lines.size()) {
      advance(line, current_file_line);

      if (current_file_line == file.lines.size() - 1) {
        file.lines.push_back(std::vector<char>{'\n'});
      }

      size_t len = line->size();
      memcpy(current_text, line->data(), len);
      current_text[len] = '\0';
      swkbdSetInitialText(&swkbd, current_text);
    }

    // Get user input, modify file
    setupKeyboard("Line text", current_text);
    if (swkbdInputText(&swkbd, swkbd_buf, sizeof(swkbd_buf)) !=
        SWKBD_BUTTON_NONE) {
      unsavedChanges = true;
      std::vector<char> new_text = char_arr_to_vector(swkbd_buf);

      if (current_file_line >= file.lines.size()) {
        // Empty line, add a new one.
        file.add_line(new_text);
      } else {
        file.edit_line(new_text, current_file_line);
      }
      update_screen(file, current_file_line);
    }
  } else if (kDown & KEY_B) { // Create file
    // if (unsavedChanges)
    // 	promptSaveChanges();
    setupKeyboard("Open a new file? (y/n)", "");
    swkbdSetFilterCallback(&swkbd, validateYesNo, NULL);
    swkbdInputText(&swkbd, swkbd_buf, sizeof(swkbd_buf));

    // Create file or show an error
    if (swkbd_buf[0] == 'y') {
      newFile();
      status_message("New file created");
    } else
      status_message("Couldn't create new file");
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
      current_file_line = line;
      if (current_file_line > MAX_LINES) {
        scroll = current_file_line - MAX_LINES;
      }
      update_screen(file, current_file_line);
    }
  } else if (kHeld & KEY_L) {
    // If held, allows for jumping to end and start of file
    fast_scroll = true;
  } else if (kDown & KEY_X) { // Save file
    promptSaveFile();
  } else if (kDown & KEY_Y) { // Open a file
    current_file_line = 0;
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
        update_screen(file, current_file_line);
        status_message("Opened " + filename);
      } else {
        file = oldfile;
        update_screen(file, current_file_line);
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
  } else if ((kDown & KEY_DLEFT) | (kHeld & KEY_CPAD_LEFT)) {
    // Move cursor left
    move_left(file);
  } else if ((kDown & KEY_DRIGHT) | (kHeld & KEY_CPAD_RIGHT)) {
    move_right(file);
  }

  if (entered_text && button != SWKBD_BUTTON_NONE) {
    std::vector<char> new_text = char_arr_to_vector(swkbd_buf);

    if (current_file_line >= file.lines.size()) {
      // Empty line, add a new one.
      file.add_line(new_text);
    } else {
      file.edit_line(new_text, current_file_line);
    }
    update_screen(file, current_file_line);
  }
}

void move_down(File &file) {
  // Move a line down (towards bottom of screen)

  unsigned int old_line = current_file_line;

  if (fast_scroll) {
    current_file_line = file.lines.size();
    scroll = current_file_line - MAX_LINES;
  } else {
    if ((current_file_line - scroll >= MAX_LINES)) {
      scroll++;
    }
    current_file_line++;
  }

  // Add new empty lines as needed
  while (current_file_line >= file.lines.size()) {
    file.lines.push_back(std::vector<char>{'\n'});
  }

  cursor_pos = 0;

  update_screen(file, current_file_line, {old_line, current_file_line});
}

void move_up(File &file) {
  unsigned int old_line = current_file_line;

  // Move a line up (towards top of screen)
  if (current_file_line != 0) {
    if (fast_scroll) {
      // Jump to the top
      current_file_line = 0;
      scroll = 0;
    } else {
      current_file_line--;
      if (current_file_line - scroll <= 0 && scroll != 0) {
        scroll--;
      }
    }

    // Trim trailing empty lines after the current line
    while (file.lines.size() > current_file_line + 1) {
      // Check if the last line is empty
      auto &lastLine = file.lines.back();
      if (lastLine.size() == 1 && lastLine[0] == '\n') {
        file.lines.pop_back();
      } else {
        break; // Stop if we hit a non-empty line
      }
    }
  }

  cursor_pos = 0;

  update_screen(file, current_file_line, {old_line, current_file_line});
}

// We do length - 2 instead of length - 1 to skip the newlines
void move_left(File &file) {
  std::string line = char_vec_to_string(file.lines[current_file_line]);
  if (fast_scroll)
    cursor_pos = 0;
  else
    cursor_pos = cursor_pos > 0 ? cursor_pos - 1 : line.length() - 2;
  update_screen(file, current_file_line);
}

void move_right(File &file) {
  std::string line = char_vec_to_string(file.lines[current_file_line]);
  if (fast_scroll)
    cursor_pos = line.length() - 2;
  else
    cursor_pos = cursor_pos < line.length() - 2? cursor_pos + 1 : 0;
  update_screen(file, current_file_line);
}

bool promptSaveFile(void) {
  char filename_buf[128];
  // Get filename
  setupKeyboard("Filename to save as",
                (currentFilename == NEW_FN ? "" : currentFilename).c_str());
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

bool saveFile(std::string filename) {
  if (write_to_file(filename, file)) {
    currentFilename = filename;
    status_message("Wrote to " + filename);
    return true;
  } else {
    status_message("Couldn't write to " + filename);
    return false;
  }
}
