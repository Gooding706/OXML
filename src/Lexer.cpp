#include <Lexer.hpp>
#include <iostream>

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

    // Helper string stream functions for lexing
    std::string peekn(std::stringstream &strm, std::size_t n)
    {
        std::streampos position = strm.tellg();
        std::string out(n, ' ');
        for (int i = 0; i < n; i++)
        {
            if (strm.eof())
            {
                throw std::overflow_error("not enough data in string stream");
            }
            out[i] = strm.get();
        }

        strm.seekg(position);
        return out;
    }

    void peekn(std::stringstream &strm, char *buff, std::size_t n)
    {
        std::streampos position = strm.tellg();
        strm.getline(buff, n);
        strm.seekg(position);
    }

    // skip white space and return number of newlines
    int skipWS(std::stringstream &strm)
    {
        int lineCount = 0;
        while (isspace(strm.peek()))
        {
            char ch = strm.peek();
            if (ch == '\r' || ch == '\n')
                lineCount++;
            strm.ignore();
        }
        return lineCount;
    }

    // should this function be in this file, should this function be a macro?
    bool newLine(char ch)
    {
        return ch == '\n' || ch == '\r';
    }

    std::string consumeUntilDelimiter(std::stringstream &strm, bool (*delimeterFunc)(char), int *lines = nullptr)
    {
        std::string out = "";
        while (!delimeterFunc(strm.peek()))
        {
            if (strm.eof())
            {
                break;
            }

            unsigned char ch = strm.get();
            out.insert(out.end(), ch);

            if (lines != nullptr && newLine(ch))
                *lines += 1;
        }

        return out;
    }

    void ignoreThroughSubstr(std::stringstream &strm, const char *substr)
    {
        while (!strm.eof())
        {
            char ch = strm.get();

            int idx = 0;
            for (; substr[idx] != '\0' && substr[idx] == ch; idx++)
                ch = strm.get();
            if (idx == strlen(substr))
                break;
        }
    }

    lexer::lexer(const std::ifstream &fileStream) : documentlexerState{0}
    {
        textStream << fileStream.rdbuf();
    }

    lexer::lexer(const char *path) : documentlexerState{0}
    {
        std::ifstream fileStream{path};
        if (fileStream.fail())
            throw std::runtime_error("Failed to open file!");
        textStream << fileStream.rdbuf();
        fileStream.close();
    }

    bool isReservedCharacter_General(char ch)
    {
        return (ch == '<' || ch == '>');
    }

    bool lexer::eof()
    {
        return (previousToken == TERMINAL);
    }

    token lexer::nextTextType()
    {
        // the function we will use to determine the delimeter when reading in text
        bool (*delimeterFunc)(char);
        switch (previousToken)
        {
        case LESSTHAN:
            delimeterFunc = ([](char ch)
                             { return (isReservedCharacter_General(ch) || isspace(ch)); });
            break;
        case GREATERTHAN:
            delimeterFunc = isReservedCharacter_General;
            break;
        case TEXT:
            delimeterFunc = ([](char ch)
                             { return (isReservedCharacter_General(ch) || isspace(ch) || ch == '='); });
            break;
        case QUOTE_CLOSE:
            delimeterFunc = ([](char ch)
                             { return (isReservedCharacter_General(ch) || isspace(ch) || ch == '='); });
            break;
        case QUOTE_OPEN:
            delimeterFunc = ([](char ch)
                             { return ch == '"'; });
            break;
        default:
            delimeterFunc = isReservedCharacter_General;
        }

        int lineCount = 0;
        std::string str = consumeUntilDelimiter(textStream, delimeterFunc, &lineCount);

        // we want to record the line number when the token started
        documentContext ctx = documentlexerState;
        documentlexerState.lineNumber += lineCount;

        previousToken = TEXT;
        return token(previousToken, str, ctx);
    }

    token lexer::getNextToken()
    {
        int newLines = skipWS(textStream);
        documentlexerState.lineNumber += newLines;

        if (textStream.eof())
        {
            previousToken = TERMINAL;
        }
        // This token must not appear in text
        else if (textStream.peek() == '<')
        {
            textStream.ignore();

            // ignore comments
            if (peekn(textStream, 3) == "!--")
            {
                ignoreThroughSubstr(textStream, "-->");
                previousToken = GREATERTHAN;
                return getNextToken();
            }

            previousToken = LESSTHAN;
        }
        else if (previousToken != GREATERTHAN)
        {
            std::streampos rewindPos = textStream.tellg();
            char ch = textStream.get();

            switch (ch)
            {
            case '>':
                previousToken = GREATERTHAN;
                break;
            case '!':
                previousToken = BANG;
                break;
            case '?':
                previousToken = QUESTION;
                break;
            case '/':
                previousToken = SLASH;
                break;
            case '"':
                previousToken = (previousToken == TEXT) ? QUOTE_CLOSE : QUOTE_OPEN;
                break;
            case '=':
                previousToken = EQUAL;
                break;
            default:
                textStream.seekg(rewindPos);
                return nextTextType();
            }
        }
        else
        {
            // we can infer we will have to try and create a TEXT token, so we call out to a function that handles that
            return nextTextType();
        }

        return token(previousToken, documentlexerState);
    }
}