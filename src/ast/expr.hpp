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

// operations:
enum class PrefixOp : short {
    Invalid = -1,

    Address,
    BitNot,
    BoolNot,
    Negate,

    OptionalType,
};

class PrefixExpr : public Expr {
public:
    using Op = PrefixOp;

    explicit PrefixExpr(std::unique_ptr<Expr>&& rhs, Op op, size_t opToken)
        : Expr(Node::Kind::PrefixExpr), rhs(std::move(rhs)), op(op),
          opToken(opToken) {}

    /// Predicates for checking the op
    bool is(Op o) const { return op == o; }
    bool isNot(Op o) const { return op != o; }
    bool isOneOf(Op o1, Op o2) const { return is(o1) || is(o2); }
    template <typename... Ts> bool isOneOf(Op o1, Op o2, Ts... os) const {
        return is(o1) || isOneOf(o2, os...);
    }

    const Expr* getRHS() const { return rhs.get(); }
    Op getOp() const { return op; }
    size_t firstTokenIndex() const override { return opToken; }
    size_t lastTokenIndex() const override { return rhs->lastTokenIndex(); }

private:
    std::unique_ptr<Expr> rhs;
    Op op;
    size_t opToken;
};

enum class InfixOp : short {
    Invalid = -1,

    // assign
    Assign,
    AssignBitAnd,
    AssignBitOr,
    AssignBitSHL,
    AssignBitSHR,
    AssignDiv,
    AssignMod,
    AssignMul,
    AssignPlus,
    AssignSub,

    // bitwise
    BitAnd,
    BitOr,
    BitSHL,
    BitSHR,

    // boolean/conditional
    BoolAnd,
    BoolOr,

    // comparison
    EqualEqual,
    Greater,
    GreaterEqual,
    Less,
    LessEqual,
    NotEqual,

    // multiplicative
    Div,
    Mod,
    Mul,

    // additive
    Add,
    Sub,
};

class InfixExpr : public Expr {
public:
    using Op = InfixOp;

    explicit InfixExpr(std::unique_ptr<Expr>&& lhs, std::unique_ptr<Expr>&& rhs,
                       Op op, size_t opToken)
        : Expr(Node::Kind::InfixExpr), lhs(std::move(lhs)), rhs(std::move(rhs)),
          op(op), opToken(opToken) {}

    /// Predicates for checking the op
    bool is(Op o) const { return op == o; }
    bool isNot(Op o) const { return op != o; }
    bool isOneOf(Op o1, Op o2) const { return is(o1) || is(o2); }
    template <typename... Ts> bool isOneOf(Op o1, Op o2, Ts... os) const {
        return is(o1) || isOneOf(o2, os...);
    }

    const Expr* getLHS() const { return lhs.get(); }
    const Expr* getRHS() const { return rhs.get(); }
    Op getOp() const { return op; }
    size_t firstTokenIndex() const override { return lhs->firstTokenIndex(); }
    size_t lastTokenIndex() const override { return rhs->lastTokenIndex(); }

private:
    std::unique_ptr<Expr> lhs;
    std::unique_ptr<Expr> rhs;
    Op op;
    size_t opToken;
};

enum class SuffixOp : short {
    Invalid = -1,

    Deref,
    Unwrap,
};

class SuffixExpr : public Expr {
public:
    using Op = SuffixOp;

    explicit SuffixExpr(std::unique_ptr<Expr>&& lhs, Op op, size_t opToken)
        : Expr(Node::Kind::SuffixExpr), lhs(std::move(lhs)), op(op),
          opToken(opToken) {}

    /// Predicates for checking the op
    bool is(Op o) const { return op == o; }
    bool isNot(Op o) const { return op != o; }
    bool isOneOf(Op o1, Op o2) const { return is(o1) || is(o2); }
    template <typename... Ts> bool isOneOf(Op o1, Op o2, Ts... os) const {
        return is(o1) || isOneOf(o2, os...);
    }

    const Expr* getLHS() const { return lhs.get(); }
    Op getOp() const { return op; }
    size_t firstTokenIndex() const override { return lhs->firstTokenIndex(); }
    size_t lastTokenIndex() const override { return opToken; }

private:
    std::unique_ptr<Expr> lhs;
    Op op;
    size_t opToken;
};

} // namespace ast
} // namespace perun

#endif // PERUN_AST_EXPR_HPP
