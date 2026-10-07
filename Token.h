// Token.h - the vocabulary of the HL language.
#pragma once
#include <string>

enum class TokenType {
    // literals / names
    Identifier, IntLiteral, DoubleLiteral, StringLiteral,
    // reserved words   (keep these four contiguous: see isReservedWord)
    KwInteger, KwDouble, KwIf, KwOutput,
    // symbols          (keep these contiguous: see isSymbol)
    Colon,      // :
    Assign,     // :=
    Equals,     // =
    Semicolon,  // ;
    ShiftOut,   // <<
    Plus,       // +
    Minus,      // -
    LParen,     // (
    RParen,     // )
    Less,       // <
    Greater,    // >
    EqEq,       // ==
    NotEq,      // !=
    // bookkeeping
    EndOfFile, Invalid
};

struct Token {
    TokenType type;
    std::string lexeme;  // for StringLiteral: the text WITHOUT the quotes
    int line;
    int column;
};

inline bool isReservedWord(TokenType t) {
    return t >= TokenType::KwInteger && t <= TokenType::KwOutput;
}
inline bool isSymbol(TokenType t) {
    return t >= TokenType::Colon && t <= TokenType::NotEq;
}
