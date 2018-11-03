#include "node.hpp"

#include "stmt.hpp"

namespace perun {
namespace ast {

bool Node::isStmt() const {
    switch (kind) {
    case Kind::Block:
    case Kind::VarDecl:
    case Kind::ParamDecl:
    case Kind::FnDecl:
    case Kind::Return:
    case Kind::IfStmt: {
        return true;
    }
    default: { return false; }
    }
}

bool Node::isExpr() const {
    switch (kind) {
    case Kind::Identifier:
    case Kind::GroupedExpr: {
        return true;
    }
    default: {
        // all literals are expressions
        return isLiteral();
    }
    }
}

bool Node::isLiteral() const {
    switch (kind) {
    case Kind::LiteralInteger:
    case Kind::LiteralString:
    case Kind::LiteralBoolean:
    case Kind::LiteralNil:
    case Kind::LiteralUndefined: {
        return true;
    }
    default: { return false; }
    }
}

Root::Root()
    : Node(Node::Kind::Root), decls(std::vector<std::unique_ptr<Stmt>>{}),
      eofToken(0), hasEofToken(false) {}

void Root::addDecl(std::unique_ptr<Stmt>&& decl) {
    decls.push_back(std::move(decl));
}

} // namespace ast
} // namespace perun
