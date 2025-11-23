#include "display.h"
#include "file_io.h"
#include <iostream>
#include <iterator>
#include <math.h>
#include <stdio.h>
#include <string>
#include <algorithm>
#include <cctype>

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
  print_centered_header(VERSION, ' ');
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
         "(SELECT): Interpret and Run\n");
}

void print_status(File &file, int current_line) {
  clear_range(&bottomScreen, STATUS_LINE, STATUS_LINES, MAX_BOT_WIDTH);
  print_centered_header("STATUS", '-');
  printf("%s:L%d/%d\n", currentFilename.c_str(), current_line,
         file.lines.size());
}

std::string strip(std::string s) {
    auto start = std::find_if_not(s.begin(), s.end(), ::isspace);
    auto end   = std::find_if_not(s.rbegin(), s.rend(), ::isspace).base();
    if (start >= end) return ""; // all spaces
    return std::string(start, end);
}

void status_message(std::string msg) {
    clear_range(&bottomScreen, STATUS_MSG_LINE, 1, MAX_BOT_WIDTH);

    // Compute number of spaces to fill
    int pad = MAX_BOT_WIDTH - 2 - msg.size() - 1; // -2 for '[' and ']'
    if (pad < 0) pad = 0;

    printf("[ %s", strip(msg).c_str());
    for (int i = 0; i < pad; i++) putchar(' ');
    printf("]\n");
}

std::string char_vec_to_string(std::vector<char> &line) {

  std::string temp_str = "";
  int letters = 0;
  for (const auto &ch : line) {
    if (letters != MAX_TOP_WIDTH) {
      // Store characters to display
      temp_str.push_back(ch);
      letters++;
    } else {
      // Too much text, display new line
      temp_str.push_back('\n');
      break;
    }
  }
  return temp_str;
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

void update_screen(File &file, unsigned int current_line) {
    scroll = current_line > MAX_LINES ? current_line - MAX_LINES : 0;

    print_status(file, current_line);

    int start_line = scroll;
    int end_line = scroll + MAX_LINES;
    if (end_line >= (int)file.lines.size())
        end_line = file.lines.size() - 1;

    clear_top_screen();

    auto iter = file.lines.begin();
    advance(iter, start_line);

    for (int file_idx = start_line; file_idx <= end_line; file_idx++, iter++) {
        std::string temp = char_vec_to_string(*iter);   // keep alive
        const char *str_to_print = temp.c_str();        // safe
        print_text(str_to_print, file_idx - start_line, current_line - scroll);
    }
}
