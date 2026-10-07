// Diagnostic.h - one error message tied to a place in the source file.
#pragma once
#include <string>

struct Diagnostic {
    int line;
    int column;
    std::string message;
};
