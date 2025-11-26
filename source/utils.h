#ifndef UTILS
#define UTILS

#include <algorithm>
#include <cctype>
#include <stdint.h>
#include <string>
#include <vector>

std::string strip(std::string s);
std::string char_vec_to_string(std::vector<char> &line);

bool is_wrapped_in_parens(const std::string &s);
bool contains_whitespace(const std::string &s);
std::string wrap(const std::string &s);

#endif /* UTILS */
