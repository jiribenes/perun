#ifndef PERUN_PARSER_PARSER_HPP
#define PERUN_PARSER_PARSER_HPP

#include <memory>

#include "error.hpp"
#include "token.hpp"
#include "tokenizer.hpp"

namespace perun {

// pre-declared as opaque to avoid unnecessary include
namespace ast {
class Tree;

class Root;

class Stmt;
class Block;

class VarDecl;
class ParamDecl;
class FnDecl;
class Return;
class IfStmt;
class AssignStmt;

class Expr;
class GroupedExpr;
class Identifier;
class PrefixExpr;
class InfixExpr;
class SuffixExpr;

// pre-declared ops
enum class AssignOp : short;
enum class PrefixOp : short;
enum class InfixOp : short;
enum class SuffixOp : short;
} // namespace ast

namespace parser {

/// Hand-made recursive descent parser
class Parser {
public:
    /// Expects a tree with an empty root
    /// -> see ast::Tree::get on how to call this properly
    explicit Parser(ast::Tree& tree);

    // top-level parsing function
    std::unique_ptr<ast::Root> parseRoot();

private:
    ast::Tree& tree;

    const std::string& source;
    std::vector<Token>& tokens;
    std::vector<std::unique_ptr<support::Error>>& errors;

    Tokenizer tokenizer;

    size_t tokenIndex = 0;
    bool hasTokens = false; // represents a dummy '-1' token index if false

    // parsing functions for nodes:
    std::unique_ptr<ast::Stmt> parseTopLevelDecl(bool mandatory);

    // statements:
    std::unique_ptr<ast::Stmt> parseStmt(bool mandatory);
    std::unique_ptr<ast::Block> parseBlock(bool mandatory);
    std::unique_ptr<ast::VarDecl> parseVarDecl(bool mandatory);
    std::unique_ptr<ast::ParamDecl> parseParamDecl();
    std::vector<std::unique_ptr<ast::ParamDecl>> parseParamDeclList();
    std::unique_ptr<ast::FnDecl> parseFnDecl(bool mandatory);
    std::unique_ptr<ast::Return> parseReturn(bool mandatory);
    std::unique_ptr<ast::IfStmt> parseIfStmt(bool mandatory);
    std::unique_ptr<ast::AssignStmt> parseAssignStmt(bool mandatory);

    // expressions:
    std::unique_ptr<ast::Expr> parseExpr(bool mandatory);
    std::unique_ptr<ast::GroupedExpr> parseGroupedExpr(bool mandatory);
    std::unique_ptr<ast::Identifier> parseIdentifier(bool mandatory);
    std::unique_ptr<ast::Expr> parsePrimaryExpr(bool mandatory);
    std::unique_ptr<ast::Expr> parsePrefixExpr(bool mandatory);
    std::unique_ptr<ast::Expr> parseMultExpr(bool mandatory);
    std::unique_ptr<ast::Expr> parseAddExpr(bool mandatory);
    std::unique_ptr<ast::Expr> parseShiftExpr(bool mandatory);
    std::unique_ptr<ast::Expr> parseBitExpr(bool mandatory);
    std::unique_ptr<ast::Expr> parseCompareExpr(bool mandatory);
    std::unique_ptr<ast::Expr> parseSuffixExpr(bool mandatory);

    // operations:
    ast::AssignOp parseAssignOp();
    ast::PrefixOp parsePrefixOp();
    ast::InfixOp parseMultOp();
    ast::InfixOp parseAddOp();
    ast::InfixOp parseShiftOp();
    ast::InfixOp parseBitOp();
    ast::InfixOp parseCompareOp();
    ast::SuffixOp parseSuffixOp();

    /// gets a token from the tokenizer and puts it into tokens
    void fetchToken();

    /// giving an iterator-like experience
    // (peek, next, prev) for the streaming tokenizer
    const Token& peekNextToken();
    const Token& nextToken();
    const Token& prevToken();

    /// tries to consume a token of given kind, returns nullptr on fail
    const Token* consumeToken(Token::Kind kind);

    /// tries to consume a token which is one of given kinds,
    /// returns nullptr on fail
    template <typename... Ts> const Token* consumeOneOf(Ts... kinds);

    /// Warning: this reference is only valid
    /// until any new token is added into tokens
    const Token& currentToken() { return tokens[tokenIndex]; }

    const Token& getToken(size_t i) {
        assert(i <= tokenIndex && hasTokens);
        return tokens[i];
    }

    std::string tokenToString(size_t index) const;
    uint64_t parseNumber(size_t index) const;

    // Add error at the end of the specified token
    void errorAtEnd(const std::string&& message, size_t token);

    // Add error at the start of the specified token given byindex
    void error(const std::string&& message, size_t token);

    // Add error at the start of the specified token
    void error(const std::string&& message, const Token& token);

    // Add error to specific ast::Loc
    void errorWithLoc(const std::string&& message, const ast::Loc&& loc);
};

} // namespace parser
} // namespace perun

#endif // PERUN_PARSER_PARSER_HPP
