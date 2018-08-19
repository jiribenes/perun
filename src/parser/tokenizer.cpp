#include "tokenizer.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>

/// Utility char functions
namespace {
inline bool isIdentifier(const char c) {
    return c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

inline bool isNonzeroNumeric(const char c) { return c >= '1' && c <= '9'; }

inline bool isNumeric(const char c) { return c >= '0' && c <= '9'; }

} // namespace

namespace perun {
namespace parser {

Tokenizer::Tokenizer(std::unique_ptr<std::string> input, size_t pos)
    : input(std::move(input)), state(State::Invalid), pos(pos) {}

Token Tokenizer::nextToken() {
    state = State::Start;

    // every token is implicitly end-of-file in the beginning
    Token token = Token(Token::Kind::EndOfFile, pos);

    // complete is for avoiding goto
    // - indicates when a token is complete so we can stop this loop
    for (bool complete = false; pos < input->size() && !complete; pos++) {
        // current char
        const char c = input->at(pos);

        switch (state) {
        case State::Start: {
            switch (c) {
            case ' ':
            case '\t':
            case '\n': {
                // skip whitespace
                token.start = pos + 1;
                break;
            }
            case '0': {
                state = State::Zero;
                token.setKind(Token::Kind::LiteralInteger);
                break;
            }
            case 'c': {
                state = State::C;
                token.setKind(Token::Kind::Identifier);
                break;
            }
            case '"': {
                state = State::String;
                token.setKind(Token::Kind::LiteralString);
                break;
            }
            case '`': {
                state = State::RawString;
                token.setKind(Token::Kind::LiteralRawString);
                break;
            }
            case '(': {
                token.setKind(Token::Kind::LParen);
                pos++;
                complete = true;
                break;
            }
            case ')': {
                token.setKind(Token::Kind::RParen);
                pos++;
                complete = true;
                break;
            }
            case '{': {
                token.setKind(Token::Kind::LBrace);
                pos++;
                complete = true;
                break;
            }
            case '}': {
                token.setKind(Token::Kind::RBrace);
                pos++;
                complete = true;
                break;
            }
            case '[': {
                token.setKind(Token::Kind::LBracket);
                pos++;
                complete = true;
                break;
            }
            case ']': {
                token.setKind(Token::Kind::RBracket);
                pos++;
                complete = true;
                break;
            }
            case '&': {
                state = State::Ampersand;
                break;
            }
            case '!': {
                state = State::Bang;
                break;
            }
            case '.': {
                state = State::Dot;
                break;
            }
            default: {
                if (isNonzeroNumeric(c)) {
                    state = State::Integer;
                    token.setKind(Token::Kind::LiteralInteger);
                } else if (isIdentifier(c)) {
                    state = State::Identifier;
                    token.setKind(Token::Kind::Identifier);
                } else {
                    token.setKind(Token::Kind::Invalid);
                    complete = true;
                }
                break;
            }
            }
            break;
        }
        case State::Zero: {
            // TODO: handle 0b/0x/0o radixes
            switch (c) {
            default: {
                pos--;
                state = State::Integer;
                break;
            }
            }
            break;
        }
        case State::C: {
            switch (c) {
            case '"': {
                state = State::String;
                token.setKind(Token::Kind::LiteralCString);
                break;
            }
            case '`': {
                state = State::RawString;
                token.setKind(Token::Kind::LiteralCRawString);
                break;
            }
            default: {
                if (isIdentifier(c)) {
                    // c is followed by an identifier char =>
                    // c belongs to the identifier itself
                    state = State::Identifier;
                } else {
                    complete = true;
                }
                break;
            }
            }
            break;
        }
        case State::String: {
            switch (c) {
            case '"': {
                pos++;
                complete = true;
                break;
            }
            case '\n': {
                error("newline is not allowed in a string!");
            }
            case '\\': {
                state = State::StringEscape;
                break;
            }
            default: {
                // we add this char into the string
                break;
            }
            }
            break;
        }
        case State::StringEscape: {
            // TODO: handle escapes properly
            switch (c) {
            case '\n': {
                error("newline is not allowed in a string!");
            }
            default: {
                state = State::String;
                break;
            }
            }
            break;
        }
        case State::RawString: {
            switch (c) {
            case '`': {
                pos++;
                complete = true;
                break;
            }
            default: {
                // we add this char into the raw string
                break;
            }
            }
            break;
        }
        case State::Ampersand: {
            switch (c) {
            case '=': { // &=
                token.setKind(Token::Kind::AmpersandEq);
                pos++;
                complete = true;
                break;
            }
            default: {
                // backtrack, went too far
                token.setKind(Token::Kind::Ampersand);
                complete = true;
                break;
            }
            }
            break;
        }
        case State::Bang: {
            switch (c) {
            case '=': { // !=
                token.setKind(Token::Kind::BangEq);
                pos++;
                complete = true;
                break;
            }
            default: {
                // backtrack, went too far
                token.setKind(Token::Kind::Bang);
                complete = true;
                break;
            }
            }
            break;
        }
        case State::Dot: {
            switch (c) {
            case '.': { // .. or ...
                state = State::DotDot;
                break;
            }
            default: { // .
                // backtrack, went too far
                token.setKind(Token::Kind::Dot);
                complete = true;
                break;
            }
            }
            break;
        }
        case State::DotDot: {
            switch (c) {
            case '.': { // ...
                token.setKind(Token::Kind::DotDotDot);
                pos++;
                complete = true;
                break;
            }
            default: { // ..
                // backtrack, went too far
                token.setKind(Token::Kind::Dot);
                complete = true;
                break;
            }
            }
            break;
        }
        case State::Identifier: {
            if (isIdentifier(c)) {
                // add to the identifier
                break;
            }

            // TODO: figure out a way to make a slice / a reference to input
            const std::string& buffer =
                input->substr(token.start, pos - token.start);
            const auto keywordKind = getKeyword(buffer);

            if (keywordKind != Token::Kind::Invalid) {
                token.setKind(keywordKind);
            }

            complete = true;
            break;
        }
        case State::Integer: {
            if (isNumeric(c) || c == '_') {
                break;
            }

            complete = true;
            break;
        }
        case State::Invalid: {
            assert(false);
        }
        }
    }

    if (pos == input->size()) {
        // finalize tokens
        switch (state) {

        case State::Start:
        case State::C:
        case State::String:
        case State::RawString:
        case State::Integer: {
            // these are fine without finalizing
            break;
        }

        // truly finalizing cases
        case State::Zero: {
            token.setKind(Token::Kind::LiteralInteger);
            break;
        }
        case State::Ampersand: {
            token.setKind(Token::Kind::Ampersand);
            break;
        }
        case State::Bang: {
            token.setKind(Token::Kind::Bang);
            break;
        }
        case State::Dot: {
            token.setKind(Token::Kind::Dot);
            break;
        }
        case State::DotDot: {
            token.setKind(Token::Kind::DotDot);
            break;
        }
        case State::Identifier: {
            // TODO: deduplicate this?
            // TODO: figure out a way to make a slice / a reference to input
            const std::string& buffer =
                input->substr(token.start, pos - token.start);
            const auto keywordKind = getKeyword(buffer);

            if (keywordKind != Token::Kind::Invalid) {
                token.setKind(keywordKind);
            }
            break;
        }

        // error cases
        case State::StringEscape: {
            error("trailing escape in string!");
        }
        case State::Invalid: {
            assert(false);
        }
        }
    }

    token.end = pos;
    return token;
}

// TODO: better error handling
[[noreturn]] void Tokenizer::error(const std::string& message) {
    std::cerr << "Encountered an error!" << std::endl;
    std::cerr << message << std::endl;

    exit(0);
}

} // namespace parser
} // namespace perun
