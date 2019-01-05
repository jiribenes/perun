#ifndef PERUN_DRIVER_DRIVER_HPP
#define PERUN_DRIVER_DRIVER_HPP

#include "../driver/error.hpp"

#include "../ast/tree.hpp"

#include <cassert>
#include <memory>
#include <string>
#include <vector>

namespace perun {
namespace driver {

struct BuildResult {
public:
    enum Kind { Invalid = 0, DError, Tree };

    explicit BuildResult(std::unique_ptr<DriverError>&& error)
        : kind(Kind::DError), driverError(std::move(error)), tree(nullptr) {}

    explicit BuildResult(std::unique_ptr<ast::Tree>&& tree)
        : kind(Kind::Tree), driverError(nullptr), tree(std::move(tree)) {}

    Kind getKind() const { return kind; }
    std::unique_ptr<DriverError> moveError() {
        assert(kind == Kind::DError);
        return std::move(driverError);
    }

    std::unique_ptr<ast::Tree> moveTree() {
        assert(kind == Kind::Tree);
        return std::move(tree);
    }

private:
    Kind kind;
    std::unique_ptr<DriverError> driverError;
    std::unique_ptr<ast::Tree> tree;
};

BuildResult build(std::vector<std::string>& args);

} // namespace driver
} // namespace perun

#endif // PERUN_DRIVER_DRIVER_HPP
