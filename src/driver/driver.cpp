#include "error.hpp"

#include "../parser/parser.hpp"

#include "../ast/printer.hpp"
#include "../ast/stmt.hpp"

#include "../support/util.hpp"

#include <iostream>
#include <utility>

using namespace perun;

[[noreturn]] static void error(DriverError err, bool printUsage = true) {
    std::cerr << err.getMessage();
    if (printUsage) {
        std::cerr << "Usage: perun file" << std::endl;
    }
    exit(1);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        error(DriverError("-", "Invalid arguments!"));
    }

    const std::string filepath = argv[1];
    auto source = support::readFile(filepath);
    if (source.empty()) {
        error(DriverError(std::move(filepath), "Invalid file!"));
    }

    parser::Parser parser(source);
    ast::Printer printer(std::cout, 0);

    std::cout << "Original file - " << filepath << " :" << std::endl;
    std::cout << source << '\n' << std::endl;

    std::cout << "Parsed file - " << filepath << " :" << std::endl;
    auto root = parser.parseRoot();
    printer.printRoot(*root);

    return 0;
}
