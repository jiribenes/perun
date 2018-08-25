#include "node.hpp"

#include "stmt.hpp"

namespace perun {
namespace ast {

Root::Root(size_t eofToken)
    : Node(Node::Kind::Root), decls(std::vector<std::unique_ptr<Stmt>>{}),
      eofToken(eofToken) {}

void Root::addDecl(std::unique_ptr<Stmt>&& decl) {
    decls.push_back(std::move(decl));
}

} // namespace ast
} // namespace perun
