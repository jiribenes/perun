#ifndef PERUN_SUPPORT_ERROR_HPP
#define PERUN_SUPPORT_ERROR_HPP

#include <string>
#include <vector>

namespace perun {
namespace support {

/// Interface for errors
class Error {
public:
    explicit Error() {}

    virtual ~Error() = default;

    virtual const std::string getMessage() const = 0;
};

} // namespace support
} // namespace perun

#endif // PERUN_SUPPORT_ERROR_HPP
