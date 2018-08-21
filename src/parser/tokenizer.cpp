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
    bool complete = false;
    while (pos < input->size() && !complete) {
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
            // loosely follows the order of Token::Kind
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
            case '@': {
                token.setKind(Token::Kind::At);
                pos++;
                complete = true;
                break;
            }
            case '\\': {
                token.setKind(Token::Kind::Backslash);
                pos++;
                complete = true;
                break;
            }
            case '!': {
                state = State::Bang;
                break;
            }
            case '^': {
                token.setKind(Token::Kind::Caret);
                pos++;
                complete = true;
                break;
            }
            case ':': {
                token.setKind(Token::Kind::Colon);
                pos++;
                complete = true;
                break;
            }
            case ',': {
                token.setKind(Token::Kind::Comma);
                pos++;
                complete = true;
                break;
            }
            case '.': {
                state = State::Dot;
                break;
            }
            case '=': {
                state = State::Eq;
                break;
            }
            case '>': {
                state = State::Greater;
                break;
            }
            case '#': {
                token.setKind(Token::Kind::Hash);
                pos++;
                complete = true;
                break;
            }
            case '<': {
                state = State::Less;
                break;
            }
            case '-': {
                state = State::Minus;
                break;
            }
            case '%': {
                state = State::Percent;
                break;
            }
            case '|': {
                state = State::Pipe;
                break;
            }
            case '+': {
                state = State::Plus;
                break;
            }
            case '?': {
                state = State::Question;
                break;
            }
            case ';': {
                token.setKind(Token::Kind::Semicolon);
                pos++;
                complete = true;
                break;
            }
            case '/': {
                state = State::Slash;
                break;
            }
            case '*': {
                state = State::Star;
                break;
            }
            case '~': {
                state = State::Tilde;
                break;
            }
            default: {
                if (isNonzeroNumeric(c)) { // 1..9
                    state = State::Integer;
                    token.setKind(Token::Kind::LiteralInteger);
                } else if (isIdentifier(c)) { // 'a'..'z' | 'A'..'Z' | '_'
                    state = State::Identifier;
                    token.setKind(Token::Kind::Identifier);
                } else {
                    token.setKind(Token::Kind::Invalid);
                    pos++;
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
                if (isIdentifier(c) || isNumeric(c)) { // alphanumeric | '_'
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
                token.setKind(Token::Kind::DotDot);
                complete = true;
                break;
            }
            }
            break;
        }
        case State::Eq: {
            switch (c) {
            case '=': { // ==
                token.setKind(Token::Kind::EqEq);
                pos++;
                complete = true;
                break;
            }
            case '>': { // =>
                token.setKind(Token::Kind::EqGreater);
                pos++;
                complete = true;
                break;
            }
            default: { // =
                // backtrack, went too far
                token.setKind(Token::Kind::Eq);
                complete = true;
                break;
            }
            }
            break;
        }
        case State::Greater: {
            switch (c) {
            case '=': { // >=
                token.setKind(Token::Kind::GreaterEq);
                pos++;
                complete = true;
                break;
            }
            case '>': { // >> or >>=
                state = State::GreaterGreater;
                break;
            }
            default: { // >
                // backtrack, went too far
                token.setKind(Token::Kind::Greater);
                complete = true;
                break;
            }
            }
            break;
        }
        case State::GreaterGreater: {
            switch (c) {
            case '>': { // >>=
                token.setKind(Token::Kind::GreaterGreaterEq);
                pos++;
                complete = true;
                break;
            }
            default: { // >>
                // backtrack, went too far
                token.setKind(Token::Kind::GreaterGreater);
                complete = true;
                break;
            }
            }
            break;
        }
        case State::Less: {
            switch (c) {
            case '=': { // <=
                token.setKind(Token::Kind::LessEq);
                pos++;
                complete = true;
                break;
            }
            case '<': { // << or <<=
                state = State::LessLess;
                break;
            }
            default: { // <
                // backtrack, went too far
                token.setKind(Token::Kind::Less);
                complete = true;
                break;
            }
            }
            break;
        }
        case State::LessLess: {
            switch (c) {
            case '<': { // <<=
                token.setKind(Token::Kind::LessLessEq);
                pos++;
                complete = true;
                break;
            }
            default: { // <<
                // backtrack, went too far
                token.setKind(Token::Kind::LessLess);
                complete = true;
                break;
            }
            }
            break;
        }
        case State::Minus: {
            switch (c) {
            case '=': { // -=
                token.setKind(Token::Kind::MinusEq);
                pos++;
                complete = true;
                break;
            }
            case '>': { // ->
                token.setKind(Token::Kind::MinusGreater);
                pos++;
                complete = true;
                break;
            }
            default: { // -
                // backtrack, went too far
                token.setKind(Token::Kind::Minus);
                complete = true;
                break;
            }
            }
            break;
        }
        case State::Percent: {
            switch (c) {
            case '=': { // %=
                token.setKind(Token::Kind::PercentEq);
                pos++;
                complete = true;
                break;
            }
            case '%': { // %% or %%=
                state = State::PercentPercent;
                break;
            }
            default: { // %
                // backtrack, went too far
                token.setKind(Token::Kind::Percent);
                complete = true;
                break;
            }
            }
            break;
        }
        case State::PercentPercent: {
            switch (c) {
            case '=': { // %%=
                token.setKind(Token::Kind::PercentPercentEq);
                pos++;
                complete = true;
                break;
            }
            default: { // %%
                // backtrack, went too far
                token.setKind(Token::Kind::PercentPercent);
                complete = true;
                break;
            }
            }
            break;
        }
        case State::Pipe: {
            switch (c) {
            case '=': { // |=
                token.setKind(Token::Kind::PipeEq);
                pos++;
                complete = true;
                break;
            }
            default: {
                // backtrack, went too far
                token.setKind(Token::Kind::Pipe);
                complete = true;
                break;
            }
            }
            break;
        }
        case State::Plus: {
            switch (c) {
            case '=': { // +=
                token.setKind(Token::Kind::PlusEq);
                pos++;
                complete = true;
                break;
            }
            case '+': { // ++
                token.setKind(Token::Kind::PlusPlus);
                pos++;
                complete = true;
                break;
            }
            default: {
                // backtrack, went too far
                token.setKind(Token::Kind::Plus);
                complete = true;
                break;
            }
            }
            break;
        }
        case State::Question: {
            switch (c) {
            case '?': { // ?=
                token.setKind(Token::Kind::QuestionEq);
                pos++;
                complete = true;
                break;
            }
            default: {
                // backtrack, went too far
                token.setKind(Token::Kind::Question);
                complete = true;
                break;
            }
            }
            break;
        }
        case State::Slash: {
            switch (c) {
            case '/': { // LineComment or DocComment
                state = State::LineCommentBegin;
                token.setKind(Token::Kind::LineComment);
                break;
            }
            case '=': { // '//='
                token.setKind(Token::Kind::SlashEq);
                pos++;
                complete = true;
                break;
            }
            default: { // '/'
                // backtrack, went too far
                token.setKind(Token::Kind::Slash);
                complete = true;
                break;
            }
            }
            break;
        }
        case State::Star: {
            switch (c) {
            case '=': { // *=
                token.setKind(Token::Kind::StarEq);
                pos++;
                complete = true;
                break;
            }
            case '*': { // **
                token.setKind(Token::Kind::StarStar);
                pos++;
                complete = true;
                break;
            }
            default: {
                // backtrack, went too far
                token.setKind(Token::Kind::Star);
                complete = true;
                break;
            }
            }
            break;
        }
        case State::Tilde: {
            switch (c) {
            case '=': { // +=
                token.setKind(Token::Kind::TildeEq);
                pos++;
                complete = true;
                break;
            }
            default: {
                // backtrack, went too far
                token.setKind(Token::Kind::Tilde);
                complete = true;
                break;
            }
            }
            break;
        }
        case State::Identifier: {
            if (isIdentifier(c) || isNumeric(c)) { // alphanumeric | '_'
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
        case State::LineCommentBegin: {
            switch (c) {
            case '/': { // DocComment or LineComment
                state = State::DocCommentBegin;
                break;
            }
            case '\n': {
                // end of a line comment
                token.setKind(Token::Kind::LineComment);
                complete = true;
                break;
            }
            default: {
                // definitely a line comment
                state = State::LineComment;
                break;
            }
            }
            break;
        }
        case State::DocCommentBegin: {
            switch (c) {
            case '/': { // LineComment
                state = State::LineComment;
                break;
            }
            case '\n': {
                // end of a doc comment
                token.setKind(Token::Kind::DocComment);
                complete = true;
                break;
            }
            default: {
                // definitely a doc comment
                token.setKind(Token::Kind::DocComment);
                state = State::DocComment;
                break;
            }
            }
            break;
        }
        case State::LineComment:
        case State::DocComment: {
            switch (c) {
            case '\n': {
                complete = true;
                break;
            }
            default: { break; }
            }
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

        // if we haven't finished the token, advance
        if (!complete) {
            pos++;
        }
    }

    // if we have reached the end of the input and still haven't finalized a
    // single token:
    if (pos == input->size() && !complete) {
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
        case State::Eq: {
            token.setKind(Token::Kind::Eq);
            break;
        }
        case State::Greater: {
            token.setKind(Token::Kind::Greater);
            break;
        }
        case State::GreaterGreater: {
            token.setKind(Token::Kind::GreaterGreater);
            break;
        }
        case State::Less: {
            token.setKind(Token::Kind::Less);
            break;
        }
        case State::LessLess: {
            token.setKind(Token::Kind::LessLess);
            break;
        }
        case State::Minus: {
            token.setKind(Token::Kind::Minus);
            break;
        }
        case State::Percent: {
            token.setKind(Token::Kind::Percent);
            break;
        }
        case State::PercentPercent: {
            token.setKind(Token::Kind::PercentPercent);
            break;
        }
        case State::Pipe: {
            token.setKind(Token::Kind::Pipe);
            break;
        }
        case State::Plus: {
            token.setKind(Token::Kind::Plus);
            break;
        }
        case State::Question: {
            token.setKind(Token::Kind::Question);
            break;
        }
        case State::Slash: {
            token.setKind(Token::Kind::Slash);
            break;
        }
        case State::Star: {
            token.setKind(Token::Kind::Star);
            break;
        }
        case State::Tilde: {
            token.setKind(Token::Kind::Tilde);
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
        case State::LineCommentBegin:
        case State::LineComment: {
            token.setKind(Token::Kind::LineComment);
            break;
        }
        case State::DocCommentBegin:
        case State::DocComment: {
            token.setKind(Token::Kind::DocComment);
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

void Tokenizer::dumpToken(const Token& token) const {
    // TODO: this copies for no real reason
    const std::string source = input->substr(token.start, token.length());
    std::cerr << getTokenName(token.getKind()) << " \"" << source << "\""
              << std::endl;
}

// TODO: better error handling
[[noreturn]] void Tokenizer::error(const std::string& message) {
    std::cerr << "Encountered an error!" << std::endl;
    std::cerr << message << std::endl;

    exit(0);
}

} // namespace parser
} // namespace perun
