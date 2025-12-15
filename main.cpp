#include "lexer.hpp"
#include "parser.hpp"
#include "semantic_analyzer.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

#include "codegenerator.hpp"

std::string readFileIntoString(const std::string& filename) {
    std::ifstream ifs("/home/kaio/Documentos/miniccompiler/" + filename);
    if (!ifs) {
        std::cerr << "Error reading file " << filename << std::endl;
    }

    std::stringstream buffer;
    buffer << ifs.rdbuf();
    ifs.close();

    return buffer.str();
}

int main() {
    const std::string content = readFileIntoString("input.txt");

    const Lexer lexer(content); // transforma o texto em tokens
    Parser parser(lexer.decode()); // organiza os tokens em um ast válida
    SemanticAnalyzer analyzer;

    CodeGenerator gen(&parser, &analyzer);
    const auto v = gen.generateCode();

    for (const auto &c : v) {
        std::cout << c;
    }
    std::cout << "Sucesso\n";

    return 0;
}