#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace perun {
namespace support {

std::string readFile(const std::string& filename) {
    std::ifstream in(filename);

    if (in) {
        std::stringstream buffer;
        buffer << in.rdbuf();
        in.close();
        return buffer.str();
    }

    return "";
}

} // namespace support
} // namespace perun
