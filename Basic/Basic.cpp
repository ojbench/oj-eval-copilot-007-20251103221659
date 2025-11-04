/*
 * File: Basic.cpp
 * ---------------
 * This file is the starter project for the BASIC interpreter.
 */

#include <cctype>
#include <iostream>
#include <string>
#include "exp.hpp"
#include "parser.hpp"
#include "program.hpp"
#include "Utils/error.hpp"
#include "Utils/tokenScanner.hpp"
#include "Utils/strlib.hpp"


/* Function prototypes */

bool processLine(std::string line, Program &program, EvalState &state, bool &quit);
Statement* parseStatement(TokenScanner &scanner, std::string &firstToken);
void runProgram(Program &program, EvalState &state);

/* Main program */

int main() {
    EvalState state;
    Program program;
    bool quit = false;
    //cout << "Stub implementation of BASIC" << endl;
    while (!quit) {
        try {
            std::string input;
            if (!getline(std::cin, input)) {
                break;  // EOF
            }
            if (input.empty())
                continue;
            if (processLine(input, program, state, quit)) {
                break;  // QUIT command
            }
        } catch (ErrorException &ex) {
            std::cout << ex.getMessage() << std::endl;
        }
    }
    return 0;
}

/*
 * Function: processLine
 * Usage: processLine(line, program, state);
 * -----------------------------------------
 * Processes a single line entered by the user.  In this version of
 * implementation, the program reads a line, parses it as an expression,
 * and then prints the result.  In your implementation, you will
 * need to replace this method with one that can respond correctly
 * when the user enters a program line (which begins with a number)
 * or one of the BASIC commands, such as LIST or RUN.
 */

bool processLine(std::string line, Program &program, EvalState &state, bool &quit) {
    TokenScanner scanner;
    scanner.ignoreWhitespace();
    scanner.scanNumbers();
    scanner.setInput(line);

    if (!scanner.hasMoreTokens()) return false;
    
    std::string token = scanner.nextToken();
    
    // Check if it's a line number
    if (scanner.getTokenType(token) == NUMBER) {
        int lineNumber = stringToInteger(token);
        
        // Check if this is just a line number (deletion)
        if (!scanner.hasMoreTokens()) {
            program.removeSourceLine(lineNumber);
            return false;
        }
        
        // It's a program line - store it
        program.addSourceLine(lineNumber, line);
        
        // Parse the statement
        std::string firstToken;
        Statement *stmt = parseStatement(scanner, firstToken);
        if (stmt != nullptr) {
            program.setParsedStatement(lineNumber, stmt);
        }
    } else {
        // It's a direct command
        if (token == "RUN") {
            runProgram(program, state);
        } else if (token == "LIST") {
            int lineNum = program.getFirstLineNumber();
            while (lineNum != -1) {
                std::cout << program.getSourceLine(lineNum) << std::endl;
                lineNum = program.getNextLineNumber(lineNum);
            }
        } else if (token == "CLEAR") {
            program.clear();
            state.Clear();
        } else if (token == "QUIT") {
            quit = true;
            return true;
        } else if (token == "HELP") {
            // Optional - not tested
        } else {
            // It's a direct statement (PRINT, LET, INPUT)
            scanner.saveToken(token);
            std::string firstToken;
            Statement *stmt = parseStatement(scanner, firstToken);
            if (stmt != nullptr) {
                stmt->execute(state, program);
                delete stmt;
            }
        }
    }
    return false;
}

Statement* parseStatement(TokenScanner &scanner, std::string &firstToken) {
    firstToken = scanner.nextToken();
    
    if (firstToken == "REM") {
        return new RemStatement();
    } else if (firstToken == "LET") {
        std::string var = scanner.nextToken();
        // Check if variable is a keyword
        if (var == "REM" || var == "LET" || var == "PRINT" || var == "INPUT" || 
            var == "END" || var == "GOTO" || var == "IF" || var == "THEN" || 
            var == "RUN" || var == "LIST" || var == "CLEAR" || var == "QUIT" || var == "HELP") {
            error("SYNTAX ERROR");
        }
        std::string equals = scanner.nextToken();
        if (equals != "=") {
            error("SYNTAX ERROR");
        }
        Expression *exp = nullptr;
        try {
            exp = parseExp(scanner);
            return new LetStatement(var, exp);
        } catch (...) {
            delete exp;
            throw;
        }
    } else if (firstToken == "PRINT") {
        Expression *exp = nullptr;
        try {
            exp = parseExp(scanner);
            return new PrintStatement(exp);
        } catch (...) {
            delete exp;
            throw;
        }
    } else if (firstToken == "INPUT") {
        std::string var = scanner.nextToken();
        // Check if variable is a keyword
        if (var == "REM" || var == "LET" || var == "PRINT" || var == "INPUT" || 
            var == "END" || var == "GOTO" || var == "IF" || var == "THEN" || 
            var == "RUN" || var == "LIST" || var == "CLEAR" || var == "QUIT" || var == "HELP") {
            error("SYNTAX ERROR");
        }
        return new InputStatement(var);
    } else if (firstToken == "GOTO") {
        std::string lineStr = scanner.nextToken();
        int targetLine = stringToInteger(lineStr);
        return new GotoStatement(targetLine);
    } else if (firstToken == "IF") {
        // Parse left expression without consuming comparison operator
        Expression *lhs = nullptr;
        Expression *rhs = nullptr;
        try {
            lhs = readE(scanner, 0);
            std::string op = scanner.nextToken();
            // Parse right expression
            rhs = readE(scanner, 0);
            std::string thenStr = scanner.nextToken();
            if (thenStr != "THEN") {
                error("SYNTAX ERROR");
            }
            std::string lineStr = scanner.nextToken();
            int targetLine = stringToInteger(lineStr);
            return new IfStatement(lhs, op, rhs, targetLine);
        } catch (...) {
            delete lhs;
            delete rhs;
            throw;
        }
    } else if (firstToken == "END") {
        return new EndStatement();
    }
    
    return nullptr;
}

void runProgram(Program &program, EvalState &state) {
    state.setCurrentLine(program.getFirstLineNumber());
    
    while (state.getCurrentLine() != -1) {
        int lineNum = state.getCurrentLine();
        
        // Check if line exists
        if (program.getSourceLine(lineNum).empty()) {
            std::cout << "LINE NUMBER ERROR" << std::endl;
            return;
        }
        
        Statement *stmt = program.getParsedStatement(lineNum);
        if (stmt != nullptr) {
            // Set next line before execution (in case of GOTO/IF/END)
            int nextLine = program.getNextLineNumber(lineNum);
            state.setCurrentLine(nextLine);
            
            // Execute statement (may change currentLine)
            int savedLine = state.getCurrentLine();
            stmt->execute(state, program);
            
            // If statement didn't change the line, use the saved next line
            if (state.getCurrentLine() == savedLine) {
                // Line wasn't changed by statement
            } else {
                // Line was changed (by GOTO/IF/END)
            }
        } else {
            // Move to next line
            state.setCurrentLine(program.getNextLineNumber(lineNum));
        }
    }
}

