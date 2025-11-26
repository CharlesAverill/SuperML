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
  uint64_t letters = 0;
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

bool is_wrapped_in_parens(const std::string &s) {
  if (s.size() < 2)
    return false;

  if (s.front() != '(' || s.back() != ')')
    return false;

  // Check paren *balance* to ensure the outer parens match
  int depth = 0;
  for (size_t i = 0; i < s.size(); ++i) {
    if (s[i] == '(')
      depth++;
    else if (s[i] == ')')
      depth--;

    // If outer paren closes before the end → not fully wrapped
    if (depth == 0 && i != s.size() - 1)
      return false;
  }
  return depth == 0;
}

bool contains_whitespace(const std::string &s) {
  for (char c : s) {
    if (std::isspace(static_cast<unsigned char>(c)))
      return true;
  }
  return false;
}

std::string wrap(const std::string &s) {
  if (contains_whitespace(s) && !is_wrapped_in_parens(s)) {
    return "(" + s + ")";
  }
  return s;
}
