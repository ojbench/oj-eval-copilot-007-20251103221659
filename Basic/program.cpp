/*
 * File: program.cpp
 * -----------------
 * This file is a stub implementation of the program.h interface
 * in which none of the methods do anything beyond returning a
 * value of the correct type.  Your job is to fill in the bodies
 * of each of these methods with an implementation that satisfies
 * the performance guarantees specified in the assignment.
 */

#include "program.hpp"

Program::Program() = default;

Program::~Program() {
    clear();
}

void Program::clear() {
    for (auto &pair : statements) {
        delete pair.second;
    }
    statements.clear();
    sourceLines.clear();
}

void Program::addSourceLine(int lineNumber, const std::string &line) {
    if (statements.find(lineNumber) != statements.end()) {
        delete statements[lineNumber];
        statements.erase(lineNumber);
    }
    sourceLines[lineNumber] = line;
}

void Program::removeSourceLine(int lineNumber) {
    if (statements.find(lineNumber) != statements.end()) {
        delete statements[lineNumber];
        statements.erase(lineNumber);
    }
    sourceLines.erase(lineNumber);
}

std::string Program::getSourceLine(int lineNumber) {
    if (sourceLines.find(lineNumber) != sourceLines.end()) {
        return sourceLines[lineNumber];
    }
    return "";
}

void Program::setParsedStatement(int lineNumber, Statement *stmt) {
    if (statements.find(lineNumber) != statements.end()) {
        delete statements[lineNumber];
    }
    statements[lineNumber] = stmt;
}

Statement *Program::getParsedStatement(int lineNumber) {
    if (statements.find(lineNumber) != statements.end()) {
        return statements[lineNumber];
    }
    return nullptr;
}

int Program::getFirstLineNumber() {
    if (sourceLines.empty()) {
        return -1;
    }
    return sourceLines.begin()->first;
}

int Program::getNextLineNumber(int lineNumber) {
    auto it = sourceLines.find(lineNumber);
    if (it != sourceLines.end()) {
        ++it;
        if (it != sourceLines.end()) {
            return it->first;
        }
    }
    return -1;
}


