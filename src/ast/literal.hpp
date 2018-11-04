#ifndef PERUN_AST_LITERAL_HPP
#define PERUN_AST_LITERAL_HPP

#include <string>

#include "node.hpp"
#include "expr.hpp"

namespace perun {
namespace ast {

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

#endif // PERUN_AST_LITERAL_HPP

