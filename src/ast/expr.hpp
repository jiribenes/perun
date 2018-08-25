#ifndef PERUN_AST_EXPR_HPP
#define PERUN_AST_EXPR_HPP

#include <string>

#include "node.hpp"

namespace perun {
namespace ast {

class Expr : public Node {
public:
    explicit Expr(Kind kind) : Node(kind) {}
};

class Literal : public Expr {
public:
    explicit Literal(Kind kind) : Expr(kind) {}
};

class LiteralInteger : public Literal {
public:
    explicit LiteralInteger(uint64_t value)
        : Literal(Node::Kind::LiteralInteger), value(value) {}

    uint64_t getValue() const { return value; }

private:
    // TODO: use infinite precision integer
    const uint64_t value;
};

class LiteralString : public Literal {
public:
    explicit LiteralString(std::string&& str, bool c = false, bool raw = false)
        : Literal(Node::Kind::LiteralString), str(std::move(str)), c(c),
          raw(raw) {}

    bool isC() const { return c; }
    bool isRaw() const { return raw; }

private:
    const std::string str;

    const bool c;
    const bool raw;
};

} // namespace ast
} // namespace perun

#endif // PERUN_AST_EXPR_HPP
