#include <Token.hpp>
namespace oxml
{
    token::token(enum tokenType type, std::size_t start, std::size_t end) : type(type), tokenStart(start), tokenEnd(end) {}

    token::token(enum tokenType type, const std::string &characterContent, std::size_t start, std::size_t end) : type(type), characterContent(characterContent), tokenStart(start), tokenEnd(end) {}

    enum tokenType token::getType() const
    {
        return type;
    }

    const std::string &token::getCharcterContent()
    {
        return characterContent;
    }

    std::size_t token::getTokenStart() const
    {
        return tokenStart;
    }

    std::size_t token::getTokenEnd() const
    {
        return tokenEnd;
    }

    std::ostream &operator<<(std::ostream &strm, token t)
    {
        switch (t.type)
        {
        case (GREATERTHAN):
            strm << "GREATERTHAN";
            break;
        case (LESSTHAN):
            strm << "LESSTHAN";
            break;
        case (TEXT):
            strm << "TEXT : " << '"' << t.characterContent << '"';
            break;
        case (SLASH):
            strm << "SLASH";
            break;
        case (EQUAL):
            strm << "EQUAL";
            break;
        case (QUOTE):
            strm << "QUOTE";
            break;
        case (BANG):
            strm << "BANG";
            break;
        case (QUESTION):
            strm << "QUESTION";
            break;
        case (TERMINAL):
            strm << "TERMINAL";
            break;
        case (ERR):
            strm << "ERR";
            break;
        case (PERCENT):
            strm << "PERCENT";
            break;
        case (SQUARE_BRACKET_OPEN):
            strm << "SQUARE_BRACKET_OPEN";
            break;
        case (SQUARE_BRACKET_CLOSE):
            strm << "SQUARE_BRACKET_CLOSE";
            break;
        }
        return strm;
    }
}