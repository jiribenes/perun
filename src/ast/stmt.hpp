#ifndef PERUN_AST_STMT_HPP
#define PERUN_AST_STMT_HPP

#include <string>

#include "node.hpp"

namespace perun {
namespace ast {

class Expr;

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
    VarDecl(bool isConst, std::string name, std::unique_ptr<Expr>&& typeExpr,
            std::unique_ptr<Expr>&& expr);

private:
    bool isConst;
    std::string name;
    // TODO: change this to allow first-class types
    std::unique_ptr<ast::Expr> typeExpr; // can be null
    std::unique_ptr<ast::Expr> expr;     // can be null
};

class Return : public Stmt {
public:
    explicit Return(size_t returnToken, std::unique_ptr<ast::Expr>&& expr);

private:
    size_t returnToken;
    std::unique_ptr<ast::Expr> expr; // can be null
};

} // namespace ast
} // namespace perun

#endif // PERUN_AST_STMT_HPP
