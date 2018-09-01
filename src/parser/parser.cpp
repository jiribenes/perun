#include "parser.hpp"

#include <iostream>

#include "../ast/expr.hpp"
#include "../ast/node.hpp"
#include "../ast/stmt.hpp"

namespace perun {
namespace parser {

std::string Parser::tokenToString(size_t index) const {
    assert(index < tokens.size() &&
           "cannot convert unbuffered token into string");

    const Token& token = tokens[index];
    return source.substr(token.start, token.length());
}

uint64_t Parser::parseNumber(size_t index) const {
    const std::string str = tokenToString(index);

    // TODO: error handling
    int radix = 10;
    if (str[0] == '0' && str.size() >= 3) {
        if (str[1] == 'b') {
            radix = 2;
        } else if (str[1] == 'o') {
            radix = 8;
        } else if (str[1] == 'x') {
            radix = 16;
        }
    }

    // TODO: this is an ugly hack - could we make it better?
    const char* realStr = radix != 10 ? (str.c_str() + 2) : str.c_str();

    return std::strtoll(realStr, nullptr, radix);
}

std::unique_ptr<ast::Root> Parser::parseRoot() {
    auto root = std::make_unique<ast::Root>();

    while (true) {
        auto&& decl = parseTopLevelDecl(false);
        if (decl == nullptr) {
            break;
        }

        root->addDecl(std::move(decl));
    }

    if (consumeToken(Token::Kind::EndOfFile)) {
        root->setEOFToken(tokenIndex);
        return root;
    } else {
        error("invalid token, expected 'EOF'");
    }
}

std::unique_ptr<ast::Stmt> Parser::parseTopLevelDecl(bool mandatory) {
    auto decl = parseVarDecl(mandatory);
    if (decl != nullptr) {
        return decl;
    }

    if (!mandatory) {
        return nullptr;
    }

    error("invalid top level decl");
}

std::unique_ptr<ast::Expr> Parser::parseExpr(bool mandatory) {
    auto primExpr = parsePrimaryExpr(mandatory);
    if (primExpr != nullptr) {
        return primExpr;
    }

    if (!mandatory) {
        return nullptr;
    }

    error("invalid expr");
}

std::unique_ptr<ast::Expr> Parser::parsePrimaryExpr(bool mandatory) {
    if (consumeToken(Token::Kind::LiteralInteger) != nullptr) {
        uint64_t value = parseNumber(tokenIndex);

        return std::make_unique<ast::LiteralInteger>(value);
    } else if (consumeToken(Token::Kind::Identifier) != nullptr) {
        std::string identifier = tokenToString(tokenIndex);

        return std::make_unique<ast::Identifier>(identifier);
    } else if (consumeToken(Token::Kind::KeywordTrue) != nullptr) {
        return std::make_unique<ast::LiteralBoolean>(true);
    } else if (consumeToken(Token::Kind::KeywordFalse) != nullptr) {
        return std::make_unique<ast::LiteralBoolean>(false);
    }

    if (mandatory) {
        error("could not parse primary expr");
    } else {
        return nullptr;
    }
}

std::unique_ptr<ast::VarDecl> Parser::parseVarDecl(bool mandatory) {
    bool isConst;
    if (auto token = consumeToken(Token::Kind::KeywordVar)) {
        isConst = false;
    } else if (auto token = consumeToken(Token::Kind::KeywordConst)) {
        isConst = true;
    } else if (mandatory) {
        error("invalid token - expected 'var' or 'const'");
    } else {
        return nullptr;
    }

    std::string name;
    if (consumeToken(Token::Kind::Identifier)) {
        name = tokenToString(tokenIndex);
    } else {
        error("unexpected token - expected 'Identifier' as name");
    }

    std::unique_ptr<ast::Expr> typeExpr = nullptr;
    if (consumeToken(Token::Kind::Colon)) {
        if (consumeToken(Token::Kind::Identifier)) {
            std::string identifier = tokenToString(tokenIndex);

            typeExpr = std::make_unique<ast::Identifier>(identifier);
        } else {
            error("unexpected token - expected 'Identifier' as type");
        }
    }

    std::unique_ptr<ast::Expr> expr = nullptr;
    if (consumeToken(Token::Kind::Eq)) {
        expr = parsePrimaryExpr(true);
    }

    if (!consumeToken(Token::Kind::Semicolon)) {
        error("expected semicolon");
    }

    return std::make_unique<ast::VarDecl>(isConst, name, std::move(typeExpr),
                                          std::move(expr));
}

void Parser::fetchToken() {
    Token token = tokenizer.nextToken();

    // this possibly invalidates all ptrs/refs into tokens
    tokens.push_back(token);
}

const Token& Parser::peekNextToken() {
    if (!hasTokens) {
        assert(tokenIndex == 0);
        fetchToken();
        return tokens[tokenIndex];
    }

    if ((tokenIndex + 1) < tokens.size()) {
        return tokens[tokenIndex + 1];
    }

    // we need to stream a new token
    fetchToken();

    return tokens[tokenIndex + 1];
}

const Token& Parser::nextToken() {
    if (!hasTokens) {
        assert(tokenIndex == 0);
        if (tokens.empty()) {
            fetchToken();
        }
        hasTokens = true;
        return tokens[tokenIndex];
    }

    tokenIndex++;

    if (tokenIndex < tokens.size()) {
        return tokens[tokenIndex];
    }

    // we need to stream a new token
    fetchToken();

    assert(tokenIndex < tokens.size() && "token index is out of bounds");

    return tokens[tokenIndex];
}

const Token& Parser::prevToken() {
    assert(tokenIndex > 0 && "token index is out of bounds");

    tokenIndex--;
    return tokens[tokenIndex];
}

const Token* Parser::consumeToken(Token::Kind kind) {
    const Token& token = peekNextToken();

    if (token.is(kind)) {
        nextToken();
        return &tokens[tokenIndex];
    }

    return nullptr;
}

template <typename... Ts> const Token* Parser::consumeOneOf(Ts... kinds) {
    const Token& token = peekNextToken();

    if (token.isOneOf(kinds...)) {
        nextToken();
        return &tokens[tokenIndex];
    }

    return nullptr;
}

// TODO: better error handling
[[noreturn]] void Parser::error(const std::string& message) {
    std::cerr << "Encountered an error!" << std::endl;
    std::cerr << message << std::endl;

    exit(0);
}

} // namespace parser
} // namespace perun
