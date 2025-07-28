#include <Token.hpp>
namespace oxml
{
    token::token(enum tokenType type)
    {
        this->type = type;
    }

    token::token(enum tokenType type, const std::string &characterContent)
    {
        this->type = type;
        this->characterContent = characterContent;
    }

    token::token(enum tokenType type, documentContext ctx)
    {
        this->type = type;
        this->tokenContext = ctx;
    }

    token::token(enum tokenType type, const std::string &characterContent, documentContext ctx)
    {
        this->type = type;
        this->characterContent = characterContent;
        this->tokenContext = ctx;
    }

    enum tokenType token::getType() const
    {
        return type;
    }

    const std::string &token::getCharcterContent()
    {
        return characterContent;
    }

    documentContext token::getTokenContext() const
    {
        return tokenContext;
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
        case (QUOTE_OPEN):
            strm << "QUOTE_OPEN";
            break;
        case (QUOTE_CLOSE):
            strm << "QUOTE_CLOSE";
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
        }
        return strm;
    }
}