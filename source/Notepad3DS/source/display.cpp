#include "display.h"
#include "../../version.h"
#include "file_io.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <iterator>
#include <math.h>
#include <stdio.h>
#include <string>

void clear_range(PrintConsole *screen, std::string start_point, int clear_lines,
                 int clear_width) {
  consoleSelect(screen);
  std::cout << start_point;
  for (int i = 0; i < clear_lines; i++)
    std::cout << std::string(clear_width, ' ');
  std::cout << start_point;
}

void clear_top_screen() {
  clear_range(&topScreen, SCREEN_START_POINT, CLEAR_SCREEN_LINES, 70);
}

void clear_bottom_screen() {
  clear_range(&bottomScreen, SCREEN_START_POINT, CLEAR_SCREEN_LINES, 70);
}

void show_logo(void) {
  clear_top_screen();
  printf(LOGO_STR);
}

void print_centered_header(std::string text, char padding) {
  int text_len = text.length();

  // Calculate padding on each side
  int total_equals = MAX_BOT_WIDTH - text_len - 2;
  int left_equals = total_equals / 2;
  int right_equals = total_equals - left_equals;

  // Print the header
  std::cout << std::string(left_equals, padding) << " " << text << " "
            << std::string(right_equals, padding);
}

void print_version() {
  consoleSelect(&bottomScreen);
  printf(VERSION_LINE);
  print_centered_header(APP_TITLE + (std::string) " " + APP_VERSION, ' ');
}

void print_instructions() {
  consoleSelect(&bottomScreen);
  printf(INSTRUCTIONS_LINE);
  print_centered_header("CONTROLS", '-');
  printf("(A): Edit selected line\n"
         "(B): New file\n"
         "(X): Save file\n"
         "(Y): Open file\n"
         "(R): Search\n"
         "(L + DPad): Jump to top/bottom\n"
         "(DPad): Change selected line\n"
         "(SELECT): Parse and Run\n");
}

void print_status(File &file, int current_line) {
  clear_range(&bottomScreen, STATUS_LINE, STATUS_LINES, MAX_BOT_WIDTH);
  print_centered_header("STATUS", '-');
  printf("%s:L%d/%d\n", currentFilename.c_str(), current_line,
         file.lines.size());
}

void status_message(std::string msg) {
  clear_range(&bottomScreen, STATUS_MSG_LINE, 1, MAX_BOT_WIDTH);

  // Compute number of spaces to fill
  int pad = MAX_BOT_WIDTH - 2 - msg.size() - 1; // -2 for '[' and ']'
  if (pad < 0)
    pad = 0;

  printf("[ %s", strip(msg).c_str());
  for (int i = 0; i < pad; i++)
    putchar(' ');
  printf("]\n");
}

void print_text(const char *str, unsigned int count,
                unsigned int selected_line) {
  if (count == selected_line)
    printf(SELECTED_TEXT_COLOUR);

  if (str[0] == '\n' && count == selected_line)
    printf("(empty line)\n");
  else
    printf("%s", str);

  printf(DEFAULT_TEXT_COLOUR);
}

void update_screen(File &file, unsigned int current_line,
                   const std::vector<unsigned int> &lines_to_redraw) {
  scroll = current_line > MAX_LINES ? current_line - MAX_LINES : 0;

  print_status(file, current_line);
  status_message("");

  unsigned int start_line = scroll;
  unsigned int end_line = scroll + MAX_LINES;
  if (end_line >= file.lines.size())
    end_line = file.lines.size() - 1;

  consoleSelect(&topScreen);

  bool force_redraw = false;
#ifdef __3DS__
  // moving up with the flicker optimization kills the application, probably OOB
  // read as file lines get clipped off
  force_redraw = true;
#endif

  if (lines_to_redraw.empty() || force_redraw) {
    // Draw entire visible screen
    clear_top_screen();

    auto iter = file.lines.begin();
    std::advance(iter, start_line);

    for (unsigned int file_idx = start_line; file_idx <= end_line;
         file_idx++, iter++) {
      std::string line_str = char_vec_to_string(*iter);

      // Move cursor to line
      printf("\x1b[%d;0H", file_idx - start_line + 1);

      // Clear line first
      printf("\033[2K");

      // Print text
      print_text(line_str.c_str(), file_idx - start_line,
                 current_line - scroll);
    }

    // Clear any remaining screen lines below the last line of the file
    for (unsigned int screen_idx = end_line + 1; screen_idx <= end_line;
         screen_idx++) {
      printf("\x1b[%d;0H\033[2K", screen_idx - start_line + 1);
    }
  } else {
    // Only redraw specified lines
    for (unsigned int line_idx : lines_to_redraw) {
      if (line_idx < start_line || line_idx > end_line ||
          !(file.lines.size() <= line_idx))
        continue; // skip lines not visible

      auto iter = file.lines.begin();
      std::advance(iter, line_idx);
      std::string line_str = char_vec_to_string(*iter);

      // Move cursor to the correct row
      printf("\x1b[%d;0H", line_idx - start_line + 1);

      // Clear line if empty or just '\n'
      printf("\033[2K");

      // Print text
      print_text(line_str.c_str(), line_idx - start_line,
                 current_line - scroll);
    }

    // Additionally, clear lines beyond the current file end that were visible
    int last_visible_line = start_line + MAX_LINES;
    for (int line_idx = file.lines.size(); line_idx <= last_visible_line;
         line_idx++) {
      clear_range(&topScreen,
                  "\x1b[" + std::to_string(line_idx - start_line + 1) + ";0H",
                  1, 70);
    }
  }
}
