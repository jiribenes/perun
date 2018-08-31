#include "node.hpp"

#include "stmt.hpp"

namespace perun {
namespace ast {

Root::Root()
    : Node(Node::Kind::Root), decls(std::vector<std::unique_ptr<Stmt>>{}),
      eofToken(0), hasEofToken(false) {}

void Root::addDecl(std::unique_ptr<Stmt>&& decl) {
    decls.push_back(std::move(decl));
}

} // namespace ast
} // namespace perun
