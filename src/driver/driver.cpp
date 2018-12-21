#include "../parser/parser.hpp"

#include "../ast/printer.hpp"
#include "../ast/stmt.hpp"

#include "../support/util.hpp"

#include <iostream>

using namespace perun;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Invalid arguments!" << std::endl;
        std::cerr << "Usage: perun file" << std::endl;
        return 1;
    }

    const std::string filepath = argv[1];
    auto source = support::readFile(filepath);
    if (source.empty()) {
        std::cerr << "Invalid file!" << std::endl;
        std::cerr << "Usage: perun file" << std::endl;
        return 1;
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
