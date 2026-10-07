#include "Lexer.h"
#include <cctype>
#include "Config.h"

namespace {
std::string toLower(std::string s) {
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}
bool isWordStart(char c) { return std::isalpha(static_cast<unsigned char>(c)) || c == '_'; }
bool isWordChar(char c)  { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; }
bool isDigit(char c)     { return c >= '0' && c <= '9'; }
}  // namespace

char Lexer::advance() {
    char c = src_[pos_++];
    if (c == '\n') { ++line_; col_ = 1; }
    else           { ++col_; }
    return c;
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (!atEnd()) {
        char c = peek();
        if (std::isspace(static_cast<unsigned char>(c))) { advance(); continue; }

        int line = line_, col = col_;
        if (isWordStart(c))      tokens.push_back(lexWord(line, col));
        else if (isDigit(c))     tokens.push_back(lexNumber(line, col));
        else if (c == '"')       tokens.push_back(lexString(line, col));
        else                     tokens.push_back(lexSymbol(line, col));
    }
    tokens.push_back({TokenType::EndOfFile, "", line_, col_});
    return tokens;
}

Token Lexer::lexWord(int line, int col) {
    std::string word;
    while (!atEnd() && isWordChar(peek())) word += advance();

    const std::string key = config::CASE_INSENSITIVE_KEYWORDS ? toLower(word) : word;
    TokenType type = TokenType::Identifier;
    if      (key == "integer") type = TokenType::KwInteger;
    else if (key == "double")  type = TokenType::KwDouble;
    else if (key == "if")      type = TokenType::KwIf;
    else if (key == "output")  type = TokenType::KwOutput;
    return {type, word, line, col};
}

Token Lexer::lexNumber(int line, int col) {
    std::string text;
    while (isDigit(peek())) text += advance();

    if (peek() == '.') {
        if (isDigit(peek(1))) {
            text += advance();  // the '.'
            while (isDigit(peek())) text += advance();
            return {TokenType::DoubleLiteral, text, line, col};
        }
        text += advance();
        error(line, col, "malformed number '" + text + "' (digits expected after the decimal point)");
        return {TokenType::Invalid, text, line, col};
    }
    return {TokenType::IntLiteral, text, line, col};
}

Token Lexer::lexString(int line, int col) {
    advance();  // opening quote
    std::string text;
    while (!atEnd() && peek() != '"' && peek() != '\n') text += advance();

    if (peek() == '"') {
        advance();  // closing quote
        return {TokenType::StringLiteral, text, line, col};
    }
    error(line, col, "unterminated string literal (missing closing \")");
    return {TokenType::Invalid, "\"" + text, line, col};
}

Token Lexer::lexSymbol(int line, int col) {
    char c = advance();
    switch (c) {
        case ':':
            if (peek() == '=') { advance(); return {TokenType::Assign, ":=", line, col}; }
            return {TokenType::Colon, ":", line, col};
        case '=':
            if (peek() == '=') { advance(); return {TokenType::EqEq, "==", line, col}; }
            return {TokenType::Equals, "=", line, col};
        case '!':
            if (peek() == '=') { advance(); return {TokenType::NotEq, "!=", line, col}; }
            error(line, col, "unexpected '!' (did you mean '!='?)");
            return {TokenType::Invalid, "!", line, col};
        case '<':
            if (peek() == '<') { advance(); return {TokenType::ShiftOut, "<<", line, col}; }
            return {TokenType::Less, "<", line, col};
        case '>': return {TokenType::Greater,   ">", line, col};
        case ';': return {TokenType::Semicolon, ";", line, col};
        case '+': return {TokenType::Plus,      "+", line, col};
        case '-': return {TokenType::Minus,     "-", line, col};
        case '(': return {TokenType::LParen,    "(", line, col};
        case ')': return {TokenType::RParen,    ")", line, col};
        default: break;
    }
    error(line, col, std::string("unexpected character '") + c + "'");
    return {TokenType::Invalid, std::string(1, c), line, col};
}
