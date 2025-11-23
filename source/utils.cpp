#include "utils.h"

#ifndef __3DS__
#define MAX_TOP_WIDTH __UINT64_MAX__
#else
#include "Notepad3DS/source/display.h"
#endif

std::string strip(std::string s) {
  auto start = std::find_if_not(s.begin(), s.end(), ::isspace);
  auto end = std::find_if_not(s.rbegin(), s.rend(), ::isspace).base();
  if (start >= end)
    return ""; // all spaces
  return std::string(start, end);
}

std::string char_vec_to_string(std::vector<char> &line) {

  std::string temp_str = "";
  int letters = 0;
  for (const auto &ch : line) {
    if ((uint64_t)letters != MAX_TOP_WIDTH) {
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
