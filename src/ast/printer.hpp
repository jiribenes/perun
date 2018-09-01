#ifndef PERUN_AST_PRINTER_HPP
#define PERUN_AST_PRINTER_HPP

#include <ostream>

namespace perun {
namespace ast {
// predeclared nodes
class Root;

class Stmt;
class Block;
class VarDecl;
class Return;

class Expr;
class Identifier;
class GroupedExpr;

class Literal;
class LiteralInteger;
class LiteralString;
class LiteralBoolean;

// TODO: generalize this into a proper recursive AST visitor
class Printer {
public:
    Printer(std::ostream& os, size_t indent) : os(os), indent(indent) {}

    void printRoot(const Root& root);

    void printStmt(const Stmt& stmt);
    void printBlock(const Block& block);
    void printVarDecl(const VarDecl& varDecl);
    void printReturn(const Return& ret);

    void printExpr(const Expr& expr);
    void printIdentifier(const Identifier& id);
    void printGroupedExpr(const GroupedExpr& grouped);

    void printLiteral(const Literal& lit);
    void printLiteralInteger(const LiteralInteger& lit);
    void printLiteralString(const LiteralString& lit);
    void printLiteralBoolean(const LiteralBoolean& lit);

private:
    void printIndent();

    std::ostream& os;
    size_t indent;
};

} // namespace ast
} // namespace perun

#endif // PERUN_AST_PRINTER_HPP
