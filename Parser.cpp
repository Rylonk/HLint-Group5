#include "Parser.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include "Config.h"

// ------------------------------------------------------------ cursor helpers
const Token& Parser::peek(size_t ahead) const {
    return tokens_[std::min(pos_ + ahead, tokens_.size() - 1)];
}
const Token& Parser::previous() const { return tokens_[pos_ > 0 ? pos_ - 1 : 0]; }

const Token& Parser::advance() {
    const Token& t = tokens_[pos_];
    if (pos_ < tokens_.size() - 1) ++pos_;
    return t;
}

std::string Parser::describe(const Token& t) {
    switch (t.type) {
        case TokenType::EndOfFile:     return "end of file";
        case TokenType::StringLiteral: return "string \"" + t.lexeme + "\"";
        default:                       return "'" + t.lexeme + "'";
    }
}

void Parser::fail(const Token& at, const std::string& message, bool skip) const {
    // Invalid tokens were already reported by the lexer, so stay silent.
    throw ParseError{{at.line, at.column, message}, at.type == TokenType::Invalid, skip};
}

const Token& Parser::expect(TokenType t, const std::string& message) {
    if (check(t)) return advance();
    fail(peek(), message + ", found " + describe(peek()));
}

void Parser::expectSemicolon() {
    if (check(TokenType::Semicolon)) { advance(); return; }

    const Token& cur = peek();
    const bool startsStatement =
        cur.type == TokenType::Identifier || cur.type == TokenType::KwIf ||
        cur.type == TokenType::KwOutput   || cur.type == TokenType::KwInteger ||
        cur.type == TokenType::KwDouble   || cur.type == TokenType::EndOfFile;

    if (!startsStatement) fail(cur, "expected ';' but found " + describe(cur));

    // Most likely the semicolon was simply forgotten: blame the END of the
    // previous token and do not skip anything, so the next statement is
    // still checked.
    const Token& prev = previous();
    size_t len = prev.lexeme.size() + (prev.type == TokenType::StringLiteral ? 2 : 0);
    throw ParseError{{prev.line, prev.column + static_cast<int>(len),
                      "missing ';' at the end of the statement"},
                     false, false};
}

// After an error, skip to just past the next ';' (or stop in front of a token
// that obviously starts a new statement on a later line).
void Parser::synchronize(size_t startPos, int errorLine) {
    while (!isAtEnd()) {
        if (check(TokenType::Semicolon)) { advance(); break; }
        const Token& t = peek();
        const bool startsStatement =
            t.type == TokenType::Identifier || t.type == TokenType::KwIf ||
            t.type == TokenType::KwOutput   || t.type == TokenType::KwInteger ||
            t.type == TokenType::KwDouble;
        if (pos_ > startPos && t.line > errorLine && startsStatement) break;
        advance();
    }
    if (pos_ == startPos && !isAtEnd()) advance();  // always make progress
}

// -------------------------------------------------------------------- program
Program Parser::parse() {
    Program program;
    while (!isAtEnd()) {
        const size_t start = pos_;
        try {
            program.push_back(parseStatement(/*allowDeclaration=*/true));
        } catch (const ParseError& e) {
            if (!e.silent) diags_.push_back(e.diag);
            if (e.skip) synchronize(start, e.diag.line);
            else if (pos_ == start) advance();
        }
    }
    return program;
}

// ----------------------------------------------------------------- statements
StmtPtr Parser::parseStatement(bool allowDeclaration) {
    const Token& t = peek();
    switch (t.type) {
        case TokenType::KwOutput: return parseOutput();
        case TokenType::KwIf:     return parseIf();

        case TokenType::KwInteger:
        case TokenType::KwDouble:
            fail(t, "a declaration must be written  name:type;  (for example  x:integer;)");

        case TokenType::Identifier: {
            const Token& next = peek(1);
            if (next.type == TokenType::Colon) {
                if (!allowDeclaration)
                    fail(t, "a variable declaration is not allowed inside an if statement");
                return parseDeclaration();
            }
            if (next.type == TokenType::Assign) return parseAssignment();
            if (next.type == TokenType::Equals) {
                if (!config::ALLOW_PLAIN_EQUALS)
                    fail(next, "use ':=' for assignment");
                return parseAssignment();
            }
            fail(next, "expected ':' (declaration) or ':=' (assignment) after '" + t.lexeme +
                           "', found " + describe(next));
        }
        default:
            fail(t, "unexpected " + describe(t) + " at the start of a statement");
    }
}

