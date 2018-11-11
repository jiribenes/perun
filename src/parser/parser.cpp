#include "parser.hpp"

#include <iostream>

#include "../ast/expr.hpp"
#include "../ast/node.hpp"
#include "../ast/stmt.hpp"
#include "../ast/literal.hpp"

namespace perun {
namespace parser {

// Root := TLD* EOF
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

// TLD := VarDecl | FnDecl
std::unique_ptr<ast::Stmt> Parser::parseTopLevelDecl(bool mandatory) {
    auto varDecl = parseVarDecl(false);
    if (varDecl != nullptr) {
        return varDecl;
    }

    auto fnDecl = parseFnDecl(false);
    if (fnDecl != nullptr) {
        return fnDecl;
    }

    if (!mandatory) {
        return nullptr;
    }

    error("invalid top level decl");
}

// statements:

// Stmt := Return | IfStmt | VarDecl
std::unique_ptr<ast::Stmt> Parser::parseStmt(bool mandatory) {
    auto returnStmt = parseReturn(false);
    if (returnStmt != nullptr) {
        return returnStmt;
    }

    auto ifStmt = parseIfStmt(false);
    if (ifStmt != nullptr) {
        return ifStmt;
    }

    auto varDecl = parseVarDecl(false);
    if (varDecl != nullptr) {
        return varDecl;
    }

    if (!mandatory) {
        return nullptr;
    }

    error("invalid stmt");
}

// Block := '{' Stmt* '}'
std::unique_ptr<ast::Block> Parser::parseBlock(bool mandatory) {
    std::vector<std::unique_ptr<ast::Stmt>> stmts{};

    auto lBrace = consumeToken(Token::Kind::LBrace);
    if (lBrace == nullptr) {
        if (!mandatory) {
            return nullptr;
        }

        error("expected '{' in Block");
    }

    size_t lBraceIndex = tokenIndex;
    size_t rBraceIndex = 0;
    while (true) {
        auto rBrace = consumeToken(Token::Kind::RBrace);
        if (rBrace != nullptr) {
            rBraceIndex = tokenIndex;
            break;
        }

        auto stmt = parseStmt(true);
        stmts.push_back(std::move(stmt));
    }

    return std::make_unique<ast::Block>(lBraceIndex, rBraceIndex,
                                        std::move(stmts));
}

// VarDecl := ('var' | 'const') Identifier (: Type)? '=' Expr ';'
std::unique_ptr<ast::VarDecl> Parser::parseVarDecl(bool mandatory) {
    bool isConst;
    if (consumeToken(Token::Kind::KeywordVar)) {
        isConst = false;
    } else if (consumeToken(Token::Kind::KeywordConst)) {
        isConst = true;
    } else if (mandatory) {
        error("invalid token - expected 'var' or 'const'");
    } else {
        return nullptr;
    }

    size_t varToken = tokenIndex;

    auto identifier = parseIdentifier(true);

    std::unique_ptr<ast::Expr> typeExpr = nullptr;
    if (consumeToken(Token::Kind::Colon)) {
        typeExpr = parseExpr(true);
    }

    std::unique_ptr<ast::Expr> expr = nullptr;
    if (consumeToken(Token::Kind::Eq)) {
        expr = parseExpr(true);
    }

    if (!consumeToken(Token::Kind::Semicolon)) {
        error("expected semicolon");
    }

    size_t semicolonToken = tokenIndex;

    return std::make_unique<ast::VarDecl>(isConst, std::move(identifier),
                                          std::move(typeExpr), std::move(expr),
                                          varToken, semicolonToken);
}

// ParamDecl := (Identifier ':')? Type
std::unique_ptr<ast::ParamDecl> Parser::parseParamDecl() {
    auto identifier = parseIdentifier(false);
    if (identifier != nullptr) {
        if (consumeToken(Token::Kind::Colon) == nullptr) {
            error("expected colon");
        }
    }

    auto typeExpr = parseExpr(true);

    return std::make_unique<ast::ParamDecl>(std::move(identifier),
                                            std::move(typeExpr));
}

// ParamDeclList := '(' (ParamDecl ',')* ParamDecl? ')'
std::vector<std::unique_ptr<ast::ParamDecl>> Parser::parseParamDeclList() {
    std::vector<std::unique_ptr<ast::ParamDecl>> params{};

    if (!consumeToken(Token::Kind::LParen)) {
        error("expected '('");
    }

    bool expectBreak = false;
    while (true) {
        if (consumeToken(Token::Kind::RParen)) {
            break;
        } else if (expectBreak) {
            error("expected '}' after no comma found previously in list");
        }

        auto param = parseParamDecl();
        params.push_back(std::move(param));

        if (!consumeToken(Token::Kind::Comma)) {
            expectBreak = true;
        }
    }

    return params;
}

// FnDecl := 'pub'? ('extern' | 'export')? 'fn' Identifier?
//           ParamDeclList ('->' Type)? (Block | ';')
std::unique_ptr<ast::FnDecl> Parser::parseFnDecl(bool mandatory) {
    size_t pubToken, modifierToken, fnToken, semicolonToken;
    bool pub = false;
    if (consumeToken(Token::Kind::KeywordPub)) {
        pub = true;
        pubToken = tokenIndex;
    }

    bool _extern = false;
    bool _export = false;
    if (consumeToken(Token::Kind::KeywordExtern)) {
        _extern = true;
        modifierToken = tokenIndex;
    } else if (consumeToken(Token::Kind::KeywordExport)) {
        _export = true;
        modifierToken = tokenIndex;
    }

    if (!consumeToken(Token::Kind::KeywordFn)) {
        if (!mandatory) {
            return nullptr;
        }

        error("unexpected token - expected 'fn' keyword");
    }
    fnToken = tokenIndex;

    auto identifier = parseIdentifier(true);

    auto params = parseParamDeclList();

    std::unique_ptr<ast::Expr> returnType = nullptr;
    if (consumeToken(Token::Kind::MinusGreater)) {
        returnType = parseExpr(true);
    }

    auto body = parseBlock(false);

    if (body == nullptr) { // empty body
        if (!consumeToken(Token::Kind::Semicolon)) {
            error("expected semicolon");
        }

        semicolonToken = tokenIndex;
    }

    return std::make_unique<ast::FnDecl>(
        std::move(identifier), std::move(params), std::move(returnType),
        std::move(body), pub, _extern, _export, fnToken, pubToken,
        modifierToken, semicolonToken);
}

// Return := 'return' Expr? ';'
std::unique_ptr<ast::Return> Parser::parseReturn(bool mandatory) {
    size_t returnToken;

    if (consumeToken(Token::Kind::KeywordReturn) != nullptr) {
        returnToken = tokenIndex;
    } else if (!mandatory) {
        return nullptr;
    } else {
        error("expected keyword 'return' while parsing return node");
    }

    auto&& expr = parseExpr(false);

    if (consumeToken(Token::Kind::Semicolon) == nullptr) {
        error("expected semicolon");
    }
    size_t semicolonToken = tokenIndex;

    return std::make_unique<ast::Return>(std::move(expr), returnToken,
                                         semicolonToken);
}

// IfStmt := 'if' Expr Block ('else' Block)?
std::unique_ptr<ast::IfStmt> Parser::parseIfStmt(bool mandatory) {
    size_t ifToken, elseToken;

    if (consumeToken(Token::Kind::KeywordIf) == nullptr) {
        if (!mandatory) {
            return nullptr;
        }

        error("expected 'if' in IfStmt");
    }

    ifToken = tokenIndex;

    auto&& expr = parseExpr(true);
    auto&& then = parseBlock(true);

    if (consumeToken(Token::Kind::KeywordElse) == nullptr) {
        elseToken = tokenIndex;
        return std::make_unique<ast::IfStmt>(std::move(expr), std::move(then),
                                             /* otherwise = */ nullptr, ifToken,
                                             elseToken);
    }

    auto&& otherwise = parseBlock(true);
    return std::make_unique<ast::IfStmt>(std::move(expr), std::move(then),
                                         std::move(otherwise), ifToken,
                                         elseToken);
}

// expressions:

// Expr := PrimaryExpr
std::unique_ptr<ast::Expr> Parser::parseExpr(bool mandatory) {
    auto suffixExpr = parseSuffixExpr(false);
    if (suffixExpr != nullptr) {
        return suffixExpr;
    }

    if (!mandatory) {
        return nullptr;
    }

    error("invalid expr");
}

// GroupedExpr := '(' Expr ')'
std::unique_ptr<ast::GroupedExpr> Parser::parseGroupedExpr(bool mandatory) {
    size_t lParenToken, rParenToken;
    if (consumeToken(Token::Kind::LParen) == nullptr) {
        if (!mandatory) {
            return nullptr;
        }

        error("expected '(' in GroupedExpr");
    }
    lParenToken = tokenIndex;

    auto&& expr = parseExpr(true);

    if (consumeToken(Token::Kind::RParen) == nullptr) {
        error("expected ')' in GroupedExpr");
    }
    rParenToken = tokenIndex;

    return std::make_unique<ast::GroupedExpr>(std::move(expr), lParenToken,
                                              rParenToken);
}

std::unique_ptr<ast::Identifier> Parser::parseIdentifier(bool mandatory) {
    if (consumeToken(Token::Kind::Identifier) != nullptr) {
        std::string identifier = tokenToString(tokenIndex);

        return std::make_unique<ast::Identifier>(identifier, tokenIndex);
    }

    if (!mandatory) {
        return nullptr;
    }

    error("could not parse identifier");
}

// PrimaryExpr := Integer | 'true' | 'false' | 'nil' | 'undefined'
//              | GroupedExpr | Identifier
std::unique_ptr<ast::Expr> Parser::parsePrimaryExpr(bool mandatory) {
    if (consumeToken(Token::Kind::LiteralInteger) != nullptr) {
        uint64_t value = parseNumber(tokenIndex);

        return std::make_unique<ast::LiteralInteger>(value, tokenIndex);
    } else if (consumeToken(Token::Kind::KeywordTrue) != nullptr) {
        return std::make_unique<ast::LiteralBoolean>(true, tokenIndex);
    } else if (consumeToken(Token::Kind::KeywordFalse) != nullptr) {
        return std::make_unique<ast::LiteralBoolean>(false, tokenIndex);
    } else if (consumeToken(Token::Kind::KeywordNil) != nullptr) {
        return std::make_unique<ast::LiteralNil>(tokenIndex);
    } else if (consumeToken(Token::Kind::KeywordUndefined) != nullptr) {
        return std::make_unique<ast::LiteralUndefined>(tokenIndex);
    }

    auto grouped = parseGroupedExpr(false);
    if (grouped != nullptr) {
        return grouped;
    }

    auto identifier = parseIdentifier(false);
    if (identifier != nullptr) {
        return identifier;
    }

    if (!mandatory) {
        return nullptr;
    }
    error("could not parse primary expr");
}

// PrefixExpr := PrefixOp PrefixExpr | SuffixExpr
std::unique_ptr<ast::Expr> Parser::parsePrefixExpr(bool mandatory) {
    // TODO
    assert(false && "Not implemented yet!");
}

// InfixExpr := TODO
std::unique_ptr<ast::Expr> Parser::parseInfixExpr(bool mandatory) {
    assert(false && "Not implemented yet!");
}

// SuffixExpr := PrimExpr (SuffixOp | FnCall)*
std::unique_ptr<ast::Expr> Parser::parseSuffixExpr(bool mandatory) {
    std::unique_ptr<ast::Expr> expr = parsePrimaryExpr(false);
    if (expr == nullptr) {
        if (!mandatory) {
            return nullptr;
        }

        error("expected PrimExpr in SuffixExpr");
    }

    while (true) {
        // either a suffix op
        auto op = parseSuffixOp();
        if (op != ast::SuffixOp::Invalid) {
            size_t opToken = tokenIndex;
            auto&& newExpr =
                std::make_unique<ast::SuffixExpr>(std::move(expr), op, opToken);
            expr = std::move(newExpr);
            continue;
        }

        // TODO: add function call, array access, slice, member access
        break;
    }

    return expr;
}

// PrefixOp := TODO
ast::PrefixOp Parser::parsePrefixOp() {
    assert(false && "Not implemented yet!");
}

// InfixOp := TODO
ast::InfixOp Parser::parseInfixOp() { assert(false && "Not implemented yet!"); }

// SuffixOp := '^' | '?'
ast::SuffixOp Parser::parseSuffixOp() {
    auto token = consumeOneOf(Token::Kind::Caret, Token::Kind::Question);
    if (token == nullptr) {
        return ast::SuffixOp::Invalid;
    }

    switch (token->getKind()) {
    case Token::Kind::Caret: {
        return ast::SuffixOp::Deref;
    }
    case Token::Kind::Question: {
        return ast::SuffixOp::Unwrap;
    }
    default: { assert(false); }
    }
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

// helper functions
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

// TODO: better error handling
[[noreturn]] void Parser::error(const std::string& message) {
    std::cerr << "Encountered an error!" << std::endl;
    std::cerr << message << std::endl;

    exit(0);
}

} // namespace parser
} // namespace perun
