#ifndef PERUN_SUPPORT_UTIL_HPP
#define PERUN_SUPPORT_UTIL_HPP

#include <string>

namespace perun {
namespace support {

/// Reads a file, located in 'filename', returns whole file as a string
/// Can return "" if couldn't open file!
std::string readFile(const std::string& filename);

} // namespace support
} // namespace perun

#endif // PERUN_SUPPORT_UTIL_HPP
