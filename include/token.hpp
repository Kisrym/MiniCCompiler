#ifndef MINICCOMPILER_TOKEN_HPP
#define MINICCOMPILER_TOKEN_HPP

#include <string>
#include <cstring>
#include <utility>

enum TokenType {
    INT,
    DOUBLE,
    FLOAT,
    STRING,
    BOOL,

    ID, // variaveis
    NUM,
    VALUE,
    IF,
    ELSE,
    PLUS,
    MINUS,
    MUL,
    DIV,
    SEMI, // ;
    ASSIGN,
    EQUAL,
    LE,
    GE,
    NEQUAL,
    LESSER,
    GREATER,
    PAREN1,
    PAREN2,
    BRACES1,
    BRACES2,
    COMMA,
    FOR,
    WHILE,
    UNKNOWN
};

std::ostream& operator<<(std::ostream &os, TokenType type);

struct Token {
    TokenType type;
    std::string value;

    Token(const TokenType type, std::string value)
        : type(type), value(std::move(value))
    {}

    friend std::ostream& operator<<(std::ostream &os, const Token &token);
    static TokenType get_type(const char type[]) {
        const auto [is_number, is_double] = is_numeric(type);

        if (is_number) {
            if (is_double) {
                return DOUBLE;
            }
            return INT;
        }

        if (!strcmp(type, "int")){
            return INT;
        }
        if (!strcmp(type, "double")){
            return DOUBLE;
        }
        if (!strcmp(type, "float")){
            return FLOAT;
        }
        if (!strcmp(type, "string")) {
            return STRING;
        }

        if (!strcmp(type, "bool")) {
            return BOOL;
        }

        if (!strcmp(type, "if")) {
            return IF;
        }

        if (!strcmp(type, "else")) {
            return ELSE;
        }

        if (!strcmp(type, "for")) {
            return FOR;
        }

        if (!strcmp(type, "while")) {
            return WHILE;
        }

        if (!strcmp(type, "+")) {
            return PLUS;
        }

        if (!strcmp(type, "-")) {
            return MINUS;
        }

        if (!strcmp(type, "*")) {
            return MUL;
        }

        if (!strcmp(type, "/")) {
            return DIV;
        }

        if (!strcmp(type, ";")) {
            return SEMI;
        }

        if (!strcmp(type, "=")) {
            return ASSIGN;
        }

        if (!strcmp(type, ">")) {
            return GREATER;
        }

        if (!strcmp(type, "<")) {
            return LESSER;
        }

        if (!strcmp(type, "==")) {
            return EQUAL;
        }

        if (!strcmp(type, "!=")) {
            return NEQUAL;
        }

        if (!strcmp(type, ">=")) {
            return GE;
        }

        if (!strcmp(type, "<=")) {
            return LE;
        }

        if (!strcmp(type, "(")) {
            return PAREN1;
        }

        if (!strcmp(type, ")")) {
            return PAREN2;
        }

        if (!strcmp(type, "{")) {
            return BRACES1;
        }

        if (!strcmp(type, "}")) {
            return BRACES2;
        }

        if (!strcmp(type, ",")) {
            return COMMA;
        }

        else { // sera considerado uma variavel
            return ID;
        }

    }
    static TokenType get_type(const char type) {
        if (type == '+') {
            return PLUS;
        }

        if (type == '-') {
            return MINUS;
        }

        if (type == '*') {
            return MUL;
        }

        if (type == '/') {
            return DIV;
        }

        if (type == ';') {
            return SEMI;
        }

        if (type == '=') {
            return ASSIGN;
        }

        if (type == '(') {
            return PAREN1;
        }

        if (type == ')') {
            return PAREN2;
        }

        if (type == '>') {
            return GREATER;
        }

        if (type == '<') {
            return LESSER;
        }

        if (type == '{') {
            return BRACES1;
        }

        if (type == '}') {
            return BRACES2;
        }

        if (type == ',') {
            return COMMA;
        }

        else {
            return UNKNOWN;
        }
    }

private:
    // (eh_numero, eh_float)
    static std::pair<bool, bool> is_numeric(const char v[]) {
        std::pair<bool, bool> result = std::make_pair(false, false);
        for (int i = 0; v[i] != '\0'; i++) {
            if (v[i] == '.') {
                result.second = true;
            }
            if (!(v[i] >= 48 && v[i] <= 57) && !(v[i] == '.')) {
                return std::make_pair(false, false);
            }
        }

        result.first = true;
        return result;
    }
};

#endif //MINICCOMPILER_TOKEN_HPP