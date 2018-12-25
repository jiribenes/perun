#ifndef PERUN_PARSER_RANGE_HPP
#define PERUN_PARSER_RANGE_HPP

#include <string>

#include "../ast/node.hpp"

namespace perun {
namespace parser {

/// Range of token indexes representing a part of the source code
struct Range {
    constexpr Range(size_t first, size_t last) : first(first), last(last) {}
    explicit constexpr Range(size_t tokenIndex)
        : first(tokenIndex), last(tokenIndex) {}

    size_t first, last; // first and last token index of this source range

    static Range get(const ast::Node& node) noexcept {
        return Range(node.firstTokenIndex(), node.lastTokenIndex());
    }
};

} // namespace parser
} // namespace perun

#endif // PERUN_PARSER_RANGE_HPP
