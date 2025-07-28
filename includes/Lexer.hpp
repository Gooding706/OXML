#pragma once
#include <DocumentStream.hpp>
#include <string>
#include <Token.hpp>
#include <deque>

namespace oxml
{

    class lexer
    {
    public:
        // create lexer from file path
        lexer(const char *path);
        // create lexer from open file
        lexer(const std::ifstream &fileStream);

        const documentStream& rdbuf();

        token getNextToken();

        bool eof();

    private:
        documentStream textStream;
        /*often it is sensical for the tokenizer to get multiple tokens at once
        in this case we will store the excess within this buffer for later
        use when the caller asks for them via : getNextToken()*/
        std::deque<token> tokenBuffer;
    };
}