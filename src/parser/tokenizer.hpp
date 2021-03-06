#ifndef PERUN_PARSER_TOKENIZER_HPP
#define PERUN_PARSER_TOKENIZER_HPP

#include <iostream>
#include <memory>
#include <vector>

#include "token.hpp"

namespace perun {
namespace parser {

/// A streaming tokenizer/lexer - acts like a basic finite automaton
class Tokenizer {

public:
    explicit Tokenizer(const std::string& input, size_t pos = 0);

    /// Returns next found token
    /// Last token will have kind 'Token::Kind::EndOfFile'
    Token nextToken();

    // Dumps the token into stderr
    // Assumes that the token was tokenized by this tokenizer
    // from this 'input'
    void dumpToken(const Token& token) const;

    const std::string& getError() const { return error; }

private:
    const std::string& input;

    enum class State {
        Invalid = -1,
        Start = 0,

        Zero,
        C,
        String,
        StringEscape,
        RawString,

        Ampersand,
        Bang,
        Colon,
        Dot,
        DotDot,
        Eq,
        Greater,
        GreaterGreater,
        Less,
        LessLess,
        Minus,
        Percent,
        PercentPercent,
        Pipe,
        Plus,
        Question,
        Slash,
        Star,
        Tilde,
        Underscore,

        Identifier,

        LineCommentBegin,
        LineComment,
        DocCommentBegin,
        DocComment,

        Integer,
        BinaryInteger,
        OctalInteger,
        HexInteger,
    };

    State state;

    /// current position in the input
    size_t pos;

    /// current error
    std::string error = "";
};

} // namespace parser
} // namespace perun

#endif // PERUN_PARSER_TOKENIZER_HPP
