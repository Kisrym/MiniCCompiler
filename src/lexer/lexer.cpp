#include "lexer.hpp"

#include <stdexcept>

char Lexer::consume() {
    if (pos <= content.length()) {
        return content[pos++];
    }
    return '\0';
}

char Lexer::peek() const {
    if (pos < content.length()) {
        return content[pos];
    }
    return '\0';
}

char Lexer::peekNext() const {
    if (pos + 1 <= content.length()) {
        return content[pos + 1];
    }
    return '\0';
}

void Lexer::emit(TokenType type, const std::string &value) {
    tokens.emplace_back(type, value);
}

std::vector<Token> Lexer::tokenize() {
    while (const char l = peek()) {
        if (isspace(l)) consume();
        else if (isdigit(l)) scanNumbers();
        else if (isalpha(l)) scanName();
        else if (single_op_structure_table.contains(l)) {
            if (const std::string tmp{l, peekNext()}; type_table.contains(tmp)) {
                emit(Token::get_type(tmp), tmp);
                consume();
                consume(); // elemento 2
                continue;
            }
            emit(Token::get_type(l), std::string{l});
            consume();
        }
        else if (l == '"') scanString();
    }

    if (peek() != '\0')
        tokens.emplace_back(Token::get_type(peek()), std::string{peek()});

    return tokens;
}

void Lexer::scanNumbers() {
    std::string value;

    while (isdigit(peek()) || (peek() == '.' && isdigit(peekNext()))) {
        value += consume(); // passa pro proximo
    }

    emit(Token::get_type(value), value);
}

void Lexer::scanName() {
    std::string value;
    bool open_paren = false;

    while (isalpha(peek()) || isdigit(peek())) {
        value += consume();
    }

    emit(Token::get_type(value), value);
}

void Lexer::scanString() {
    std::string value;
    consume();

    while (peek() != '"') {
        value += consume();
    }

    consume();
    emit(STRING, value);
}