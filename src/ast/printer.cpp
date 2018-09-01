#include "printer.hpp"

#include "expr.hpp"
#include "node.hpp"
#include "stmt.hpp"

namespace perun {
namespace ast {

void Printer::printIndent() {
    for (size_t i = 0; i < indent; ++i) {
        os << ' ';
    }
}

void Printer::printRoot(const Root& root) {
    for (auto&& decl : root.getDecls()) {
        printIndent();
        printStmt(*decl);
        os << '\n';
    }
}

void Printer::printStmt(const Stmt& stmt) {
    // TODO: use llvm RTTI
    switch (stmt.getKind()) {
    case Node::Kind::Block: {
        printBlock(static_cast<const Block&>(stmt));
        break;
    }
    case Node::Kind::VarDecl: {
        printVarDecl(static_cast<const VarDecl&>(stmt));
        break;
    }
    case Node::Kind::Return: {
        printReturn(static_cast<const Return&>(stmt));
        break;
    }
    default: {
        // TODO: raise error
        break;
    }
    }
}

void Printer::printBlock(const Block& block) {
    auto&& stmts = block.getStmts();
    if (stmts.empty()) {
        os << "{}";
        return;
    }

    os << "{";
    indent += 4;
    for (auto&& stmt : stmts) {
        printIndent();
        printStmt(*stmt);
        os << '\n';
    }

    indent -= 4;
    os << "}";
}

void Printer::printVarDecl(const VarDecl& varDecl) {
    auto&& mutabilityStr = varDecl.isConst() ? "const" : "var";
    os << mutabilityStr << ' ' << varDecl.getName();

    auto&& type = varDecl.getType();
    if (type != nullptr) {
        os << ": ";
        printExpr(*type);
    }

    auto&& expr = varDecl.getExpr();
    if (expr != nullptr) {
        os << " = ";
        printExpr(*expr);
    }

    os << ";";
}

void Printer::printReturn(const Return& ret) {
    os << "return";

    auto&& expr = ret.getExpr();
    if (expr != nullptr) {
        os << ' ';
        printExpr(*expr);
    }

    os << ";";
}

void Printer::printExpr(const Expr& expr) {
    // TODO: use llvm RTTI
    switch (expr.getKind()) {
    case Node::Kind::Identifier: {
        printIdentifier(static_cast<const Identifier&>(expr));
        break;
    }
    case Node::Kind::LiteralInteger:
    case Node::Kind::LiteralString:
    case Node::Kind::LiteralBoolean: {
        printLiteral(static_cast<const Literal&>(expr));
        break;
    }
    default: {
        // TODO: raise error
        break;
    }
    }
}

void Printer::printIdentifier(const Identifier& id) { os << id.getName(); }

void Printer::printLiteral(const Literal& lit) {
    // TODO: use llvm RTTI
    switch (lit.getKind()) {
    case Node::Kind::LiteralInteger: {
        printLiteralInteger(static_cast<const LiteralInteger&>(lit));
        break;
    }
    case Node::Kind::LiteralString: {
        printLiteralString(static_cast<const LiteralString&>(lit));
        break;
    }
    case Node::Kind::LiteralBoolean: {
        printLiteralBoolean(static_cast<const LiteralBoolean&>(lit));
        break;
    }
    default: {
        // TODO: raise error
        break;
    }
    }
}

void Printer::printLiteralInteger(const LiteralInteger& lit) {
    os << lit.getValue();
}

void Printer::printLiteralString(const LiteralString& lit) {
    // TODO: handle escapes, raw strings, c strings
    os << "\"" << lit.getValue() << "\"";
}

void Printer::printLiteralBoolean(const LiteralBoolean& lit) {
    auto&& boolStr = lit.getValue() ? "true" : "false";
    os << boolStr;
}

} // namespace ast
} // namespace perun
