cmake_minimum_required(VERSION 4.1)
project(MiniCCompiler)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(SOURCES
    main.cpp
        src/parser/parser.cpp
        include/token.hpp
        src/semantic_analyzer/semantic_analyzer.cpp
        include/semantic_analyzer.hpp
        include/codegenerator.hpp
        include/symbol.hpp
        src/codegenerator/codegenerator.cpp
        src/lexer/lexer.cpp
        include/lexer.hpp
)

add_executable(main ${SOURCES})

target_include_directories(main PRIVATE
    include
    ast
)

target_compile_options(main PRIVATE -Wall) #-fsanitize=address -g -O0)
#target_link_options(main PRIVATE -fsanitize=address -g -O0)

