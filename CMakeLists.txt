cmake_minimum_required(VERSION 3.22)

project(llvm_frontend)

# add_executable(llvm_frontend
#   lexer.cpp
#   lexer.h
#   main.cpp
#   parser.cpp
#   parser.h
# )
add_executable(llvm_frontend_onefiler
  oneFiler.cpp
)