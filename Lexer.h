// Lexer.h - turns HL source text into a list of tokens.
#pragma once
#include <string>
#include <utility>
#include <vector>
#include "Diagnostic.h"
#include "Token.h"

class Lexer {
public:
    explicit Lexer(std::string source) : src_(std::move(source)) {}

    // Always returns a token list that ends with EndOfFile.  Bad characters
    // become Invalid tokens (and a diagnostic) so the parser can keep going.
    std::vector<Token> tokenize();
    const std::vector<Diagnostic>& diagnostics() const { return diags_; }

private:
    bool atEnd() const { return pos_ >= src_.size(); }
    char peek(size_t ahead = 0) const {
        return pos_ + ahead < src_.size() ? src_[pos_ + ahead] : '\0';
    }
    char advance();
    void error(int line, int col, const std::string& msg) {
        diags_.push_back({line, col, msg});
    }

    Token lexWord(int line, int col);
    Token lexNumber(int line, int col);
    Token lexString(int line, int col);
    Token lexSymbol(int line, int col);

    std::string src_;
    size_t pos_ = 0;
    int line_ = 1;
    int col_ = 1;
    std::vector<Diagnostic> diags_;
};
