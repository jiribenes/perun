#include "error.hpp"

#include <sstream>
#include <string>

#include "../ast/loc.hpp"

namespace perun {
namespace parser {

ParseError::ParseError(const std::string&& filename, const ast::Loc&& loc,
                       const std::string&& text, const std::string&& sourceLine)
    : support::Error(), filename(std::move(filename)), loc(std::move(loc)),
      text(std::move(text)), sourceLine(std::move(sourceLine)) {}

const std::string ParseError::getMessage() const {
    std::string firstLine = filename + ":" + std::to_string(loc.line + 1) +
                            ":" + std::to_string(loc.column + 1) +
                            ": error: " + text;

    // source line wasn't given
    if (sourceLine.empty()) {
        return firstLine;
    }

    std::stringstream ss;
    ss << firstLine << "\n";
    ss << sourceLine << "\n";

    // print a line with a marker where the error is located
    for (size_t i = 0; i < sourceLine.size() + 1; ++i) {
        if (i == loc.column) {
            ss << "^";
        } else {
            ss << " ";
        }
    }

    return ss.str();
}

} // namespace parser
} // namespace perun
