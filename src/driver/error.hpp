#ifndef PERUN_DRIVER_ERROR_HPP
#define PERUN_DRIVER_ERROR_HPP

#include "../support/error.hpp"

namespace perun {

class DriverError final : public support::Error {
public:
    DriverError(const std::string&& filename, const std::string&& text);

    ~DriverError() {}

    const std::string getMessage() const override;

private:
    const std::string filename;
    const std::string text;
};

} // namespace perun

#endif // PERUN_DRIVER_ERROR_HPP
