#include "../Notepad3DS/source/file_io.h"
#include "../globals.h"
#include "interpreter.h"
#include <iostream>
#include <string>

void stepCallback(State state) {}

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: devel <filename>\n";
    return 1;
  }

  std::string filename = argv[1];
  interpreterMain(filename);

  return 0;
}
