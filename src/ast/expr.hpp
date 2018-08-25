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

private:
    uint64_t value;
};

class LiteralString : public Literal {
public:
    LiteralString(std::string&& str, bool isC = false, bool isRaw = false)
        : Literal(Node::Kind::LiteralString), str(std::move(str)), isC(isC),
          isRaw(isRaw) {}

private:
    std::string str;

    bool isC;
    bool isRaw;
};

} // namespace ast
} // namespace perun

#endif // PERUN_AST_EXPR_HPP
