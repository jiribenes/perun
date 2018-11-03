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
            std::unique_ptr<Expr>&& typeExpr, std::unique_ptr<Expr>&& expr);

    bool isConst() const { return constant; }

    const Identifier* getIdentifier() const { return identifier.get(); }

    // can be null
    const Expr* getType() const { return typeExpr.get(); }

    // can be null
    const Expr* getExpr() const { return expr.get(); }

private:
    /// true if the vardecl is const
    bool constant;
    std::unique_ptr<Identifier> identifier;
    // TODO: change this to allow first-class types
    std::unique_ptr<Expr> typeExpr; // can be null
    std::unique_ptr<Expr> expr;     // can be null
};

class ParamDecl : public Stmt {
public:
    ParamDecl(std::unique_ptr<Identifier>&& identifier,
              std::unique_ptr<Expr>&& type);

    const Identifier* getIdentifier() const { return identifier.get(); }
    const Expr* getType() const { return type.get(); }

private:
    std::unique_ptr<Identifier> identifier; // can be null
    std::unique_ptr<Expr> type;
};

class FnDecl : public Stmt {
public:
    FnDecl(std::unique_ptr<Identifier>&& identifier,
           std::vector<std::unique_ptr<ParamDecl>>&& params,
           std::unique_ptr<Expr>&& returnType, std::unique_ptr<Block>&& body,
           bool pub = false, bool _extern = false, bool _export = false);

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
};

class Return : public Stmt {
public:
    explicit Return(size_t returnToken, std::unique_ptr<Expr>&& expr);

    // can be null
    const Expr* getExpr() const { return expr.get(); }

private:
    size_t returnToken;
    std::unique_ptr<Expr> expr; // can be null
};

class IfStmt : public Stmt {
public:
    explicit IfStmt(std::unique_ptr<Expr>&& condition,
                    std::unique_ptr<Block>&& then,
                    std::unique_ptr<Block>&& otherwise);

    const Expr* getCondition() const { return condition.get(); }
    const Block* getThenBlock() const { return then.get(); }

    // can be null
    const Block* getElseBlock() const { return otherwise.get(); }

private:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Block> then;
    std::unique_ptr<Block> otherwise; // can be null
};

} // namespace ast
} // namespace perun

#endif // PERUN_AST_STMT_HPP
