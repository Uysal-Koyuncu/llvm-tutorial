#ifndef DEFINED_PARSER_H
#define DEFINED_PARSER_H

#include <memory>
#include <string>
#include <vector>

namespace parser {

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
  

  // Functions
  int getNextToken();
}

#endif