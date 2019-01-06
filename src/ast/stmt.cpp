#include "stmt.hpp"

#include "expr.hpp"

namespace perun {
namespace ast {

VarDecl::VarDecl(bool constant, std::unique_ptr<Identifier>&& identifier,
                 std::unique_ptr<Expr>&& typeExpr, std::unique_ptr<Expr>&& expr,
                 size_t varToken, size_t semicolonToken)
    : Stmt(Node::Kind::VarDecl), constant(constant),
      identifier(std::move(identifier)), typeExpr(std::move(typeExpr)),
      expr(std::move(expr)), varToken(varToken),
      semicolonToken(semicolonToken) {}

ParamDecl::ParamDecl(std::unique_ptr<Identifier>&& identifier,
                     std::unique_ptr<Expr>&& type)
    : Stmt(Node::Kind::ParamDecl), identifier(std::move(identifier)),
      type(std::move(type)) {}

size_t ParamDecl::firstTokenIndex() const {
    if (identifier != nullptr) {
        return identifier->firstTokenIndex();
    }
    return type->firstTokenIndex();
}

size_t ParamDecl::lastTokenIndex() const { return type->firstTokenIndex(); }

FnDecl::FnDecl(std::unique_ptr<Identifier>&& identifier,
               std::vector<std::unique_ptr<ParamDecl>>&& params,
               std::unique_ptr<Expr>&& returnType,
               std::unique_ptr<Block>&& body, bool pub, bool _extern,
               bool _export, size_t fnToken, size_t pubToken,
               size_t modifierToken, size_t semicolonToken)
    : Stmt(Node::Kind::FnDecl), identifier(std::move(identifier)),
      params(std::move(params)), returnType(std::move(returnType)),
      body(std::move(body)), pub(pub), _extern(_extern), _export(_export),
      fnToken(fnToken), pubToken(pubToken), modifierToken(modifierToken),
      semicolonToken(semicolonToken) {}

size_t FnDecl::firstTokenIndex() const {
    if (isPub()) {
        return pubToken;
    }

    if (isExtern() || isExport()) {
        return modifierToken;
    }

    return fnToken;
}

size_t FnDecl::lastTokenIndex() const {
    if (hasBody()) {
        return body->lastTokenIndex();
    }

    return semicolonToken;
}

Return::Return(std::unique_ptr<Expr>&& expr, size_t returnToken,
               size_t semicolonToken)
    : Stmt(Node::Kind::Return), expr(std::move(expr)), returnToken(returnToken),
      semicolonToken(semicolonToken) {}

IfStmt::IfStmt(std::unique_ptr<Expr>&& condition, std::unique_ptr<Block>&& then,
               std::unique_ptr<Block>&& otherwise, size_t ifToken,
               size_t elseToken)
    : Stmt(Node::Kind::IfStmt), condition(std::move(condition)),
      then(std::move(then)), otherwise(std::move(otherwise)), ifToken(ifToken),
      elseToken(elseToken) {}

AssignStmt::AssignStmt(std::unique_ptr<Expr>&& lhs, std::unique_ptr<Expr>&& rhs,
                       Op op, size_t opToken, size_t semicolonToken)
    : Stmt(Node::Kind::AssignStmt), lhs(std::move(lhs)), rhs(std::move(rhs)),
      op(op), opToken(opToken), semicolonToken(semicolonToken) {}

size_t AssignStmt::firstTokenIndex() const { return lhs->firstTokenIndex(); }
size_t AssignStmt::lastTokenIndex() const { return semicolonToken; }

} // namespace ast
} // namespace perun
