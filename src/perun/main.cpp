#include "../driver/driver.hpp"
#include "../driver/error.hpp"

#include "../support/error.hpp"

#include <cassert>
#include <iostream>
#include <utility>

using namespace perun;

static void printUsage() {
    std::cout << "Usage: perun [-h/--help] [-v/--verbose] <input>" << std::endl;
}

int main(int argc, char* argv[]) {
    std::vector<std::string> args(argv + 1, argv + argc);

    for (auto&& arg : args) {
        if (arg == "--help" || arg == "-h") {
            printUsage();
            return 0;
        }
    }

    auto&& result = driver::build(args);

    switch (result.getKind()) {
    case driver::BuildResult::Kind::Invalid: {
        assert(false);
    }
    case driver::BuildResult::Kind::DError: {
        auto&& err = result.moveError();
        std::cerr << err->getMessage();
        return 1;
    }
    case driver::BuildResult::Kind::Tree: {
        auto&& tree = result.moveTree();
        assert(tree != nullptr);
        if (tree->hasErrors()) {
            for (auto&& err : tree->getErrors()) {
                std::cerr << err->getMessage() << std::endl;
            }
            return 1;
        }
        return 0;
    }
    }

    return 1;
}