StmtPtr Parser::parseDeclaration() {
    const Token& name = advance();  // identifier
    advance();                      // ':'

    const Token& typeTok = peek();
    DataType type;
    if      (typeTok.type == TokenType::KwInteger) type = DataType::Integer;
    else if (typeTok.type == TokenType::KwDouble)  type = DataType::Double;
    else fail(typeTok, "unknown data type " + describe(typeTok) + " (expected integer or double)");
    advance();

    if (symbols_.count(name.lexeme))
        fail(name, "variable '" + name.lexeme + "' is already declared");
    symbols_[name.lexeme] = type;  // register now so later uses don't cascade errors

    expectSemicolon();
    return std::make_unique<DeclareStmt>(name.lexeme, type, name.line);
}

StmtPtr Parser::parseAssignment() {
    const Token& name = advance();  // identifier
    advance();                      // ':=' or '='

    auto it = symbols_.find(name.lexeme);
    if (it == symbols_.end())
        fail(name, "variable '" + name.lexeme + "' is used before it is declared");

    ExprPtr value = parseExpression();
    if (it->second == DataType::Integer && value->type == DataType::Double)
        fail(name, "cannot assign a double value to the integer variable '" + name.lexeme + "'");

    expectSemicolon();
    return std::make_unique<AssignStmt>(name.lexeme, std::move(value), name.line);
}

StmtPtr Parser::parseOutput() {
    const Token& kw = advance();  // 'output'
    expect(TokenType::ShiftOut, "expected '<<' after 'output'");

    StmtPtr stmt;
    if (check(TokenType::StringLiteral)) {
        stmt = std::make_unique<OutputStmt>(advance().lexeme, kw.line);
    } else if (check(TokenType::Identifier) || check(TokenType::IntLiteral) ||
               check(TokenType::DoubleLiteral)) {
        stmt = std::make_unique<OutputStmt>(parseExpression(), kw.line);
    } else {
        fail(peek(), "expected a string or an expression after '<<', found " + describe(peek()));
    }
    expectSemicolon();
    return stmt;
}

StmtPtr Parser::parseIf() {
    const Token& kw = advance();  // 'if'
    expect(TokenType::LParen, "expected '(' after 'if'");

    ExprPtr lhs = parseExpression();

    RelOp op;
    switch (peek().type) {
        case TokenType::Less:    op = RelOp::Less;     break;
        case TokenType::Greater: op = RelOp::Greater;  break;
        case TokenType::EqEq:    op = RelOp::Equal;    break;
        case TokenType::NotEq:   op = RelOp::NotEqual; break;
        default:
            fail(peek(), "expected a comparison operator (>, <, == or !=), found " +
                             describe(peek()));
    }
    advance();

    ExprPtr rhs = parseExpression();
    expect(TokenType::RParen, "expected ')' to close the condition");

    if (isAtEnd()) fail(peek(), "the if statement has no statement to execute");
    StmtPtr body = parseStatement(/*allowDeclaration=*/false);

    return std::make_unique<IfStmt>(std::move(lhs), op, std::move(rhs), std::move(body), kw.line);
}

// ---------------------------------------------------------------- expressions
ExprPtr Parser::parseExpression() {
    ExprPtr left = parseTerm();
    while (check(TokenType::Plus) || check(TokenType::Minus)) {
        const Token& opTok = advance();
        const char op = opTok.type == TokenType::Plus ? '+' : '-';
        ExprPtr right = parseTerm();
        const DataType type = (left->type == DataType::Double || right->type == DataType::Double)
                                  ? DataType::Double : DataType::Integer;
        left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right), type, opTok.line);
    }
    return left;
}

ExprPtr Parser::parseTerm() {
    const Token& t = peek();
    switch (t.type) {
        case TokenType::IntLiteral: {
            if (config::ENFORCE_LITERAL_LIMITS ? t.lexeme.size() != 1 : t.lexeme.size() > 9)
                fail(t, "integer '" + t.lexeme + "' is not allowed: integers must be a single digit (0-9)");
            advance();
            return std::make_unique<NumberExpr>(Value::makeInt(std::stoi(t.lexeme)), t.line);
        }
        case TokenType::DoubleLiteral: {
            const size_t decimals = t.lexeme.size() - t.lexeme.find('.') - 1;
            if (config::ENFORCE_LITERAL_LIMITS && decimals > 2)
                fail(t, "double '" + t.lexeme + "' has more than 2 decimal places");
            advance();
            const double v = std::round(std::stod(t.lexeme) * 100.0) / 100.0;
            return std::make_unique<NumberExpr>(Value::makeDouble(v), t.line);
        }
        case TokenType::Identifier: {
            auto it = symbols_.find(t.lexeme);
            if (it == symbols_.end())
                fail(t, "variable '" + t.lexeme + "' is used before it is declared");
            advance();
            return std::make_unique<VarExpr>(t.lexeme, it->second, t.line);
        }
        default:
            fail(t, "expected a number or a variable, found " + describe(t));
    }
}
