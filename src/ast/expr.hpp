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

class Literal : public Expr {
public:
    explicit Literal(Kind kind) : Expr(kind) {}

    virtual ~Literal() = default;

    virtual size_t firstTokenIndex() const = 0;
    virtual size_t lastTokenIndex() const = 0;
};

class LiteralInteger : public Literal {
public:
    explicit LiteralInteger(uint64_t value, size_t intToken)
        : Literal(Node::Kind::LiteralInteger), value(value),
          intToken(intToken) {}

    uint64_t getValue() const { return value; }

    size_t firstTokenIndex() const override { return intToken; }
    size_t lastTokenIndex() const override { return intToken; }

private:
    // TODO: use infinite precision integer
    const uint64_t value;

    size_t intToken;
};

class LiteralString : public Literal {
public:
    explicit LiteralString(std::string&& str, bool c, bool raw, size_t strToken)
        : Literal(Node::Kind::LiteralString), str(std::move(str)), c(c),
          raw(raw), strToken(strToken) {}

    const std::string& getValue() const { return str; }
    bool isC() const { return c; }
    bool isRaw() const { return raw; }

    size_t firstTokenIndex() const override { return strToken; }
    size_t lastTokenIndex() const override { return strToken; }

private:
    const std::string str;

    const bool c;
    const bool raw;

    size_t strToken;
};

class LiteralBoolean : public Literal {
public:
    explicit LiteralBoolean(bool value, size_t boolToken)
        : Literal(Node::Kind::LiteralBoolean), value(value),
          boolToken(boolToken) {}

    bool getValue() const { return value; }

    size_t firstTokenIndex() const override { return boolToken; }
    size_t lastTokenIndex() const override { return boolToken; }

private:
    const bool value;

    size_t boolToken;
};

class LiteralNil : public Literal {
public:
    explicit LiteralNil(size_t nilToken)
        : Literal(Node::Kind::LiteralNil), nilToken(nilToken) {}

    size_t firstTokenIndex() const override { return nilToken; }
    size_t lastTokenIndex() const override { return nilToken; }

private:
    size_t nilToken;
};

class LiteralUndefined : public Literal {
public:
    explicit LiteralUndefined(size_t undefinedToken)
        : Literal(Node::Kind::LiteralUndefined),
          undefinedToken(undefinedToken) {}

    size_t firstTokenIndex() const override { return undefinedToken; }
    size_t lastTokenIndex() const override { return undefinedToken; }

private:
    size_t undefinedToken;
};

} // namespace ast
} // namespace perun

#endif // PERUN_AST_EXPR_HPP
