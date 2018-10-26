#ifndef PERUN_PARSER_TOKEN_HPP
#define PERUN_PARSER_TOKEN_HPP

#include <cassert>
#include <string>

namespace perun {
namespace parser {

struct Token {
    enum class Kind {
        Invalid = -1,
        EndOfFile = 0,

        // parens
        LParen,
        RParen,
        LBrace,
        RBrace,
        LBracket,
        RBracket,

        // operators
        Ampersand,
        AmpersandEq,
        At,
        Backslash,
        Bang, // !
        BangEq,
        Caret,
        Colon,
        ColonColon,
        Comma,
        Dot,
        DotDot,
        DotDotDot,
        Eq,
        EqEq,
        EqGreater, // =>
        Greater,   // >
        GreaterEq,
        GreaterGreater,
        GreaterGreaterEq,
        Hash,
        Less, // <
        LessEq,
        LessLess,
        LessLessEq,
        Minus,
        MinusEq,
        MinusGreater, // ->
        Percent,
        PercentEq,
        PercentPercent,
        PercentPercentEq,
        Pipe,
        PipeEq,
        Plus,
        PlusEq,
        PlusPlus,
        Question, // ?
        QuestionEq,
        Semicolon,
        Slash, // /
        SlashEq,
        Star,
        StarEq,
        StarStar,
        Tilde,
        TildeEq,

        // keywords
        KeywordAlign,
        KeywordAnd,
        KeywordAsm,
        KeywordBreak,
        KeywordCatch,
        KeywordConst,
        KeywordContinue,
        KeywordDefer,
        KeywordElse,
        KeywordEnum,
        KeywordError,
        KeywordExport,
        KeywordExtern,
        KeywordFalse,
        KeywordFn,
        KeywordFor,
        KeywordIf,
        KeywordIn,
        KeywordMatch,
        KeywordNil,
        KeywordNot,
        KeywordOr,
        KeywordPriv,
        KeywordPub,
        KeywordPure,
        KeywordReturn,
        KeywordStruct,
        KeywordThis,
        KeywordTrue,
        KeywordTry,
        KeywordUndefined,
        KeywordUnion,
        KeywordUse,
        KeywordVar,
        KeywordVolatile,
        KeywordWhile,

        // literals
        LiteralChar,
        LiteralFloat,
        LiteralInteger,

        LiteralCRawString,
        LiteralCString,
        LiteralRawString,
        LiteralString,

        Identifier,

        // comments
        LineComment,
        DocComment,
    };

    Token(Kind kind, size_t start) : start(start), kind(kind) {}

    Kind getKind() const { return kind; }
    void setKind(Kind k) { kind = k; }

    /// Predicates for checking token's kind
    bool is(Kind k) const { return kind == k; }
    bool isNot(Kind k) const { return kind != k; }

    bool isOneOf(Kind k1, Kind k2) const { return is(k1) || is(k2); }
    template <typename... Ts> bool isOneOf(Kind k1, Kind k2, Ts... ks) const {
        return is(k1) || isOneOf(k2, ks...);
    }

    size_t length() const {
        assert(end >= start && "Token's end is before its start");
        return end - start;
    }

    /// Start and end offsets from the beginning of the file
    size_t start;
    size_t end; // undefined by default

private:
    Kind kind;
};

const char* getTokenName(Token::Kind kind);

// TODO: ideally hide everything regarding keywords in the Token class
struct Keyword {
    const char* str;
    const Token::Kind kind;
};

static const Keyword keywords[] = {
    {"align", Token::Kind::KeywordAlign},
    {"and", Token::Kind::KeywordAnd},
    {"asm", Token::Kind::KeywordAsm},
    {"break", Token::Kind::KeywordBreak},
    {"catch", Token::Kind::KeywordCatch},
    {"const", Token::Kind::KeywordConst},
    {"continue", Token::Kind::KeywordContinue},
    {"defer", Token::Kind::KeywordDefer},
    {"else", Token::Kind::KeywordElse},
    {"enum", Token::Kind::KeywordEnum},
    {"error", Token::Kind::KeywordError},
    {"export", Token::Kind::KeywordExport},
    {"extern", Token::Kind::KeywordExtern},
    {"false", Token::Kind::KeywordFalse},
    {"fn", Token::Kind::KeywordFn},
    {"for", Token::Kind::KeywordFor},
    {"if", Token::Kind::KeywordIf},
    {"in", Token::Kind::KeywordIn},
    {"match", Token::Kind::KeywordMatch},
    {"nil", Token::Kind::KeywordNil},
    {"not", Token::Kind::KeywordNot},
    {"or", Token::Kind::KeywordOr},
    {"priv", Token::Kind::KeywordPriv},
    {"pub", Token::Kind::KeywordPub},
    {"pure", Token::Kind::KeywordPure},
    {"return", Token::Kind::KeywordReturn},
    {"struct", Token::Kind::KeywordStruct},
    {"this", Token::Kind::KeywordThis},
    {"true", Token::Kind::KeywordTrue},
    {"try", Token::Kind::KeywordTry},
    {"undefined", Token::Kind::KeywordUndefined},
    {"union", Token::Kind::KeywordUnion},
    {"use", Token::Kind::KeywordUse},
    {"var", Token::Kind::KeywordVar},
    {"volatile", Token::Kind::KeywordVolatile},
    {"while", Token::Kind::KeywordWhile},
};

bool isKeyword(const std::string& str);

/// Returns a keyword token kind if string is a keyword
/// otherwise returns Token::Kind::Invalid
Token::Kind getKeyword(const std::string& str);

} // namespace parser
} // namespace perun

#endif // PERUN_PARSER_TOKEN_HPP
