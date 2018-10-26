#ifndef PERUN_PARSER_PARSER_HPP
#define PERUN_PARSER_PARSER_HPP

#include <memory>

#include "token.hpp"
#include "tokenizer.hpp"

namespace perun {

// pre-declared as opaque to avoid unnecessary include
namespace ast {
class Root;

class Stmt;
class Block;

class VarDecl;
class ParamDecl;
class FnDecl;
class Return;

class Expr;
class GroupedExpr;
class Identifier;
} // namespace ast

namespace parser {

/// Hand-made recursive descent parser
class Parser {
public:
    explicit Parser(const std::string& source)
        : source(source), tokenizer(source) {}

    // parsing functions for nodes
    std::unique_ptr<ast::Root> parseRoot();
    std::unique_ptr<ast::Stmt> parseTopLevelDecl(bool mandatory);

    std::unique_ptr<ast::Expr> parseExpr(bool mandatory);
    std::unique_ptr<ast::GroupedExpr> parseGroupedExpr(bool mandatory);
    std::unique_ptr<ast::Identifier> parseIdentifier(bool mandatory);
    std::unique_ptr<ast::Expr> parsePrimaryExpr(bool mandatory);

    std::unique_ptr<ast::Stmt> parseStmt(bool mandatory);
    std::unique_ptr<ast::Return> parseReturn(bool mandatory);
    std::unique_ptr<ast::VarDecl> parseVarDecl(bool mandatory);
    std::unique_ptr<ast::ParamDecl> parseParamDecl();
    std::unique_ptr<ast::FnDecl> parseFnDecl(bool mandatory);
    std::unique_ptr<ast::Block> parseBlock(bool mandatory);

private:
    const std::string& source;
    Tokenizer tokenizer;

    std::vector<Token> tokens{};

    size_t tokenIndex = 0;
    bool hasTokens = false; // represents a dummy '-1' token index if false

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

    std::string tokenToString(size_t index) const;
    uint64_t parseNumber(size_t index) const;

    std::vector<std::unique_ptr<ast::ParamDecl>> parseParamDeclList();

    [[noreturn]] void error(const std::string& message);
};

} // namespace parser
} // namespace perun

#endif // PERUN_PARSER_PARSER_HPP
