#ifndef PERUN_AST_STMT_HPP
#define PERUN_AST_STMT_HPP

#include <experimental/optional>
#include <string>

#include "node.hpp"

namespace perun {
namespace ast {

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

} // namespace ast
} // namespace perun

#endif // PERUN_AST_STMT_HPP
