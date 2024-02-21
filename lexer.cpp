//  lexer.cpp - Lexer for the Kaleidoscope language

#include "lexer.h"

#include <iostream>
#include <string>

namespace {
  static std::string IdentifierStr; // Filled in if TOK_IDENTIFIER
  static double NumVal;             // Filled in if TOK_NUMBER

  enum LexerError {
    LEXERR_MALFORMED_NUMBER = 1,

  };

  const std::string DEBUG_MESSAGES[] = {
    "Malformed number",
  };

  int debugReturn(LexerError lexerError) {
    std::cerr << "Lexer error: " << DEBUG_MESSAGES[lexerError - 1] << std::endl;
    return lexerError;
  }
} // namespace


namespace lexer {

int gettok() {
  static int LastChar = ' ';      // !STATIC! always the next unparsed character.

  // Whitespace: [ \t\n]*
  while (isspace(LastChar)) LastChar = getchar();

  // Identifier: [a-zA-Z][a-zA-Z0-9]*
  if (isalpha(LastChar)) {
    for (IdentifierStr = LastChar; isalnum(LastChar); LastChar = getchar()) {
      IdentifierStr += LastChar;
    }

    if (IdentifierStr == "def") return TOK_DEF;
    if (IdentifierStr == "extern") return TOK_EXTERN;
    return TOK_IDENTIFIER;
  }

  // Number: [0-9]*\.[0-9]+
  if (isdigit(LastChar) || LastChar == '.' ) {    // .7 is a valid float
    int dotcount = LastChar == '.' ? 1 : 0;
    std::string NumStr;
    for (NumStr = LastChar; isdigit(LastChar) || LastChar == '.'; LastChar = getchar()) {
      if (LastChar == '.') {
        if (dotcount > 0) return debugReturn(LEXERR_MALFORMED_NUMBER);
        dotcount++;
      }
      NumStr += LastChar;
    }
    NumVal = strtod(NumStr.c_str(), 0);
    return TOK_NUMBER;
  }

  // Comment: #.*
  if (LastChar == '#') {
    for (LastChar = getchar(); LastChar != EOF && LastChar != '\n' && LastChar != '\r'; LastChar = getchar());      // Skip until end of line or EOF
    
    if (LastChar != EOF) return gettok();
  }

  // EOF: regex \Z
  if (LastChar == EOF) return TOK_EOF;

  // Return ASCII otherwise for semantic parsing
  int ThisChar = LastChar;
  LastChar = getchar();
  return ThisChar;
}
  
} // namespace lexer