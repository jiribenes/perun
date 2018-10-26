#ifndef PERUN_AST_NODE_HPP
#define PERUN_AST_NODE_HPP

#include <cstdlib>
#include <memory>
#include <vector>

namespace perun {
namespace ast {

// pre-declared as opaque to avoid unnecessary include
class Stmt;

class Node {
public:
    /// Node kinds (end nodes only)
    enum class Kind {
        Root = 0,

        // statements:
        Block,
        VarDecl,
        ParamDecl,
        FnDecl,
        Return,

        // expressions:
        Identifier,
        GroupedExpr,

        // literals:
        LiteralInteger,
        LiteralString,
        LiteralBoolean,
    };

    explicit Node(Kind kind) : kind(kind) {}

    virtual ~Node() = default;

    bool isStmt() const;
    bool isExpr() const;

    /// Predicates for checking node's kind
    bool is(Kind k) const { return kind == k; }
    bool isNot(Kind k) const { return kind != k; }
    bool isOneOf(Kind k1, Kind k2) const { return is(k1) || is(k2); }
    template <typename... Ts> bool isOneOf(Kind k1, Kind k2, Ts... ks) const {
        return is(k1) || isOneOf(k2, ks...);
    }

    Kind getKind() const { return kind; }

private:
    Kind kind;
};

class Root : public Node {
public:
    explicit Root(); // ctor defined in 'node.cpp'

    void addDecl(std::unique_ptr<Stmt>&& decl);
    void setEOFToken(size_t token) {
        eofToken = token;
        hasEofToken = true;
    }

    const std::vector<std::unique_ptr<Stmt>>& getDecls() const { return decls; }

private:
    std::vector<std::unique_ptr<Stmt>> decls;
    size_t eofToken;
    bool hasEofToken;
};

} // namespace ast
} // namespace perun

#endif // PERUN_AST_NODE_HPP
