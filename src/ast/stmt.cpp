#include "stmt.hpp"

#include "expr.hpp"

namespace perun {
namespace ast {

VarDecl::VarDecl(bool isConst, std::string name,
                 std::unique_ptr<Expr>&& typeExpr, std::unique_ptr<Expr>&& expr)
    : Stmt(Node::Kind::VarDecl), isConst(isConst), name(name),
      typeExpr(std::move(typeExpr)), expr(std::move(expr)) {}

} // namespace ast
} // namespace perun
