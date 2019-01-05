#include "tree.hpp"

namespace perun {
namespace ast {

/// Returns a relative location from a position
Loc Tree::getLocFromPos(const size_t pos, const size_t start) const {
    assert(pos < source.size());

    size_t line = 0;
    size_t column = 0;
    size_t line_start_pos = start;

    // find line, column, line_start_pos
    for (size_t i = start; i < pos; ++i) {
        auto&& c = source[i];
        column++;

        if (c == '\n') {
            line++;
            column = 0;
            line_start_pos = i + 1;
        }
    }

    size_t line_end_pos = source.size() - 1;

    // find line_end_pos
    for (size_t i = pos; i < source.size(); ++i) {
        auto&& c = source[i];
        if (c == '\n') {
            line_end_pos = i;
            break;
        }
    }

    return Loc(line, column, line_start_pos, line_end_pos);
}

/// Returns a relative location from a token
Loc Tree::getLocFromToken(const parser::Token& token,
                          const size_t start) const {
    return getLocFromPos(token.start, start);
}

/// Returns a relative location from a token index
Loc Tree::getLocFromTokenIndex(const size_t tokenIndex,
                               const size_t start) const {
    assert(tokenIndex < tokens.size());
    return getLocFromToken(tokens[tokenIndex], start);
}

std::unique_ptr<Tree> Tree::get(const std::string filename,
                                const std::string source) {
    std::vector<ErrorPtr> errors{};
    std::vector<parser::Token> tokens{};
    std::unique_ptr<Root> root = nullptr;
    auto&& tree = std::make_unique<Tree>(std::move(filename), std::move(source),
                                         std::move(root), std::move(tokens),
                                         std::move(errors));

    parser::Parser parser(*tree);

    try {
        tree->setRoot(parser.parseRoot());
    } catch (...) {
        // parsing ended with an unrecoverable error.
        // ... sad.
    }

    assert(tree != nullptr);

    return std::move(tree);
}

} // namespace ast
} // namespace perun
