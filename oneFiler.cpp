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

/*
  Unit Parsing Functions:
    These functions will be called by a casing during iterating over the tokens
*/
// Number: <TOK_NUMBER>
static std::unique_ptr<ExprAST> parseNumberExpr() {
  auto result = std::make_unique<NumberExprAST>(NumVal);  // consume the number
  getNextToken();
  return std::move(result);
}

// Parentheses: '(' <expression> ')'
// makes sure the parentheses are respected first
static std::unique_ptr<ExprAST> parseParenExpr() {
  getNextToken();  // consume '('
  auto v = parseExpression();   // possible recursion!
  if (!v) return nullptr;

  if (CurTok != ')')
    return logError("expected ')'");
  getNextToken();  // consume ')'
  return v;
}

// Identifier: <TOK_IDENTIFIER> | <TOK_IDENTIFIER> '(' <expression> ')'
// first case: just an identifier
// second case: a function call with arguments, reduced into a variable reference? (TODO: why not a value?)
static std::unique_ptr<ExprAST> parseIdentifierExpr() {
  std::string idName = IdentifierStr;

  getNextToken();  // consume identifier

  // (CurTok is similar to 'peek'ing into the lexer)
  if (CurTok != '(') // simple variable reference
    return std::make_unique<VariableExprAST>(idName);
  
  // function call (?)
  getNextToken();  // consume '('
  std::vector<std::unique_ptr<ExprAST>> args;
  if (CurTok != ')') {
    while (true) {
      if (auto arg = ParseExpression()) {
        args.push_back(std::move(arg)); // consume the argument
      }
      else {
        return nullptr;
      }
      // Note this is not casing, but a check for the next token (parseExpression() will progress along the tokens)
      if (CurTok == ')')
        break;
      if (CurTok != ',')
        return logError("Expected ')' or ',' in argument list");
      getNextToken();  // consume ','; move to next argument in function call
    }

    getNextToken();  // consume ')'
    return std::make_unique<CallExprAST>(idName, std::move(args));
  }
}

// Helper caser to unify the Unit Parsing Functions
// The entire LK(1) grammar lies within CurTok peeking.
static std::unique_ptr<ExprAST> parsePrimary() {
  switch (CurTok) {
    case TOK_IDENTIFIER:
      return parseIdentifierExpr();
    case TOK_NUMBER:
      return parseNumberExpr();
    case '(':
      return parseParenExpr();
    default:
      return logError("unknown token when expecting an expression");
  }
}

// Binop Precedence
static std::map<char, int> BinopPrecedence;

static int getTokPrecedence() {
  if (!isascii(CurTok))
    return -1;
  
  if (BinopPrecedence.find(CurTok) == BinopPrecedence.end())
    return -1;

  return BinopPrecedence[CurTok];
}

// Binary Expression Parsing: see 
// expression ::= primary [binoprhs]
/*
  comment: "Consider, for example, the expression “a+b+(c+d)*e*f+g”. Operator precedence parsing considers this as a stream of primary expressions separated by binary operators. As such, it will first parse the leading primary expression “a”, then it will see the pairs [+, b] [+, (c+d)] [*, e] [*, f] and [+, g]. Note that because parentheses are primary expressions, the binary expression parser doesn’t need to worry about nested subexpressions like (c+d) at all."
*/
// "parentheses are primary expressions".
static std::unique_ptr<ExprAST> parseExpression() {
  auto lhs = parsePrimary();
  if (!lhs)
    return nullptr;
  
  return parseBinOpRHS(0, std::move(lhs));
}

// TODO: compile a better BNF for entire code
// binoprhs ::= (<binop> primary)*
// TODO: I feel this one is a bit different from the others
static std::unique_ptr<ExprAST> parseBinOpRHS(int exprPrec, std::unique_ptr<ExprAST> lhs) {
  while (true) {
    int tokPrec = getTokPrecedence();

    // We have to only parse if the precedence of the next operator is higher than the current one. For example: a+b*c, we have to parse b*c first, then a+(b*c)
    if (tokPrec < exprPrec)
      return lhs;
    
    // knowing binop
    int binOp = CurTok;
    getNextToken();

    // parse primary expression trailing the binop. for example, in a+b*c, we parse b*c
    // without precedence, respect right-to-left associativity
    auto rhs = parsePrimary();
    if (!rhs)
      return nullptr;
    
    int nextPrec = getTokPrecedence();    // the token was updated in parsePrimary()
    if (tokProc < nextProc) {
      rhs = parseBinOpRHS(TokPrec + 1, std::move(rhs)); // why 1? we are, like, recording the current level of precedence. Quite like a stack. Like a curr_high. The next time this falls below the TokPrec + 1, we know the higher-precedence expression has been fully parsed.
      if (!rhs)
        return nullptr;
    }
    // precedence breaks associativity. for example a*b+c -> (a*b)+c
    lhs = std::make_unique<BinaryExprAST>(binOp, std::move(lhs), std::move(rhs));
  }
}

// Till 2.5 end; next: 2.6 - parsing prototypes, defs, externs.

// dummy main
int main() {
  // define the precedence of the binary operators
  BinopPrecedence['<'] = 10;
  BinopPrecedence['+'] = 20;
  BinopPrecedence['-'] = 20;
  BinopPrecedence['*'] = 40;
  BinopPrecedence['/'] = 40;
}