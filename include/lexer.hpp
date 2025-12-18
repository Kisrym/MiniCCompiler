#ifndef MINICCOMPILER_LEXER_RMK_HPP
#define MINICCOMPILER_LEXER_RMK_HPP

#include <string>
#include <utility>
#include <vector>

#include "token.hpp"

class Lexer {
private:
    std::string content;
    std::size_t pos = 0;

    std::vector<Token> tokens;

    // scanners
    void scanNumbers();
    void scanName();
    void scanString();

    // util
    char consume();
    char peek() const;
    char peekNext() const;
    void emit(TokenType type, const std::string &value);

public:
    explicit Lexer(std::string content)
        : content(std::move(content))
    {}

    std::vector<Token> tokenize();
};

#endif //MINICCOMPILER_LEXER_RMK_HPP