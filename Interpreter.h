// Interpreter.h - executes a program that the parser has already validated.
#pragma once
#include <map>
#include <ostream>
#include <string>
#include "Ast.h"

class Interpreter {
public:
    explicit Interpreter(std::ostream& out) : out_(out) {}
    void run(const Program& program);

private:
    void exec(const Stmt& stmt);
    Value eval(const Expr& expr) const;
    bool compare(const Value& l, RelOp op, const Value& r) const;
    static std::string format(const Value& v);

    std::ostream& out_;
    std::map<std::string, Value> vars_;
};
