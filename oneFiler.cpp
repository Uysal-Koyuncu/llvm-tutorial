// oneFiler.cpp

#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace {
  static std::string IdentifierStr; // Filled in if TOK_IDENTIFIER
  static double NumVal;             // Filled in if TOK_NUMBER
}

enum LexerError {
  LEXERR_MALFORMED_NUMBER = 1,
};

const std::string DEBUG_MESSAGES[] = {
  "Malformed number",
};

int lexerDebugReturn(LexerError lexerError) {
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
        if (dotcount > 0) return lexerDebugReturn(LEXERR_MALFORMED_NUMBER);
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

/*
  AST Types
*/
// Expression AST
class ExprAST {
  public:
    virtual ~ExprAST() = default;     // virtual destructor
};

// Number AST
class NumberExprAST : public ExprAST {
  public:
    NumberExprAST(double val) : d_val(val) {}

  private:
    double d_val;
};

class VariableExprAST : public ExprAST {
  public:
    VariableExprAST(const std::string &name) : d_name(name) {}

  private:
    std::string d_name;
};

class BinaryExprAST : public ExprAST {
  public:
    // TODO: why is this a unique_ptr?
    BinaryExprAST(char op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs)
      : d_op(op), d_lhs(std::move(lhs)), d_rhs(std::move(rhs)) {}

  private:
    char d_op;
    std::unique_ptr<ExprAST> d_lhs, d_rhs;
};

class CallExprAST : public ExprAST {
  public:
    CallExprAST(const std::string &callee, std::vector<std::unique_ptr<ExprAST>> args)
      : d_callee(callee), d_args(std::move(args)) {}

  private:
    std::string d_callee;     // name of the function
    std::vector<std::unique_ptr<ExprAST>> d_args;
};

// Note how the following classes are not implementing ExprAST
class PrototypeAST {
  public:
    PrototypeAST(const std::string &name, std::vector<std::string> args)
      : d_name(name), d_args(std::move(args)) {}

  private:
    std::string d_name;
    std::vector<std::string> d_args;
};

class FunctionAST {
  public:
    FunctionAST(std::unique_ptr<PrototypeAST> proto, std::unique_ptr<ExprAST> body)
      : d_proto(std::move(proto)), d_body(std::move(body)) {}

  private:
    std::unique_ptr<PrototypeAST> d_proto;
    std::unique_ptr<ExprAST> d_body;
};

std::unique_ptr<ExprAST> logError(const char *str) {
  std::cerr << "Error: " << str << std::endl;
  return nullptr;
}

std::unique_ptr<PrototypeAST> logErrorPrototype(const char *str) {
  logError(str);
  return nullptr;
}

static int CurTok;
static int getNextToken() {
  CurTok = gettok();    // get the next token from the lexer
  return CurTok;
}

// TODO: better understand this!!
// Number: <TOK_NUMBER>
static std::unique_ptr<ExprAST> parseNumberExpr() {
  auto result = std::make_unique<NumberExprAST>(NumVal);  // consume the number
  getNextToken();
  return std::move(result);
}

// Parentheses: '(' <expression> ')'
static std::unique_ptr<ExprAST> parseParenExpr() {
  getNextToken();  // consume '('
  auto v = parseExpression();
  if (!v) return nullptr;

}


// dummy main
int main() {
  return 0;
}