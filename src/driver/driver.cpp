#include "driver.hpp"

#include "error.hpp"

#include "../support/util.hpp"

#include "../ast/printer.hpp"
#include "../ast/tree.hpp"

#include "../parser/parser.hpp"

#include <algorithm>

namespace perun {
namespace driver {

static bool hasFlag(std::string flag, std::vector<std::string>& args) {
    auto&& it = std::find(args.begin(), args.end(), flag);
    bool contains = it != args.end();
    if (contains) {
        args.erase(it);
    }
    return contains;
}

BuildResult build(std::vector<std::string>& args) {
    bool verbose = hasFlag("--verbose", args) || hasFlag("-v", args);

    // process all remaining arguments - they should be all params and not flags
    for (auto&& arg : args) {
        if (arg.empty() || arg[0] == '-') {
            return BuildResult(std::make_unique<DriverError>(
                "unsupported option '" + arg + "'"));
        }
    }

    if (args.empty()) {
        return BuildResult(std::make_unique<DriverError>("no input file"));
    }

    if (args.size() != 1) {
        return BuildResult(
            std::make_unique<DriverError>("only one input file allowed"));
    }

    const std::string file = args[0];

    std::string source = support::readFile(file);
    if (source.empty()) {
        return BuildResult(std::make_unique<DriverError>(
            "could not load file: '" + file + "'"));
    }

    auto&& tree = ast::Tree::get(std::move(file), std::move(source));
    assert(tree != nullptr);

    if (tree->hasErrors()) {
        return BuildResult(std::move(tree));
    }

    if (verbose) {
        // print ast formatted
        ast::Printer printer(std::cout, 0);
        printer.printRoot(*tree->getRoot());
    }

    return BuildResult(std::move(tree));
}

} // namespace driver
} // namespace perun
