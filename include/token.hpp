#ifndef MINICCOMPILER_TOKEN_HPP
#define MINICCOMPILER_TOKEN_HPP

#include <string>
#include <cstring>
#include <unordered_map>
#include <utility>

enum TokenType {
    /* TIPOS */
    INT,
    DOUBLE,
    FLOAT,
    STRING,
    BOOL,

    /* VARIÁVEIS */
    ID,
    CONST,

    /* KEYWORDS */
    IF,
    ELSE,
    FOR,
    WHILE,

   /* OPERAÇÕES */
    PLUS,
    INCREMENT,
    MINUS,
    DECREMENT,
    MUL,
    DIV,
    ASSIGN,
    EQUAL,
    NEQUAL,
    LE, // <=
    GE, // >=
    LESSER,
    GREATER,
    AND,
    OR,
    NOT,

    /* ESTRUTURAS */
    SEMI, // ;
    PAREN1, // (
    PAREN2, // )
    BRACES1,// {
    BRACES2,// }
    COMMA,  // ,
    QUOT,

    UNKNOWN
};

inline std::unordered_map<std::string, TokenType> type_table = {
    /* TIPOS */
    {"int", INT},
    {"double", DOUBLE},
    {"float", FLOAT},
    {"string", STRING},
    {"bool", BOOL},

    {"const", CONST},

    /* KEYWORDS */
    {"if", IF},
    {"else", ELSE},
    {"for", FOR},
    {"while", WHILE},

    /* OPERAÇÕES */
    {"==", EQUAL},
    {"!=", NEQUAL},
    {"<=", LE},
    {">=", GE},
    {"&&", AND},
    {"||", OR},
    {"++", INCREMENT},
    {"--", DECREMENT},

    /* LITERAIS BOOL */
    {"true", BOOL},
    {"false", BOOL}
};

inline std::unordered_map<char, TokenType> single_op_structure_table = {
     /* OPERACOES */
    {'+', PLUS},
    {'-', MINUS},
    {'*', MUL},
    {'/', DIV},
    {'=', ASSIGN},
    {'<', LESSER},
    {'>', GREATER},
    {'!', NOT},
    {'&', AND},
    {'|', OR},

    /* ESTRUTURAS */
    {';', SEMI},
    {'(', PAREN1},
    {')', PAREN2},
    {'{', BRACES1},
    {'}', BRACES2},
    {',', COMMA}
};

struct Token {
    TokenType type;
    std::string value;

    Token()
        : type(UNKNOWN)
    {};
    Token(const TokenType type, std::string value)
        : type(type), value(std::move(value))
    {}

    static TokenType get_type(const std::string &type) {
        if (const auto [is_number, is_double] = is_numeric(type); is_number) {
            if (is_double) return DOUBLE;
            return INT;
        }

        if (type_table.contains(type)) return type_table.at(type);
        return ID;
    }

    static TokenType get_type(const char op) {
        if (single_op_structure_table.contains(op)) return single_op_structure_table.at(op);
        return UNKNOWN;
    }

private:
    // (eh_numero, eh_float)
    static std::pair<bool, bool> is_numeric(const std::string &v) {
        std::pair<bool, bool> result = std::make_pair(false, false);

        for (const char &c : v) {
            if (c == '.') {
                result.second = true;
            }
            else if (!isdigit(c)) {
                return std::make_pair(false, false);
            }
        }

        result.first = true;
        return result;
    }
};

#endif //MINICCOMPILER_TOKEN_HPP