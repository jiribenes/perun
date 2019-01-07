#ifndef PERUN_PARSER_TOKEN_HPP
#define PERUN_PARSER_TOKEN_HPP

#include <cassert>
#include <string>

namespace perun {
namespace parser {

struct Token {
    enum class Kind {
        Invalid = -1,
// This uses special macros defined in `tokenkinds.def`.
// See that file for more details on how this works.
#define TOKEN(kind, name) kind,
#define KEYWORD(kind, name) Keyword##kind,
#define LITERAL(kind, name) Literal##kind,
#include "tokenkinds.def"
#undef TOKEN
#undef KEYWORD
#undef LITERAL
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

    /// Returns a name of the token('s kind)
    const char* getName() const;

    /// Start and end offsets from the beginning of the file
    size_t start;
    size_t end; // undefined by default

private:
    Kind kind;
};

const char* getTokenName(Token::Kind kind);

// Thin wrapper to allow a keyword table
struct Keyword {
    const char* str;
    const Token::Kind kind;
};

static const Keyword keywords[] = {
// This uses special macros defined in `tokenkinds.def`.
// See that file for more details on how this works.
#define KEYWORD(kind, name) {name, Token::Kind::Keyword##kind},
#define TOKEN(kind, name)   /* empty */
#define LITERAL(kind, name) /* empty */
#include "tokenkinds.def"
#undef KEYWORD
#undef TOKEN
#undef LITERAL
};

bool isKeyword(const std::string& str);

/// Returns a keyword token kind if string is a keyword
/// otherwise returns Token::Kind::Invalid
Token::Kind getKeyword(const std::string& str);

} // namespace parser
} // namespace perun

#endif // PERUN_PARSER_TOKEN_HPP
