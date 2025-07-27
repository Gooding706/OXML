#pragma once
#include <fstream>
#include <sstream>
#include <string>

namespace oxml
{
    enum tokenType
    {
        GREATERTHAN,
        LESSTHAN,
        TEXT,
        SLASH,
        EQUAL,
        QUOTE_OPEN,
        QUOTE_CLOSE,
        BANG,
        QUESTION,
        TERMINAL
    };

    /*
    we hold a document context on each token and during the lexing process in order to
    print reasonable errors during lexing and parsing
    */
    struct documentContext
    {
        /*
        refers to the position of the line containing the token within the text stream
        textStream.seekg(lineStart);
        textStream.getLine(); => line containing token
        */
        std::streampos lineStart;
        int lineNumber;
    };

    struct token
    {
        token(enum tokenType type);
        token(enum tokenType type, const std::string &characterContent);

        token(enum tokenType type, documentContext ctx);
        token(enum tokenType type, const std::string &characterContent, documentContext ctx);

        tokenType getType() const;
        const std::string &getCharcterContent();
        documentContext getTokenContext() const;

        friend std::ostream &operator<<(std::ostream &strm, token t);

    private:
        tokenType type;
        std::string characterContent;
        documentContext tokenContext;
    };

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