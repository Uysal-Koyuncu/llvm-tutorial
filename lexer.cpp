#include <iostream>
#include <string>

// Lexer returns 0-255 if unknown character (i.e. returns ASCII), otherwise negative values are meaningful

enum LexerError {
  LEXERR_MALFORMED_NUMBER = 1,

};

static std::string DEBUG_MESSAGES[] = {
  "Malformed number",
};

static int debugReturn(LexerError lexerError) {
  std::cerr << "Lexer error: " << DEBUG_MESSAGES[lexerError - 1] << std::endl;
  return lexerError;
}

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

static std::string IdentifierStr; // Filled in if TOK_IDENTIFIER
static double NumVal;             // Filled in if TOK_NUMBER

static int gettok() {
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
}