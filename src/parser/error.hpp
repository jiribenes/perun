#ifndef PERUN_PARSER_ERROR_HPP
#define PERUN_PARSER_ERROR_HPP

#include "../ast/loc.hpp"

#include "../support/error.hpp"

namespace perun {
namespace parser {

/// Parser error
class ParseError : public support::Error {
public:
    ParseError(const std::string&& filename, const ast::Loc&& loc,
               const std::string&& text, const std::string&& sourceLine);

    const std::string getMessage() const override;

private:
    const std::string filename;
    const ast::Loc loc;
    const std::string text;

    // can be empty meaning we have no source code for this
    // TODO: use string view -- this copies for no real reason
    const std::string sourceLine;
};

} // namespace parser
} // namespace perun

#endif // PERUN_PARSER_ERROR_HPP
