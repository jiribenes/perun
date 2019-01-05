#ifndef PERUN_AST_TREE_HPP
#define PERUN_AST_TREE_HPP

#include <memory>
#include <string>
#include <vector>

#include "../ast/stmt.hpp"

#include "../parser/parser.hpp"
#include "../parser/token.hpp"

#include "../support/error.hpp"

namespace perun {
namespace ast {

class Root;

/// Manager of a single Abstract Syntax Tree
class Tree {
public:
    using ErrorPtr = std::unique_ptr<support::Error>;

    explicit Tree(std::string filename, std::string source,
                  std::unique_ptr<Root>&& root,
                  std::vector<parser::Token>&& tokens,
                  std::vector<ErrorPtr>&& errors)
        : filename(std::move(filename)), source(std::move(source)),
          root(std::move(root)), tokens(std::move(tokens)),
          errors(std::move(errors)) {}

    const std::string& getFilename() const { return filename; }
    const std::string& getSource() const { return source; }

    const Root* getRoot() const { return root.get(); }
    void setRoot(std::unique_ptr<Root>&& r) {
        assert(root == nullptr);
        root = std::move(r);
    }

    const std::vector<parser::Token>& getTokens() const { return tokens; }
    std::vector<parser::Token>& getTokensMut() { return tokens; }

    const std::vector<ErrorPtr>& getErrors() const { return errors; }
    std::vector<ErrorPtr>& getErrorsMut() { return errors; }
    bool hasErrors() const { return !errors.empty(); }
    void addError(ErrorPtr&& error_ptr) {
        errors.push_back(std::move(error_ptr));
    }

    /// Returns a relative location from a position
    Loc getLocFromPos(const size_t pos, const size_t start = 0) const;

    /// Returns a relative location from a token
    Loc getLocFromToken(const parser::Token& token,
                        const size_t start = 0) const;

    /// Returns a relative location from a token index
    Loc getLocFromTokenIndex(const size_t tokenIndex,
                             const size_t start = 0) const;

    static std::unique_ptr<Tree> get(const std::string filename,
                                     const std::string source);

private:
    const std::string filename;
    const std::string source;
    std::unique_ptr<Root> root;

    std::vector<parser::Token> tokens;

    std::vector<ErrorPtr> errors;
};

} // namespace ast
} // namespace perun

#endif // PERUN_AST_TREE_HPP
