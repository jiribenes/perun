#ifndef PERUN_AST_STMT_HPP
#define PERUN_AST_STMT_HPP

#include <cassert>
#include <string>

#include "node.hpp"

namespace perun {
namespace ast {

class Expr;
class Identifier;

class Stmt : public Node {
public:
    explicit Stmt(Node::Kind kind) : Node(kind) {}

    virtual ~Stmt() = default;

    virtual size_t firstTokenIndex() const = 0;
    virtual size_t lastTokenIndex() const = 0;
};

class Block : public Stmt {
public:
    Block(size_t lBraceToken, size_t rBraceToken,
          std::vector<std::unique_ptr<Stmt>>&& stmts)
        : Stmt(Node::Kind::Block), lBraceToken(lBraceToken),
          rBraceToken(rBraceToken), stmts(std::move(stmts)), labelToken(0),
          hasLabel(false) {}
    Block(size_t lBraceToken, size_t rBraceToken,
          std::vector<std::unique_ptr<Stmt>>&& stmts, size_t labelToken)
        : Stmt(Node::Kind::Block), lBraceToken(lBraceToken),
          rBraceToken(rBraceToken), stmts(std::move(stmts)),
          labelToken(labelToken), hasLabel(true) {}

    const std::vector<std::unique_ptr<Stmt>>& getStmts() const { return stmts; }

    void addStmt(std::unique_ptr<Stmt>&& stmt) {
        stmts.push_back(std::move(stmt));
    }

    size_t firstTokenIndex() const override { return lBraceToken; }
    size_t lastTokenIndex() const override { return rBraceToken; }

private:
    size_t lBraceToken;
    size_t rBraceToken;

    std::vector<std::unique_ptr<Stmt>> stmts;

    // TODO: use an optional type (?)
    size_t labelToken;
    bool hasLabel;
};

class VarDecl : public Stmt {
public:
    // defined in stmt.cpp
    VarDecl(bool constant, std::unique_ptr<Identifier>&& identifier,
            std::unique_ptr<Expr>&& typeExpr, std::unique_ptr<Expr>&& expr,
            size_t varToken, size_t semicolonToken);

    bool isConst() const { return constant; }

    const Identifier* getIdentifier() const { return identifier.get(); }

    // can be null
    const Expr* getType() const { return typeExpr.get(); }

    // can be null
    const Expr* getExpr() const { return expr.get(); }

    size_t firstTokenIndex() const override { return varToken; }
    size_t lastTokenIndex() const override { return semicolonToken; }

private:
    /// true if the vardecl is const
    bool constant;
    std::unique_ptr<Identifier> identifier;
    // TODO: change this to allow first-class types
    std::unique_ptr<Expr> typeExpr; // can be null
    std::unique_ptr<Expr> expr;     // can be null

    size_t varToken;
    size_t semicolonToken;
};

class ParamDecl : public Stmt {
public:
    ParamDecl(std::unique_ptr<Identifier>&& identifier,
              std::unique_ptr<Expr>&& type);

    const Identifier* getIdentifier() const { return identifier.get(); }
    const Expr* getType() const { return type.get(); }

    size_t firstTokenIndex() const override;
    size_t lastTokenIndex() const override;

private:
    std::unique_ptr<Identifier> identifier; // can be null
    std::unique_ptr<Expr> type;
};

class FnDecl : public Stmt {
public:
    FnDecl(std::unique_ptr<Identifier>&& identifier,
           std::vector<std::unique_ptr<ParamDecl>>&& params,
           std::unique_ptr<Expr>&& returnType, std::unique_ptr<Block>&& body,
           bool pub, bool _extern, bool _export, size_t fnToken,
           size_t pubToken, size_t modifierToken, size_t semicolonToken);

    const Identifier* getIdentifier() const { return identifier.get(); }

    const std::vector<std::unique_ptr<ParamDecl>>& getParams() const {
        return params;
    }

    const ParamDecl* getParam(size_t i) const {
        assert(i < params.size());
        return params[i].get();
    }

    const Expr* getReturnType() const { return returnType.get(); }

    bool hasBody() const { return body != nullptr; }
    const Block* getBody() const { return body.get(); }

    bool isPub() const { return pub; }

    bool isExtern() const { return _extern; }
    bool isExport() const { return _export; }

    size_t firstTokenIndex() const override;
    size_t lastTokenIndex() const override;

private:
    std::unique_ptr<Identifier> identifier; // can be null

    std::vector<std::unique_ptr<ParamDecl>> params;

    std::unique_ptr<Expr> returnType; // can be null
    std::unique_ptr<Block> body;      // can be null

    // these have a weird name to prevent clashing with C++ keywords
    bool pub;

    // only one of the following can be active at a time
    bool _extern;
    bool _export;

    // all the ones that don't exist are set to 0
    size_t fnToken, pubToken, modifierToken, semicolonToken;
};

class Return : public Stmt {
public:
    explicit Return(std::unique_ptr<Expr>&& expr, size_t returnToken,
                    size_t semicolonToken);

    // can be null
    const Expr* getExpr() const { return expr.get(); }

    size_t firstTokenIndex() const override { return returnToken; }
    size_t lastTokenIndex() const override { return semicolonToken; }

private:
    std::unique_ptr<Expr> expr; // can be null

    size_t returnToken, semicolonToken;
};

class IfStmt : public Stmt {
public:
    explicit IfStmt(std::unique_ptr<Expr>&& condition,
                    std::unique_ptr<Block>&& then,
                    std::unique_ptr<Block>&& otherwise, size_t ifToken,
                    size_t elseToken);

    const Expr* getCondition() const { return condition.get(); }
    const Block* getThenBlock() const { return then.get(); }

    // can be null
    const Block* getElseBlock() const { return otherwise.get(); }

    size_t firstTokenIndex() const override { return ifToken; }
    size_t lastTokenIndex() const override {
        if (otherwise != nullptr) {
            return otherwise->lastTokenIndex();
        }
        return then->lastTokenIndex();
    }

private:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Block> then;
    std::unique_ptr<Block> otherwise; // can be null

    size_t ifToken, elseToken;
};

enum class AssignOp : short {
    Invalid = -1,

    Assign,
    AssignAdd,
    AssignBitAnd,
    AssignBitOr,
    AssignBitSHL,
    AssignBitSHR,
    AssignDiv,
    AssignMod,
    AssignMul,
    AssignSub,
};

class AssignStmt : public Stmt {
public:
    using Op = AssignOp;

    AssignStmt(std::unique_ptr<Expr>&& lhs, std::unique_ptr<Expr>&& rhs, Op op,
               size_t opToken, size_t semicolonToken);

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
    size_t firstTokenIndex() const override;
    size_t lastTokenIndex() const override;

private:
    std::unique_ptr<Expr> lhs;
    std::unique_ptr<Expr> rhs;
    Op op;
    size_t opToken;
    size_t semicolonToken;
};

} // namespace ast
} // namespace perun

#endif // PERUN_AST_STMT_HPP
