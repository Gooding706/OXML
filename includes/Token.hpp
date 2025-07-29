#pragma once
#include <sstream>
#include <tuple>

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
        TERMINAL,
        SQUAREBRACKET_OPEN,
        SQUAREBRACKET_CLOSE,
        ERR
    };

    class token
    {
    public:
        token(enum tokenType type, std::size_t start, std::size_t end);
        token(enum tokenType type, const std::string &characterContent, std::size_t start, std::size_t end);

        tokenType getType() const;
        const std::string &getCharcterContent();

        std::size_t getTokenStart() const;
        std::size_t getTokenEnd() const;

        friend std::ostream &operator<<(std::ostream &strm, token t);


    private:
        tokenType type;
        std::string characterContent;

        // line number of token start/end, to be used in error reporting
        std::size_t tokenStart;
        std::size_t tokenEnd;
    };
}