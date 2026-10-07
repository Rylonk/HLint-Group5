// HLInt - a simple interpreter for the hypothetical language "HL".
//
// Pipeline (matches "The Process" in the project specification):
//   1. open the source file (PROG1.HL, PROG2.HL, ...)
//   2. remove all spaces            -> NOSPACES.TXT
//   3. find reserved words/symbols  -> RES_SYM.TXT
//   4. check the syntax             -> prints "ERROR" or "NO ERROR(S) FOUND"
//   5. if there were no errors, run the program
//
// Usage:   HLInt PROG1.HL        (or run with no argument and type the name)
//
// Exit code: 0 = ok, 1 = syntax error(s) found, 2 = file problem.
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "Config.h"
#include "FileIO.h"
#include "Interpreter.h"
#include "Lexer.h"
#include "Parser.h"

namespace {

std::string trim(const std::string& s) {
    const char* ws = " \t\r\n\"";  // quotes too: Windows drag-and-drop adds them
    const size_t b = s.find_first_not_of(ws);
    if (b == std::string::npos) return "";
    return s.substr(b, s.find_last_not_of(ws) - b + 1);
}

// Details go to stderr so that stdout carries exactly what the spec asks for.
void printDiagnostics(std::vector<Diagnostic> diags) {
    std::stable_sort(diags.begin(), diags.end(), [](const Diagnostic& a, const Diagnostic& b) {
        return a.line != b.line ? a.line < b.line : a.column < b.column;
    });
    for (const Diagnostic& d : diags)
        std::cerr << "  line " << d.line << ", col " << d.column << ": " << d.message << '\n';
}

}  // namespace

int main(int argc, char** argv) {
    std::string path;
    if (argc >= 2) {
        path = argv[1];
    } else {
        std::cout << "Enter the HL source file (e.g. PROG1.HL): ";
        std::getline(std::cin, path);
    }
    path = trim(path);

    // 1. open the source file
    std::string source;
    if (!readFile(path, source)) {
        std::cerr << "Cannot open source file '" << path << "'\n";
        return 2;
    }

    // 2. NOSPACES.TXT
    if (!writeFile(config::NOSPACES_FILE, removeSpaces(source)))
        std::cerr << "Warning: could not write " << config::NOSPACES_FILE << '\n';

    // 3. RES_SYM.TXT
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();
    if (!writeFile(config::RES_SYM_FILE, buildReservedSymbolReport(tokens)))
        std::cerr << "Warning: could not write " << config::RES_SYM_FILE << '\n';

    // 4. syntax check
    Parser parser(tokens);
    Program program = parser.parse();

    std::vector<Diagnostic> errors = lexer.diagnostics();
    errors.insert(errors.end(), parser.diagnostics().begin(), parser.diagnostics().end());

    if (!errors.empty()) {
        std::cout << "ERROR" << std::endl;
        printDiagnostics(errors);
        return 1;
    }
    std::cout << "NO ERROR(S) FOUND" << std::endl;

    // 5. run it
    Interpreter interpreter(std::cout);
    interpreter.run(program);
    return 0;
}
