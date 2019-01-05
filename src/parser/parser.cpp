#include "parser.hpp"

#include <iostream>

#include "../ast/expr.hpp"
#include "../ast/literal.hpp"
#include "../ast/node.hpp"
#include "../ast/stmt.hpp"
#include "../ast/tree.hpp"

namespace perun {
namespace parser {

Parser::Parser(ast::Tree& tree)
    : tree(tree), source(tree.getSource()), tokens(tree.getTokensMut()),
      errors(tree.getErrorsMut()), tokenizer(tree.getSource()) {}

// Note - TODO:
// The parser now throws 42 on non-recoverable errors.
// More errors should be made recoverable and
// all other non-recoverable errors should be dealt with
// properly without resorting to exceptions

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
        errorAtEnd("invalid token, expected 'EOF'", tokenIndex);
        throw 42;
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

    error("invalid top level decl", tokenIndex);
    throw 42;
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

    error("invalid stmt", tokenIndex);
    throw 42;
}

// Block := '{' Stmt* '}'
std::unique_ptr<ast::Block> Parser::parseBlock(bool mandatory) {
    std::vector<std::unique_ptr<ast::Stmt>> stmts{};

    auto lBrace = consumeToken(Token::Kind::LBrace);
    if (lBrace == nullptr) {
        if (!mandatory) {
            return nullptr;
        }

        error("expected '{' in Block", tokenIndex);
        throw 42;
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
        if (stmt == nullptr) {
            // we couldn't parse a statement
            break;
        }

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
        error("invalid token - expected 'var' or 'const'", tokenIndex);
        throw 42;
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

    size_t semicolonToken = tokenIndex;
    if (!consumeToken(Token::Kind::Semicolon)) {
        errorAtEnd("expected semicolon after VarDecl", tokenIndex);
        // continue as if we got a semicolon
    }

    return std::make_unique<ast::VarDecl>(isConst, std::move(identifier),
                                          std::move(typeExpr), std::move(expr),
                                          varToken, semicolonToken);
}

// ParamDecl := (Identifier ':')? Type
std::unique_ptr<ast::ParamDecl> Parser::parseParamDecl() {
    auto identifier = parseIdentifier(false);
    if (identifier != nullptr) {
        if (consumeToken(Token::Kind::Colon) == nullptr) {
            error("expected colon", tokenIndex);
            throw 42;
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
        error("expected '('", tokenIndex);
        throw 42;
    }

    bool expectBreak = false;
    while (true) {
        if (consumeToken(Token::Kind::RParen)) {
            break;
        } else if (expectBreak) {
            error("expected '}' after no comma found previously in list",
                  tokenIndex);
            throw 42;
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

        error("unexpected token - expected 'fn' keyword", tokenIndex);
        throw 42;
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
        semicolonToken = tokenIndex;

        if (!consumeToken(Token::Kind::Semicolon)) {
            errorAtEnd(
                "expected semicolon after FnDecl when it is only a prototype",
                tokenIndex);
            // continue as if we got ';'
        }
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
        error("expected keyword 'return' while parsing return node",
              tokenIndex);
        throw 42;
    }

    auto&& expr = parseExpr(false);

    size_t semicolonToken = tokenIndex;
    if (consumeToken(Token::Kind::Semicolon) == nullptr) {
        errorAtEnd("expected semicolon after Return", tokenIndex);
        // continue as if we got ';'
    }

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

        error("expected 'if' in IfStmt", tokenIndex);
        throw 42;
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

// Expr := CompareExpr
std::unique_ptr<ast::Expr> Parser::parseExpr(bool mandatory) {
    auto expr = parseCompareExpr(false);
    if (expr != nullptr) {
        return expr;
    }

    if (!mandatory) {
        return nullptr;
    }

    error("invalid expr", tokenIndex);
    throw 42;
}

// GroupedExpr := '(' Expr ')'
std::unique_ptr<ast::GroupedExpr> Parser::parseGroupedExpr(bool mandatory) {
    size_t lParenToken, rParenToken;
    if (consumeToken(Token::Kind::LParen) == nullptr) {
        if (!mandatory) {
            return nullptr;
        }

        error("expected '(' in GroupedExpr", tokenIndex);
        throw 42;
    }
    lParenToken = tokenIndex;

    auto&& expr = parseExpr(true);

    if (consumeToken(Token::Kind::RParen) == nullptr) {
        errorAtEnd("expected ')' in GroupedExpr", tokenIndex);
        // continue as if we got ')'
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

    error("could not parse identifier", tokenIndex);
    throw 42;
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

    error("could not parse primary expr", tokenIndex);
    throw 42;
}

// PrefixExpr := PrefixOp PrefixExpr | SuffixExpr
std::unique_ptr<ast::Expr> Parser::parsePrefixExpr(bool mandatory) {
    auto op = parsePrefixOp();
    if (op == ast::PrefixOp::Invalid) {
        return parseSuffixExpr(mandatory);
    }

    auto&& expr = parsePrefixExpr(true);
    size_t opToken = tokenIndex;
    auto&& prefix_expr =
        std::make_unique<ast::PrefixExpr>(std::move(expr), op, opToken);

    return std::move(prefix_expr);
}

// MultExpr := PrefixExpr (MultOp PrefixExpr)*
std::unique_ptr<ast::Expr> Parser::parseMultExpr(bool mandatory) {
    std::unique_ptr<ast::Expr> expr = parsePrefixExpr(false);
    if (expr == nullptr) {
        if (!mandatory) {
            return nullptr;
        }

        error("expected PrefixExpr in MultExpr", tokenIndex);
        throw 42;
    }

    while (true) {
        auto op = parseMultOp();
        if (op == ast::InfixOp::Invalid) {
            break;
        }

        size_t opToken = tokenIndex;

        // if we parsed the MultOp correctly,
        // then the next thing must be a PrefixExpr
        auto&& rhs = parsePrefixExpr(true);

        auto&& newExpr = std::make_unique<ast::InfixExpr>(
            std::move(expr), std::move(rhs), op, opToken);

        expr = std::move(newExpr);
    }

    return expr;
}

// AddExpr := MultExpr (AddOp MultExpr)*
std::unique_ptr<ast::Expr> Parser::parseAddExpr(bool mandatory) {
    std::unique_ptr<ast::Expr> expr = parseMultExpr(false);
    if (expr == nullptr) {
        if (!mandatory) {
            return nullptr;
        }

        error("expected MultExpr in AddExpr", tokenIndex);
        throw 42;
    }

    while (true) {
        auto op = parseAddOp();
        if (op == ast::InfixOp::Invalid) {
            break;
        }

        size_t opToken = tokenIndex;

        // if we parsed the AddOp correctly,
        // then the next thing must be a MultExpr
        auto&& rhs = parseMultExpr(true);

        auto&& newExpr = std::make_unique<ast::InfixExpr>(
            std::move(expr), std::move(rhs), op, opToken);

        expr = std::move(newExpr);
    }

    return expr;
}

// ShiftExpr := AddExpr (ShiftOp AddExpr)*
std::unique_ptr<ast::Expr> Parser::parseShiftExpr(bool mandatory) {
    std::unique_ptr<ast::Expr> expr = parseAddExpr(false);
    if (expr == nullptr) {
        if (!mandatory) {
            return nullptr;
        }

        error("expected AddExpr in ShiftExpr", tokenIndex);
        throw 42;
    }

    while (true) {
        auto op = parseShiftOp();
        if (op == ast::InfixOp::Invalid) {
            break;
        }

        size_t opToken = tokenIndex;

        // if we parsed the ShiftOp correctly,
        // then the next thing must be a AddExpr
        auto&& rhs = parseAddExpr(true);

        auto&& newExpr = std::make_unique<ast::InfixExpr>(
            std::move(expr), std::move(rhs), op, opToken);

        expr = std::move(newExpr);
    }

    return expr;
}

// BitExpr := ShiftExpr (BitOp ShiftExpr)*
std::unique_ptr<ast::Expr> Parser::parseBitExpr(bool mandatory) {
    std::unique_ptr<ast::Expr> expr = parseShiftExpr(false);
    if (expr == nullptr) {
        if (!mandatory) {
            return nullptr;
        }

        error("expected ShiftExpr in BitExpr", tokenIndex);
        throw 42;
    }

    while (true) {
        auto op = parseBitOp();
        if (op == ast::InfixOp::Invalid) {
            break;
        }

        size_t opToken = tokenIndex;

        // if we parsed the BitOp correctly,
        // then the next thing must be a ShiftExpr
        auto&& rhs = parseShiftExpr(true);

        auto&& newExpr = std::make_unique<ast::InfixExpr>(
            std::move(expr), std::move(rhs), op, opToken);

        expr = std::move(newExpr);
    }

    return expr;
}

// CompareExpr := BitExpr (CompareOp BitExpr)*
std::unique_ptr<ast::Expr> Parser::parseCompareExpr(bool mandatory) {
    std::unique_ptr<ast::Expr> expr = parseBitExpr(false);
    if (expr == nullptr) {
        if (!mandatory) {
            return nullptr;
        }

        error("expected BitExpr in CompareExpr", tokenIndex);
        throw 42;
    }

    while (true) {
        auto op = parseCompareOp();
        if (op == ast::InfixOp::Invalid) {
            break;
        }

        size_t opToken = tokenIndex;

        // if we parsed the CompareOp correctly,
        // then the next thing must be a BitExpr
        auto&& rhs = parseBitExpr(true);

        auto&& newExpr = std::make_unique<ast::InfixExpr>(
            std::move(expr), std::move(rhs), op, opToken);

        expr = std::move(newExpr);
    }

    return expr;
}

// SuffixExpr := PrimExpr (SuffixOp | FnCall)*
std::unique_ptr<ast::Expr> Parser::parseSuffixExpr(bool mandatory) {
    std::unique_ptr<ast::Expr> expr = parsePrimaryExpr(false);
    if (expr == nullptr) {
        if (!mandatory) {
            return nullptr;
        }

        error("expected PrimExpr in SuffixExpr", tokenIndex);
        throw 42;
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

// PrefixOp := '&' | '~' | '!' | '-' | '?'
ast::PrefixOp Parser::parsePrefixOp() {
    auto token = consumeOneOf(Token::Kind::Ampersand, Token::Kind::Tilde,
                              Token::Kind::Bang, Token::Kind::Minus,
                              Token::Kind::Question);
    if (token == nullptr) {
        return ast::PrefixOp::Invalid;
    }

    switch (token->getKind()) {
    case Token::Kind::Ampersand: {
        return ast::PrefixOp::Address;
    }
    case Token::Kind::Tilde: {
        return ast::PrefixOp::BitNot;
    }
    case Token::Kind::Bang: {
        return ast::PrefixOp::BoolNot;
    }
    case Token::Kind::Minus: {
        return ast::PrefixOp::Negate;
    }
    case Token::Kind::Question: {
        return ast::PrefixOp::OptionalType;
    }
    default: { assert(false); }
    }
}

// MultOp := '/' | '%' | '*'
ast::InfixOp Parser::parseMultOp() {
    auto token = consumeOneOf(Token::Kind::Slash, Token::Kind::Percent,
                              Token::Kind::Star);
    if (token == nullptr) {
        return ast::InfixOp::Invalid;
    }

    switch (token->getKind()) {
    case Token::Kind::Slash: {
        return ast::InfixOp::Div;
    }
    case Token::Kind::Percent: {
        return ast::InfixOp::Mod;
    }
    case Token::Kind::Star: {
        return ast::InfixOp::Mul;
    }
    default: { assert(false); }
    }
}

// AddOp := '+' | '-'
ast::InfixOp Parser::parseAddOp() {
    auto token = consumeOneOf(Token::Kind::Plus, Token::Kind::Minus);
    if (token == nullptr) {
        return ast::InfixOp::Invalid;
    }

    switch (token->getKind()) {
    case Token::Kind::Plus: {
        return ast::InfixOp::Add;
    }
    case Token::Kind::Minus: {
        return ast::InfixOp::Sub;
    }
    default: { assert(false); }
    }
}

// ShiftOp := '>>' | '<<'
ast::InfixOp Parser::parseShiftOp() {
    auto token =
        consumeOneOf(Token::Kind::GreaterGreater, Token::Kind::LessLess);
    if (token == nullptr) {
        return ast::InfixOp::Invalid;
    }

    switch (token->getKind()) {
    case Token::Kind::GreaterGreater: {
        return ast::InfixOp::BitSHR;
    }
    case Token::Kind::LessLess: {
        return ast::InfixOp::BitSHL;
    }
    default: { assert(false); }
    }
}

// BitOp := '&' | '|'
ast::InfixOp Parser::parseBitOp() {
    auto token = consumeOneOf(Token::Kind::Ampersand, Token::Kind::Pipe);
    if (token == nullptr) {
        return ast::InfixOp::Invalid;
    }

    switch (token->getKind()) {
    case Token::Kind::Ampersand: {
        return ast::InfixOp::BitAnd;
    }
    case Token::Kind::Pipe: {
        return ast::InfixOp::BitOr;
    }
    default: { assert(false); }
    }
}

// CompareOp := '==' | '>' | '>=' | '<' | '<=' | '!='
ast::InfixOp Parser::parseCompareOp() {
    auto token = consumeOneOf(Token::Kind::EqEq, Token::Kind::Greater,
                              Token::Kind::GreaterEq, Token::Kind::Less,
                              Token::Kind::LessEq, Token::Kind::BangEq);
    if (token == nullptr) {
        return ast::InfixOp::Invalid;
    }

    switch (token->getKind()) {
    case Token::Kind::EqEq: {
        return ast::InfixOp::EqualEqual;
    }
    case Token::Kind::Greater: {
        return ast::InfixOp::Greater;
    }
    case Token::Kind::GreaterEq: {
        return ast::InfixOp::GreaterEqual;
    }
    case Token::Kind::Less: {
        return ast::InfixOp::Less;
    }
    case Token::Kind::LessEq: {
        return ast::InfixOp::LessEqual;
    }
    case Token::Kind::BangEq: {
        return ast::InfixOp::NotEqual;
    }
    default: { assert(false); }
    }
}

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

    while (token.isOneOf(Token::Kind::LineComment, Token::Kind::DocComment)) {
        // skip all line comments and doc comments
        // TODO: parse doc comments as a part of the AST
        //       attached to the node they belong to
        token = tokenizer.nextToken();
    }

    if (token.isNot(Token::Kind::Invalid)) {
        // this possibly invalidates all ptrs/refs into tokens
        tokens.push_back(token);
        return;
    }

    // tokenizer had an error
    if (!tokenizer.getError().empty()) {
        std::string errorString = tokenizer.getError();
        error(std::move(errorString), token);
        throw 42;
    }

    // tokenizer produced a bad token
    error("tokenizer produced an invalid token", tokenIndex);
    throw 42;
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

void Parser::errorAtEnd(const std::string&& message, size_t token) {
    auto&& tok = getToken(token);
    size_t endPos = tok.end;
    ast::Loc loc = tree.getLocFromPos(endPos);
    errorWithLoc(std::move(message), std::move(loc));
}

void Parser::errorPos(const std::string&& message, size_t pos) {
    ast::Loc loc = tree.getLocFromPos(pos);
    errorWithLoc(std::move(message), std::move(loc));
}

void Parser::error(const std::string&& message, size_t token) {
    error(std::move(message), getToken(token));
}

void Parser::error(const std::string&& message, const Token& token) {
    ast::Loc loc = tree.getLocFromToken(token);
    errorWithLoc(std::move(message), std::move(loc));
}

void Parser::errorWithLoc(const std::string&& message, const ast::Loc&& loc) {
    const std::string sourceLine =
        source.substr(loc.line_start_pos, loc.lineLength());
    const std::string filenameCopy = tree.getFilename();
    errors.push_back(std::make_unique<ParseError>(
        std::move(filenameCopy), std::move(loc), std::move(message),
        std::move(sourceLine)));
}

} // namespace parser
} // namespace perun
