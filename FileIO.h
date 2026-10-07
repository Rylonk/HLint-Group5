// FileIO.h - reading the source file and producing the two report files.
#pragma once
#include <string>
#include <vector>
#include "Token.h"

// Reads a whole file into 'out'. Returns false if it cannot be opened.
bool readFile(const std::string& path, std::string& out);

// Writes 'content' to 'path'. Returns false on failure.
bool writeFile(const std::string& path, const std::string& content);

// The program text with all blanks removed (content of NOSPACES.TXT).
std::string removeSpaces(const std::string& source);

// A table of every reserved word and symbol in the program, in order of
// appearance (content of RES_SYM.TXT).
std::string buildReservedSymbolReport(const std::vector<Token>& tokens);
