#include "lexer.hpp"
#include "constraints.hpp"

/*
int x = 5 + 10;

if (x > 5) {
}

*/

Lexer::Lexer(const std::string &code) {
    /*std::string i;
    std::istringstream ss(code);
    while (getline(ss, i, ';')) { // vai separar de ; em ;
        this->code.push_back(i);
    }*/

    std::string bff;
    int blocks = 0;

    for (const auto &c : code) {
        if (c == '{') {
            blocks++;
        }

        else if (c == '}') {
            bff += "}";
            blocks--;

            if (blocks == 0) {
                this->code.push_back(bff);
                bff.clear();
            }
            continue;
        }

        if (blocks == 0 && c == ';') {
            bff += ';';
            this->code.push_back(bff);
            bff.clear();
            continue;
        }

        bff += c;
    }

    /*if (!bff.empty())
        this->code.push_back(bff);*/

    if (code.empty()) {
        throw std::runtime_error("Missing semicolon ';'");
    }
}

std::vector<Token> Lexer::decode_instruction(const std::string &instruction) {
    char buffer[1000];
    std::size_t start = 0;
    std::size_t end = 0;
    char l;
    std::vector<Token> tokens;

    for (std::size_t i = 0; i < instruction.size(); i++) {
        if (is_symbol(instruction.at(i))) {
            
            // buffer é tratado como um ponteiro para o primeiro indice
            // entao ao somá-lo com um tamanho, irá tratar como o ponteiro deste tamanho

            /*
            buffer = &buffer[0]
            buffer + 5 = &buffer[5]
            */

            // essa condição avalia operações booleanas compostas com = (>=, <=, ==)
            const bool cond_eq1 = (instruction.at(i) == '>' || instruction.at(i) == '<' || instruction.at(i) == '=' || instruction.at(i) == '!')
                         && (i + 1 <= instruction.size()) && (instruction.at(i + 1) == '=');

            // essa condição avalia or e and
            char tmp;
            const bool cond_eq2 = (
                ((tmp = ((instruction.at(i) == '&') ? '&' : '\0'))) || ((tmp = ((instruction.at(i) == '|') ? '|' : '\0'))
                )) && (i + 1 <= instruction.size()) && (instruction.at(i + 1) == tmp);

            if (start == end && (!cond_eq2 && !cond_eq1)) { // simbolos seguidos diferentes das compostas
                std::string bff = {instruction.at(i)};
                tokens.emplace_back(Token::get_type(instruction.at(i)), bff);
                continue;
            }

            std::string bff(buffer + start, buffer + end);

            if (!bff.empty())
                tokens.emplace_back(Token::get_type(bff.c_str()), bff);

            if (cond_eq1 || cond_eq2){
                std::string bff2;
                bff2 += instruction.at(i);
                bff2 += instruction.at(i + 1);

                tokens.emplace_back(Token::get_type(bff2.c_str()), bff2);
                i++;
            }

            else {
                tokens.emplace_back(Token::get_type(instruction.at(i)), std::string() + instruction.at(i));
            }

            start = end; // "reinicia" o buffer
        }

        else {
            if (instruction.at(i) == ' ') {
                std::string bff(buffer + start, buffer + end);
                tokens.emplace_back(Token::get_type(bff.c_str()), bff);
                start = end;
                continue;
            }

            l = instruction.at(i);
            buffer[end++] = l;
    
        }
    }

    // pega o último elemento
    std::string bff(buffer + start, buffer + end);

    if (!bff.empty()) { // se o código começar com ;, vai ignorar
        tokens.emplace_back(Token::get_type(bff.c_str()), bff);
    }

    return tokens;
}

std::string Lexer::remove_spaces(const std::string &input) {
    std::string result;
    std::string bff;
    for (const char i : input) {
        if (i != ' ' && i != '\n') {
            result += i;
            bff += i;
        }

        if (i == ';') {
            bff.clear();
        }

        else if (i == ' ' || i == '\n') {
            if (bff.find("int") != std::string::npos || bff.find("double") != std::string::npos || bff.find("float") != std::string::npos || bff.find("bool") != std::string::npos) {
                result += ' ';
                bff.clear();
            }
            /*
            if (bff == "int" || bff == "(int" || bff == "double" || bff == "(double" || bff == "string" || bff == "(string" || bff == "bool" || bff == "(bool") {
                result += ' ';
            }*/
            else {
                bff.clear();
            }

        }
    }

    return result;
}

bool Lexer::is_symbol(const char symbol) {
    return (symbol == '=' ||symbol == '+' || symbol == '-' || symbol == '*' || symbol == '/' || symbol == '<' || symbol == '>' || symbol == '(' || symbol == ')' || symbol == '{' || symbol == '}' || symbol == ';' || symbol == ',' || symbol == '!' || symbol == '&' || symbol == '|');
}

std::vector<std::vector<Token>> Lexer::decode() const {
    std::vector<std::vector<Token>> tokens;
    for (const auto &ins : this->code) {
        tokens.push_back(decode_instruction(remove_spaces(ins)));
    }

    return tokens;
}

std::ostream& operator<<(std::ostream &os, const TokenType type) {
    switch (type) {
        case INT: os << "INT"; return os;
        case DOUBLE: os << "DOUBLE"; return os;
        case FLOAT: os << "FLOAT"; return os;
        case STRING: os << "STRING"; return os;
        case ID: os << "ID"; return os;
        case NUM: os << "NUM"; return os;
        case IF: os << "IF"; return os;
        case ELSE: os << "ELSE"; return os;
        case PLUS: os << "PLUS"; return os;
        case MINUS: os << "MINUS"; return os;
        case MUL: os << "MUL"; return os;
        case DIV: os << "DIV"; return os;
        case SEMI: os << "SEMI"; return os;
        case ASSIGN: os << "ASSIGN"; return os;
        case EQUAL: os << "EQUAL"; return os;
        case NEQUAL: os << "NEQUAL"; return os;
        case LESSER: os << "LESSER"; return os;
        case GREATER: os << "GREATER"; return os;
        case LE: os << "LESEQUAL"; return os;
        case GE: os << "GREQUAL"; return os;

        default: os << "MINUS"; return os;
    }
} 

std::ostream& operator<<(std::ostream &os, const Token &token) {
    if (token.type >= 0 && token.type <= 3) {
        os << token.type;
        return os;
    }

    os << token.type << "(" << token.value << ")";
    return os;
}