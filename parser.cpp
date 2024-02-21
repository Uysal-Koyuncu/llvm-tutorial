//  parser.cpp - Parser for the Kaleidoscope language

#include "lexer.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

// TODO: potentially separate into another file
static int CurTok;

namespace parser {

int getNextToken() {
  CurTok = lexer::gettok();    // get the next token from the lexer
  return CurTok;
}

} // namespace parser