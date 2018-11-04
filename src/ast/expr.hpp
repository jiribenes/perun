#ifndef PERUN_AST_EXPR_HPP
#define PERUN_AST_EXPR_HPP

#include <string>

#include "node.hpp"

namespace perun {
namespace ast {

class Expr : public Node {
public:
    explicit Expr(Kind kind) : Node(kind) {}

    virtual ~Expr() = default;

    virtual size_t firstTokenIndex() const = 0;
    virtual size_t lastTokenIndex() const = 0;
};

class Identifier : public Expr {
public:
    explicit Identifier(std::string name, size_t idToken)
        : Expr(Node::Kind::Identifier), name(name), idToken(idToken) {}

    const std::string& getName() const { return name; }

    size_t firstTokenIndex() const override { return idToken; }
    size_t lastTokenIndex() const override { return idToken; }

private:
    std::string name;

    size_t idToken;
};

class GroupedExpr : public Expr {
public:
    explicit GroupedExpr(std::unique_ptr<Expr>&& expr, size_t lParenToken,
                         size_t rParenToken)
        : Expr(Node::Kind::GroupedExpr), expr(std::move(expr)),
          lParenToken(lParenToken), rParenToken(rParenToken) {}

    // always non-null
    const Expr* getExpr() const { return expr.get(); }

    size_t firstTokenIndex() const override { return lParenToken; }
    size_t lastTokenIndex() const override { return rParenToken; }

private:
    std::unique_ptr<Expr> expr;

    size_t lParenToken, rParenToken;
};

} // namespace ast
} // namespace perun

#endif // PERUN_AST_EXPR_HPP
