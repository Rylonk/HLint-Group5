// Parser.h - recursive-descent syntax checker + AST builder.
//
// Grammar of HL (the whole language):
//
//   program    := statement*
//   statement  := declaration | assignment | output | ifStmt
//   declaration:= IDENT ':' ('integer' | 'double') ';'
//   assignment := IDENT (':=' | '=') expr ';'
//   output     := 'output' '<<' (STRING | expr) ';'
//   ifStmt     := 'if' '(' expr relop expr ')' statement      (no declarations)
//   relop      := '>' | '<' | '==' | '!='
//   expr       := term (('+' | '-') term)*
//   term       := INT | DOUBLE | IDENT
//
// Besides pure syntax, the parser also checks the simple "semantic" rules a
// student compiler normally needs: variables must be declared (once) before
// use, and a double cannot be stored in an integer variable.
//
// Errors do not stop the parse: after each error the parser skips ahead to the
// next statement so that ALL mistakes in the file are reported in one run.
#pragma once
#include <map>
#include <string>
#include <utility>
#include <vector>
#include "Ast.h"
#include "Diagnostic.h"
#include "Token.h"

class Parser {
public:
    explicit Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

    Program parse();
    const std::vector<Diagnostic>& diagnostics() const { return diags_; }

private:
    struct ParseError {
        Diagnostic diag;
        bool silent;  // already reported by the lexer -> don't print twice
        bool skip;    // true: resynchronise by skipping to the next statement
    };

    // token cursor helpers
    const Token& peek(size_t ahead = 0) const;
    const Token& previous() const;
    bool check(TokenType t) const { return peek().type == t; }
    bool isAtEnd() const { return peek().type == TokenType::EndOfFile; }
    const Token& advance();
    const Token& expect(TokenType t, const std::string& message);
    void expectSemicolon();
    [[noreturn]] void fail(const Token& at, const std::string& message, bool skip = true) const;
    static std::string describe(const Token& t);
    void synchronize(size_t startPos, int errorLine);

    // grammar rules
    StmtPtr parseStatement(bool allowDeclaration);
    StmtPtr parseDeclaration();
    StmtPtr parseAssignment();
    StmtPtr parseOutput();
    StmtPtr parseIf();
    ExprPtr parseExpression();
    ExprPtr parseTerm();

    std::vector<Token> tokens_;
    size_t pos_ = 0;
    std::vector<Diagnostic> diags_;
    std::map<std::string, DataType> symbols_;  // declared variables
};
