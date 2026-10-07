#include "Interpreter.h"
#include <cmath>
#include <iomanip>
#include <sstream>

namespace {
// Doubles in HL carry a precision of 2 decimal places.
double round2(double v) {
    double r = std::round(v * 100.0) / 100.0;
    if (r == 0.0) r = 0.0;  // turns -0.0 into 0.0 so we never print "-0.00"
    return r;
}
}  // namespace

void Interpreter::run(const Program& program) {
    for (const auto& stmt : program) exec(*stmt);
}

void Interpreter::exec(const Stmt& stmt) {
    switch (stmt.kind) {
        case Stmt::Kind::Declare: {
            const auto& s = static_cast<const DeclareStmt&>(stmt);
            // Variables start at 0 so reading one before assigning is harmless.
            vars_[s.name] = (s.type == DataType::Integer) ? Value::makeInt(0)
                                                          : Value::makeDouble(0.0);
            break;
        }
        case Stmt::Kind::Assign: {
            const auto& s = static_cast<const AssignStmt&>(stmt);
            const Value v = eval(*s.value);
            Value& slot = vars_[s.name];
            // int -> double widening is allowed; the parser rejected double -> int.
            slot = (slot.type == DataType::Double) ? Value::makeDouble(round2(v.asDouble())) : v;
            break;
        }
        case Stmt::Kind::Output: {
            const auto& s = static_cast<const OutputStmt&>(stmt);
            out_ << (s.isString ? s.text : format(eval(*s.value))) << '\n';
            break;
        }
        case Stmt::Kind::If: {
            const auto& s = static_cast<const IfStmt&>(stmt);
            if (compare(eval(*s.lhs), s.op, eval(*s.rhs))) exec(*s.body);
            break;
        }
    }
}

Value Interpreter::eval(const Expr& expr) const {
    switch (expr.kind) {
        case Expr::Kind::Number:
            return static_cast<const NumberExpr&>(expr).value;
        case Expr::Kind::Variable:
            return vars_.at(static_cast<const VarExpr&>(expr).name);
        case Expr::Kind::Binary: {
            const auto& e = static_cast<const BinaryExpr&>(expr);
            const Value l = eval(*e.lhs), r = eval(*e.rhs);
            const int sign = (e.op == '+') ? 1 : -1;
            if (l.type == DataType::Integer && r.type == DataType::Integer)
                return Value::makeInt(l.i + sign * r.i);
            return Value::makeDouble(round2(l.asDouble() + sign * r.asDouble()));
        }
    }
    return Value{};
}

bool Interpreter::compare(const Value& l, RelOp op, const Value& r) const {
    // Compare in hundredths so that e.g. 0.1+0.2 == 0.3 behaves as expected.
    const long long a = std::llround(l.asDouble() * 100.0);
    const long long b = std::llround(r.asDouble() * 100.0);
    switch (op) {
        case RelOp::Less:     return a <  b;
        case RelOp::Greater:  return a >  b;
        case RelOp::Equal:    return a == b;
        case RelOp::NotEqual: return a != b;
    }
    return false;
}

std::string Interpreter::format(const Value& v) {
    std::ostringstream ss;
    if (v.type == DataType::Integer) ss << v.i;
    else                             ss << std::fixed << std::setprecision(2) << v.d;
    return ss.str();
}
