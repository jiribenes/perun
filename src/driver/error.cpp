#include "error.hpp"

#include <string>

namespace perun {

DriverError::DriverError(const std::string&& filename, const std::string&& text)
    : support::Error(), filename(std::move(filename)), text(std::move(text)) {}

const std::string DriverError::getMessage() const {
    auto fileString = filename.empty() ? "" : filename + ": ";
    return fileString + "error: " + text + "\n";
}

} // namespace perun
