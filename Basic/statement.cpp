/*
 * File: statement.cpp
 * -------------------
 * This file implements the constructor and destructor for
 * the Statement class itself.  Your implementation must do
 * the same for the subclasses you define for each of the
 * BASIC statements.
 */

#include "statement.hpp"

int stringToInt(std::string str);

Statement::Statement() = default;

Statement::~Statement() = default;

// REM Statement
RemStatement::RemStatement() {}
RemStatement::~RemStatement() {}
void RemStatement::execute(EvalState &state, Program &program) {
    // Do nothing - comments are ignored
}

// LET Statement
LetStatement::LetStatement(std::string var, Expression *exp) : variable(var), expr(exp) {}
LetStatement::~LetStatement() {
    delete expr;
}
void LetStatement::execute(EvalState &state, Program &program) {
    state.setValue(variable, expr->eval(state));
}

// PRINT Statement
PrintStatement::PrintStatement(Expression *exp) : expr(exp) {}
PrintStatement::~PrintStatement() {
    delete expr;
}
void PrintStatement::execute(EvalState &state, Program &program) {
    std::cout << expr->eval(state) << std::endl;
}

// INPUT Statement
InputStatement::InputStatement(std::string var) : variable(var) {}
InputStatement::~InputStatement() {}
void InputStatement::execute(EvalState &state, Program &program) {
    std::cout << " ? ";
    std::string input;
    std::getline(std::cin, input);
    try {
        int value = stringToInteger(input);
        state.setValue(variable, value);
    } catch (...) {
        std::cout << "INVALID NUMBER" << std::endl;
        execute(state, program);  // Re-prompt
    }
}

// GOTO Statement
GotoStatement::GotoStatement(int line) : targetLine(line) {}
GotoStatement::~GotoStatement() {}
void GotoStatement::execute(EvalState &state, Program &program) {
    state.setCurrentLine(targetLine);
}

// IF Statement
IfStatement::IfStatement(Expression *lhs, std::string oper, Expression *rhs, int line) 
    : left(lhs), op(oper), right(rhs), targetLine(line) {}
IfStatement::~IfStatement() {
    delete left;
    delete right;
}
void IfStatement::execute(EvalState &state, Program &program) {
    int leftVal = left->eval(state);
    int rightVal = right->eval(state);
    bool condition = false;
    
    if (op == "=") condition = (leftVal == rightVal);
    else if (op == "<") condition = (leftVal < rightVal);
    else if (op == ">") condition = (leftVal > rightVal);
    
    if (condition) {
        state.setCurrentLine(targetLine);
    }
}

// END Statement
EndStatement::EndStatement() {}
EndStatement::~EndStatement() {}
void EndStatement::execute(EvalState &state, Program &program) {
    state.setCurrentLine(-1);  // Signal to stop execution
}
