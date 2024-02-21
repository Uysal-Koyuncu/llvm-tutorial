#ifndef DEFINED_LEXER_H
#define DEFINED_LEXER_H

//  lexer.h - Lexer for the Kaleidoscope language
#include <iostream>
#include <string>

// Lexer returns 0-255 if unknown character (i.e. returns ASCII), otherwise negative values are meaningful
enum Token {
  TOK_EOF = -1,

  // commands
  TOK_DEF = -2,
  TOK_EXTERN = -3,

  // primary
  TOK_IDENTIFIER = -4,
  TOK_NUMBER = -5,
  TOK_LEXERR = -6,
};

namespace lexer {
  int gettok();    // returns the next token from standard input
}

#endif