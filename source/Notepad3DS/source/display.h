#pragma once

#include "../../globals.h"
#include "file.h"
#include <3ds.h>
#include <string>

#define VERSION "SuperML"

#define CLEAR_SCREEN_LINES 40
#define MAX_LINES 28
#define MAX_TOP_WIDTH 49
#define MAX_BOT_WIDTH 40
#define SCREEN_START_POINT "\x1b[0;0H"
#define INSTRUCTIONS_LINE "\x1b[1;0H"
#define STATUS_LINE "\x1b[11;0H"
#define STATUS_LINENO 11
#define STATUS_LINES 3
#define STATUS_MSG_LINE "\x1b[14;0H"
#define VERSION_LINE "\x1b[28;0H"

#define DEFAULT_TEXT_COLOUR "\x1b[0m"
#define SELECTED_TEXT_COLOUR "\x1b[47;30m"

extern PrintConsole topScreen, bottomScreen;
extern unsigned int scroll;
extern std::string currentFilename;
extern unsigned int cursor_pos;

void clear_top_screen();
void clear_bottom_screen();
void update_screen(File &file, unsigned int current_line,
                   const std::vector<unsigned int> &lines_to_redraw = {});
void print_text(const char *str, unsigned int count,
                unsigned int selected_line);

void print_instructions();
void print_version();
void status_message(std::string str);

void show_logo(void);

std::string char_vec_to_string(std::vector<char> &line);
