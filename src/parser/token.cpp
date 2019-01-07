#include "token.hpp"

#include <cassert>

namespace perun {
namespace parser {

const char* Token::getName() const { return getTokenName(kind); }

// this should be synchronized with Token::Kind
const char* getTokenName(Token::Kind kind) {
    switch (kind) {
// This uses special macros defined in `tokenkinds.def`.
// See that file for more details on how this works.
#define TOKEN(kind, name)                                                      \
    case Token::Kind::kind:                                                    \
        return name;
#define KEYWORD(kind, name) TOKEN(Keyword##kind, name)
#define LITERAL(kind, name) TOKEN(Literal##kind, name)
#include "tokenkinds.def"
#undef TOKEN
#undef KEYWORD
#undef LITERAL
    default:
        assert(false);
    }
}

bool isKeyword(const std::string& str) {
    for (const Keyword& kw : keywords) {
        if (kw.str == str) {
            return true;
        }
    }
    return false;
}

Token::Kind getKeyword(const std::string& str) {
    for (const Keyword& kw : keywords) {
        if (kw.str == str) {
            return kw.kind;
        }
    }

    return Token::Kind::Invalid;
}

} // namespace parser
} // namespace perun
