#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include <Token.hpp>

namespace oxml
{

    class lexer
    {
    public:
        // create lexer from file path
        lexer(const char *path);
        // create lexer from open file
        lexer(const std::ifstream &fileStream);

        token getNextToken();

        bool eof();

    private:
        std::stringstream textStream;
        documentContext documentlexerState;

        // Using the previous token we can determine how we should Lex the next token
        tokenType previousToken{GREATERTHAN};
        
    private:
        token nextTextType();
    };
}