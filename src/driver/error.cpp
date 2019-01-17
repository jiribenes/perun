#include "error.hpp"

#include <string>

namespace perun {

DriverError::DriverError(const std::string&& text)
    : support::Error(), text(std::move(text)) {}

const std::string DriverError::getMessage() const {
    return "perun: error: " + text + "\n";
}

} // namespace perun
