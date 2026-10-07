// Ast.h - the tree the parser builds and the interpreter walks.
#pragma once
#include <memory>
#include <string>
#include <utility>
#include <vector>

enum class DataType { Integer, Double };

// A runtime value: either an int or a double.
struct Value {
    DataType type = DataType::Integer;
    int i = 0;
    double d = 0.0;

    static Value makeInt(int v)      { Value x; x.type = DataType::Integer; x.i = v; return x; }
    static Value makeDouble(double v){ Value x; x.type = DataType::Double;  x.d = v; return x; }
    double asDouble() const { return type == DataType::Integer ? static_cast<double>(i) : d; }
};

// ---------------------------------------------------------------- expressions
struct Expr {
    enum class Kind { Number, Variable, Binary };
    Kind kind;
    DataType type;  // static type, worked out by the parser
    int line;
    Expr(Kind k, DataType t, int l) : kind(k), type(t), line(l) {}
    virtual ~Expr() = default;
};
using ExprPtr = std::unique_ptr<Expr>;

struct NumberExpr final : Expr {
    Value value;
    NumberExpr(Value v, int line) : Expr(Kind::Number, v.type, line), value(v) {}
};
struct VarExpr final : Expr {
    std::string name;
    VarExpr(std::string n, DataType t, int line)
        : Expr(Kind::Variable, t, line), name(std::move(n)) {}
};
struct BinaryExpr final : Expr {
    char op;  // '+' or '-'
    ExprPtr lhs, rhs;
    BinaryExpr(char o, ExprPtr l, ExprPtr r, DataType t, int line)
        : Expr(Kind::Binary, t, line), op(o), lhs(std::move(l)), rhs(std::move(r)) {}
};

// ----------------------------------------------------------------- statements
enum class RelOp { Less, Greater, Equal, NotEqual };

struct Stmt {
    enum class Kind { Declare, Assign, Output, If };
    Kind kind;
    int line;
    Stmt(Kind k, int l) : kind(k), line(l) {}
    virtual ~Stmt() = default;
};
using StmtPtr = std::unique_ptr<Stmt>;

struct DeclareStmt final : Stmt {
    std::string name;
    DataType type;
    DeclareStmt(std::string n, DataType t, int line)
        : Stmt(Kind::Declare, line), name(std::move(n)), type(t) {}
};
struct AssignStmt final : Stmt {
    std::string name;
    ExprPtr value;
    AssignStmt(std::string n, ExprPtr v, int line)
        : Stmt(Kind::Assign, line), name(std::move(n)), value(std::move(v)) {}
};
struct OutputStmt final : Stmt {
    bool isString;
    std::string text;  // used when isString
    ExprPtr value;     // used otherwise
    OutputStmt(std::string t, int line)
        : Stmt(Kind::Output, line), isString(true), text(std::move(t)) {}
    OutputStmt(ExprPtr v, int line)
        : Stmt(Kind::Output, line), isString(false), value(std::move(v)) {}
};
struct IfStmt final : Stmt {
    ExprPtr lhs;
    RelOp op;
    ExprPtr rhs;
    StmtPtr body;  // one-way if: exactly one statement, no else
    IfStmt(ExprPtr l, RelOp o, ExprPtr r, StmtPtr b, int line)
        : Stmt(Kind::If, line), lhs(std::move(l)), op(o), rhs(std::move(r)), body(std::move(b)) {}
};

using Program = std::vector<StmtPtr>;
