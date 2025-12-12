#include "lexer.hpp"
#include "parser.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

int main() {
/*     std::ifstream test_code("test.txt");

    if (!test_code.is_open()) {
        std::cerr << "erro ao abrir o arquivo;\n";
        return 0;
    }

    std::stringstream buffer;
    buffer << test_code.rdbuf();

    std::string content = buffer.str(); */

    const std::string content = "if (x > 0) { if (y < 3) { x = 1; } else { x = 2; } }";
    Lexer lex(content);


    auto decoded_code = lex.decode();
    Parser parser(decoded_code);

    const auto *a = dynamic_cast<IfStmt*>(parser.parseStatement());
    parser.next_instruction();
    const auto *b = dynamic_cast<IfStmt*>(parser.parseStatement());

    if (!a) {
        std::cout << "ponteiro a invalido" << std::endl;
    }
    else {
        std::cout << "ponteiro a existe" << std::endl;
    }

    delete a;


    /* for (auto i : lex.decode()) {
        for (auto v : i) {
            std::cout << v << " ";
        }
        std::cout << "\n";
    } */

    return 0;
}