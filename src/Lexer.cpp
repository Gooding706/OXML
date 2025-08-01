#include <Lexer.hpp>
#include <iostream>

namespace oxml
{
    lexer::lexer(const char *path) : textStream(path) {}

    // create lexer from open file
    lexer::lexer(const std::ifstream &fileStream) : textStream(fileStream) {}

    const documentStream &lexer::rdbuf() { return textStream; }

    // function to ignore all whitespace + comments, after this function is called the next character will be "tokenize-able"
    void lexer::ignoreUnread()
    {
        while (!textStream.eof())

        {
            textStream.ignoreWS();
            /*4 here is a "magic number" denoting strlen("<!--"), that being said the start of
        comments being <!-- is stable under the XML standard so I feel ok about leaving this
        value hardcoded
        */
            if (textStream.peekn(4) == "<!--")
            {
                textStream.ignore(std::numeric_limits<std::streamsize>::max(), "-->");
            }
            else
            {
                break;
            }
        }
    }

    std::string lexer::extractCharData()
    {
        // strlen(<![CDATA[) == 9
        textStream.ignore(9, '\0');
        std::size_t tokenStart = textStream.tellLine();
        std::string out = textStream.getUntil("]]>");
        if (textStream.eof())
            throw generateException("unclosed CDATA tag", tokenStart, textStream.tellLine());
        /*this is a hacky solution because getUntil with a string delimeter will
        read the delimeter into its return value*/
        out.resize(out.length() - 3);

        return out;
    }

    token lexer::tokenizeBody()
    {
        std::size_t tokenStart = textStream.tellLine();
        std::string content = "";

        // are these kinds of loops bad practice?
        while (!textStream.eof())
        {
            content.append(textStream.getUntil('<'));
            if (eof())
                throw generateException("Body text with no terminal tag found during lexing", tokenStart, textStream.tellLine());
            else if (textStream.peekn(2) == "<!")
            {
                if (textStream.peekn(4) == "<!--")
                    textStream.ignore(std::numeric_limits<std::streamsize>::max(), "-->");
                else if (textStream.peekn(9) == "<![CDATA[")
                {
                    try
                    {
                        std::string charData = extractCharData();
                        content.append(charData);
                    }
                    catch (std::runtime_error e)
                    {
                        throw e;
                    }
                }
                else
                {
                    break;
                }
            }
            else
                break;
        }
        if (content.length() == 0)
        {
            return getNextToken();
        }
        return token(TEXT, content, tokenStart, textStream.tellLine());
    }

    void lexer::tokenizeStringLiteral()
    {
        std::size_t tokenStart = textStream.tellLine();
        std::string strData = textStream.getUntil('"');
        if (textStream.eof())
            throw generateException("unclosed string literal, expected \" found eof", tokenStart, textStream.tellLine());

        textStream.ignore();
        tokenBuffer.push_back(token(TEXT, strData, tokenStart, textStream.tellLine()));
        tokenBuffer.push_back(token(QUOTE, tokenStart, textStream.tellLine()));
    }

    bool identifierIllegal(char ch)
    {
        return (isspace(ch) ||
                ch == '<' ||
                ch == '>' ||
                ch == '=' ||
                ch == '!' ||
                ch == '"' ||
                ch == '#' ||
                ch == '$' ||
                ch == '%' ||
                ch == '&' ||
                ch == '\'' ||
                ch == '(' ||
                ch == ')' ||
                ch == '*' ||
                ch == '+' ||
                ch == ',' ||
                ch == '/' ||
                ch == ';' ||
                ch == '&' ||
                ch == '[' ||
                ch == ']' ||
                ch == '^' ||
                ch == '`' ||
                ch == '{' ||
                ch == '|' ||
                ch == '}' ||
                ch == '~');
    }

    token lexer::tokenizeIdentifier()
    {
        std::string text = textStream.getUntil(identifierIllegal);
        return token(TEXT, text, textStream.tellLine(), textStream.tellLine());
    }

    token lexer::getNextToken()
    {
        // we can source from our buffer until we run out
        ignoreUnread();
        if (tokenBuffer.size() > 0)
        {
            token out = tokenBuffer.front();
            tokenBuffer.pop_front();
            return out;
        }
        if (eof())
            return token(TERMINAL, textStream.tellLine(), textStream.tellLine());

        std::size_t currentLine = textStream.tellLine();
        switch (textStream.get())
        {
        case '!':
            return token(BANG, currentLine, currentLine);
        case '?':
            return token(QUESTION, currentLine, currentLine);
        case '<':
            if (textStream.peekn(8) == "[CDATA[")
            {
                textStream.unget();
                return tokenizeBody();
            }
            return token(LESSTHAN, currentLine, currentLine);
        case '/':
            return token(SLASH, currentLine, currentLine);
        case '>':
            try
            {
                ignoreUnread();
                tokenBuffer.push_back(tokenizeBody());
            }
            catch (std::runtime_error e)
            {
                throw e;
            }
            return token(GREATERTHAN, currentLine, currentLine);
        case '=':
            return token(EQUAL, currentLine, currentLine);
        case '"':
            tokenizeStringLiteral();
            return token(QUOTE, currentLine, currentLine);
        case '%':
            return token(PERCENT, currentLine, currentLine);
        case '[':
            return token(SQUARE_BRACKET_OPEN, currentLine, currentLine);
        case ']':
            return token(SQUARE_BRACKET_CLOSE, currentLine, currentLine);
        default:
            textStream.unget();
            return tokenizeIdentifier();
        }
    }

    bool lexer::eof() { return textStream.eof() && tokenBuffer.empty(); }

    // FIXME: this modifies the global state of the text stream object, is that ok?
    std::runtime_error lexer::generateException(const char *errBody, std::size_t tokenStart, std::size_t tokenEnd)
    {
        std::stringstream ss;
        ss << errBody << '\n';

        int i = tokenStart;
        int j = tokenEnd;
        for (; i <= j; i++)
        {
            textStream.seekLine(i);
            ss << i << " | " << textStream.getUntil('\n') << '\n';
        }

        return std::runtime_error(ss.str());
    }

    std::runtime_error lexer::generateException(const char *errBody, token t)
    {
        return generateException(errBody, t.getTokenStart(), t.getTokenEnd());
    }
}