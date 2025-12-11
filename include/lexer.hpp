#ifndef __LEXER_H__
#define __LEXER_H__

#include <string>
#include <vector>
#include <sstream>

#include "token.hpp"

class Lexer {
    std::vector<std::string> code;

    static std::vector<Token> decode_instruction(const std::string &instruction);
    static std::string remove_spaces(const std::string &input);
    static bool is_symbol(char symbol);
public:
    Lexer(const std::string &code);
    std::vector<std::vector<Token>> decode() const;
};

#endif

/*inline std::string ltrim(std::string word) {
        int spaces = 0;

        for (std::size_t i = 0; i < word.size() - 1; i++) {
            if (isspace(word.at(i))) {
                spaces++;
            }
            else {
                pc += spaces; // incrementa os espaços que foram comidos
                return word.substr(spaces);
            }
        }

        return word; // não há espaços
    }

    inline std::string rtrim(std::string word) {
        int spaces = 0;

        for (std::size_t i = word.size() - 1; i >= 0; i--) {
            if (isspace(word.at(i))) {
                spaces++;
            }
            else if (word.at(i) != ';') {
                pc += spaces; // incrementa os espaços que foram comidos
                return word.substr(0, (word.size() - 1) - spaces).append(";");
            }
        }

        return word; // não há espaços
    }

    inline std::string trim(std::string word) {
        return rtrim(ltrim(word));
    }*/