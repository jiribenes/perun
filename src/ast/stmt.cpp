#include "stmt.hpp"

#include "expr.hpp"

namespace perun {
namespace ast {

VarDecl::VarDecl(bool constant, std::unique_ptr<Identifier>&& identifier,
                 std::unique_ptr<Expr>&& typeExpr, std::unique_ptr<Expr>&& expr)
    : Stmt(Node::Kind::VarDecl), constant(constant),
      identifier(std::move(identifier)), typeExpr(std::move(typeExpr)),
      expr(std::move(expr)) {}

ParamDecl::ParamDecl(std::unique_ptr<Identifier>&& identifier,
                     std::unique_ptr<Expr>&& type)
    : Stmt(Node::Kind::ParamDecl), identifier(std::move(identifier)),
      type(std::move(type)) {}

FnDecl::FnDecl(std::unique_ptr<Identifier>&& identifier,
               std::vector<std::unique_ptr<ParamDecl>>&& params,
               std::unique_ptr<Expr>&& returnType,
               std::unique_ptr<Block>&& body, bool pub, bool _extern,
               bool _export)
    : Stmt(Node::Kind::FnDecl), identifier(std::move(identifier)),
      params(std::move(params)), returnType(std::move(returnType)),
      body(std::move(body)), pub(pub), _extern(_extern), _export(_export) {}

Return::Return(size_t returnToken, std::unique_ptr<Expr>&& expr)
    : Stmt(Node::Kind::Return), returnToken(returnToken),
      expr(std::move(expr)) {}

} // namespace ast
} // namespace perun
