#ifndef PERUN_AST_LOC_HPP
#define PERUN_AST_LOC_HPP

#include <string>
#include <utility>

namespace perun {
namespace ast {

/// Source location
struct Loc {
    // Note: line and column are 0-indexed, not 1-indexed!
    constexpr Loc(size_t line, size_t column, size_t line_start_pos,
                  size_t line_end_pos)
        : line(line), column(column), line_start_pos(line_start_pos),
          line_end_pos(line_end_pos) {}

    size_t line, column; // line and column of this location

    // position of the start and end of the line
    size_t line_start_pos, line_end_pos;

    /// Get the length of the line
    constexpr size_t lineLength() const {
    	return line_end_pos - line_start_pos;
    }
};

using Range = std::pair<Loc, Loc>;

} // namespace ast
} // namespace perun

#endif // PERUN_AST_LOC_HPP
