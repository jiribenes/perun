#include "printer.hpp"

#include <cassert>

#include "expr.hpp"
#include "node.hpp"
#include "stmt.hpp"
#include "literal.hpp"

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
    case Node::Kind::ParamDecl: {
        printParamDecl(static_cast<const ParamDecl&>(stmt));
        break;
    }
    case Node::Kind::FnDecl: {
        printFnDecl(static_cast<const FnDecl&>(stmt));
        break;
    }
    case Node::Kind::Return: {
        printReturn(static_cast<const Return&>(stmt));
        break;
    }
    case Node::Kind::IfStmt: {
        printIfStmt(static_cast<const IfStmt&>(stmt));
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

    os << "{\n";
    indent += 4;
    for (auto&& stmt : stmts) {
        printIndent();
        printStmt(*stmt);
        os << '\n';
    }

    indent -= 4;
    printIndent();
    os << "}";
}

void Printer::printVarDecl(const VarDecl& varDecl) {
    auto&& mutabilityStr = varDecl.isConst() ? "const" : "var";
    os << mutabilityStr << ' ' << varDecl.getIdentifier()->getName();

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

void Printer::printParamDecl(const ParamDecl& paramDecl) {
    auto&& identifier = paramDecl.getIdentifier();
    if (identifier != nullptr) {
        printIdentifier(*identifier);
        os << ": ";
    }

    auto&& typeExpr = paramDecl.getType();
    printExpr(*typeExpr);
}

void Printer::printFnDecl(const FnDecl& fnDecl) {
    if (fnDecl.isPub()) {
        os << "pub ";
    }

    if (fnDecl.isExtern()) {
        os << "extern ";
    } else if (fnDecl.isExport()) {
        os << "export ";
    }

    os << "fn ";

    auto&& identifier = fnDecl.getIdentifier();
    if (identifier != nullptr) {
        printIdentifier(*identifier);
    }

    { // params
        os << "(";

        size_t paramsSize = fnDecl.getParams().size();
        for (size_t i = 0; i < paramsSize; ++i) {
            auto&& param = fnDecl.getParam(i);
            printParamDecl(*param);

            if (i + 1 < paramsSize) {
                os << ", ";
            }
        }

        os << ")";
    }

    auto&& returnType = fnDecl.getReturnType();
    if (returnType != nullptr) {
        os << " -> ";
        printExpr(*returnType);
    }

    auto&& body = fnDecl.getBody();
    if (body != nullptr) {
        os << " ";
        printBlock(*body);
    } else {
        os << ";\n";
    }
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

void Printer::printIfStmt(const IfStmt& ifStmt) {
    os << "if ";

    auto&& condition = ifStmt.getCondition();
    printExpr(*condition);

    os << ' ';

    auto&& then = ifStmt.getThenBlock();
    printBlock(*then);

    auto&& otherwise = ifStmt.getElseBlock();
    if (otherwise != nullptr) {
        os << " else ";
        printBlock(*otherwise);
    }

    os << '\n';
}

void Printer::printExpr(const Expr& expr) {
    // TODO: use llvm RTTI
    switch (expr.getKind()) {
    case Node::Kind::Identifier: {
        printIdentifier(static_cast<const Identifier&>(expr));
        break;
    }
    case Node::Kind::GroupedExpr: {
        printGroupedExpr(static_cast<const GroupedExpr&>(expr));
        break;
    }
    case Node::Kind::SuffixExpr: {
        printSuffixExpr(static_cast<const SuffixExpr&>(expr));
        break;
    }
    case Node::Kind::LiteralInteger:
    case Node::Kind::LiteralString:
    case Node::Kind::LiteralBoolean:
    case Node::Kind::LiteralNil:
    case Node::Kind::LiteralUndefined: {
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

void Printer::printGroupedExpr(const GroupedExpr& grouped) {
    os << '(';

    auto&& inner = grouped.getExpr();
    assert(inner != nullptr);
    printExpr(*inner);

    os << ')';
}

void Printer::printSuffixExpr(const SuffixExpr& expr) {
    auto&& lhs = expr.getLHS();

    assert(lhs != nullptr);

    printExpr(*lhs);
    printSuffixOp(expr.getOp());
}

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
    case Node::Kind::LiteralNil: {
        printLiteralNil(static_cast<const LiteralNil&>(lit));
        break;
    }
    case Node::Kind::LiteralUndefined: {
        printLiteralUndefined(static_cast<const LiteralUndefined&>(lit));
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

void Printer::printLiteralNil(const LiteralNil& lit) { os << "nil"; }

void Printer::printLiteralUndefined(const LiteralUndefined& lit) {
    os << "undefined";
}

void Printer::printSuffixOp(const SuffixOp& op) {
    switch (op) {
    case SuffixOp::Deref: {
        os << "^";
        return;
    }
    case SuffixOp::Unwrap: {
        os << "?";
        return;
    }
    default: { assert(false); }
    }
}

} // namespace ast
} // namespace perun
