#ifndef PERUN_PARSER_PARSER_HPP
#define PERUN_PARSER_PARSER_HPP

#include <memory>

#include "token.hpp"
#include "tokenizer.hpp"

namespace perun {

// pre-declared as opaque to avoid unnecessary include
namespace ast {
class Expr;
} // namespace ast

namespace parser {

/// Hand-made recursive descent parser
class Parser {
public:
    explicit Parser(const std::string& source)
        : source(source), tokenizer(source) {}

    // parsing functions for nodes
    std::unique_ptr<ast::Expr> parsePrimaryExpr(bool mandatory);

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

    /// tries to consume a token which is one of given kinds, returns nullptr on
    /// fail
    template <typename... Ts> const Token* consumeOneOf(Ts... kinds);

    /// Warning: this reference is only valid
    /// until any new token is added into tokens
    const Token& currentToken() { return tokens[tokenIndex]; }

    std::string tokenToString(size_t index) const;
    uint64_t parseNumber(size_t index) const;

    [[noreturn]] void error(const std::string& message);
};

} // namespace parser
} // namespace perun

#endif // PERUN_PARSER_PARSER_HPP