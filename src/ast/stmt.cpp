#include "stmt.hpp"

#include "expr.hpp"

namespace perun {
namespace ast {

VarDecl::VarDecl(bool constant, std::string name,
                 std::unique_ptr<Expr>&& typeExpr, std::unique_ptr<Expr>&& expr)
    : Stmt(Node::Kind::VarDecl), constant(constant), name(name),
      typeExpr(std::move(typeExpr)), expr(std::move(expr)) {}

Return::Return(size_t returnToken, std::unique_ptr<ast::Expr>&& expr)
    : Stmt(Node::Kind::Return), returnToken(returnToken),
      expr(std::move(expr)) {}

} // namespace ast
} // namespace perun
