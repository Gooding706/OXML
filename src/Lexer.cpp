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
            }
            else
                break;
        }

        return token(TEXT, content, tokenStart, textStream.tellLine());
    }

    // TODO: is there a way to make this error handling less ugly?
    void lexer::tokenizeInnerTagAttribute()
    {
        textStream.ignoreWS();

        // expect TEXT
        std::string attributeName = textStream.getUntil([](char ch)
                                                        { return isspace(ch) || ch == '='; });
        if (attributeName.size() == 0)
            throw generateException("attribute with name of length 0", textStream.tellLine(), textStream.tellLine());
        else
            tokenBuffer.push_back(token(TEXT, attributeName, textStream.tellLine(), textStream.tellLine()));

        textStream.ignoreWS();
        if (textStream.get() != '=')
            throw generateException("Expected token =, following attribute declaration", textStream.tellLine(), textStream.tellLine());
        else
            tokenBuffer.push_back(token(EQUAL, textStream.tellLine(), textStream.tellLine()));

        textStream.ignoreWS();
        if (textStream.get() != '"')
            throw generateException("Expected token \", following =", textStream.tellLine(), textStream.tellLine());
        else
            tokenBuffer.push_back(token(QUOTE_OPEN, textStream.tellLine(), textStream.tellLine()));

        std::size_t tokenStart = textStream.tellLine();
        std::string attributeValue = textStream.getUntil([](char ch)
                                                         { return ch == '"'; });
        if (textStream.eof())
            throw generateException("Unterminated string literal", tokenStart, textStream.tellLine());
        else
            tokenBuffer.push_back(token(TEXT, attributeValue, textStream.tellLine(), textStream.tellLine()));

        textStream.get();
        tokenBuffer.push_back(token(QUOTE_CLOSE, textStream.tellLine(), textStream.tellLine()));
    }

    void lexer::tokenizeInnerTag()
    {
        // start with <
        textStream.ignore();
        tokenBuffer.push_back(token(LESSTHAN, textStream.tellLine(), textStream.tellLine()));

        std::size_t tokenStart = textStream.tellLine();
        std::string tagName = textStream.getUntil([](char ch)
                                                  { return (isspace(ch) || ch == '>'); });

        if (tagName.size() == 0)
            throw generateException("Tag with name of length 0", tokenStart, textStream.tellLine());

        tokenBuffer.push_back(token(TEXT, tagName, tokenStart, textStream.tellLine()));

        textStream.ignoreWS();

        while (textStream.peek() != '>')
        {
            tokenizeInnerTagAttribute();
            if (textStream.eof())
                throw generateException("Expected tag to end with >, found eof", textStream.tellLine(), textStream.tellLine());
        }

        textStream.ignore();
        tokenBuffer.push_back(token(GREATERTHAN, textStream.tellLine(), textStream.tellLine()));
    }

    void lexer::tokenizeClosingTag()
    {
        // we guarantee that the next 2 charcters are </
        textStream.ignore(2, '\0');
        std::size_t currentLine = textStream.tellLine();
        tokenBuffer.push_back(token(LESSTHAN, currentLine, currentLine));
        tokenBuffer.push_back(token(SLASH, currentLine, currentLine));

        std::string tagName = textStream.getUntil([](char ch)
                                                  { return (isspace(ch) || ch == '>'); });
        tokenBuffer.push_back(token(TEXT, tagName, currentLine, currentLine));
        textStream.ignoreWS();

        if (textStream.get() != '>')
            throw generateException("Expected tag to end with >", currentLine, textStream.tellLine());
        else
            tokenBuffer.push_back(token(GREATERTHAN, tagName, currentLine, currentLine));
    }

    void lexer::tokenizeIdentifier()
    {
        // we guarantee that the next 2 charcters are </
        textStream.ignore(2, '\0');
        std::size_t currentLine = textStream.tellLine();
        tokenBuffer.push_back(token(LESSTHAN, currentLine, currentLine));
        tokenBuffer.push_back(token(QUESTION, currentLine, currentLine));

        std::string identifierName = textStream.getUntil([](char ch){
            return (bool)isspace(ch);
        });
        if(identifierName.size() == 0)
            throw generateException("found identifier name of length 0", currentLine, currentLine);

        textStream.ignoreWS();
        while (textStream.peekn(2) != "?>")
        {
            tokenizeInnerTagAttribute();
            if (textStream.eof())
                throw generateException("Expected identifier to end with ?>, found eof", textStream.tellLine(), textStream.tellLine());
            textStream.ignoreWS();
        }
        
        textStream.ignore(2, '\0');
        currentLine = textStream.tellLine();
        tokenBuffer.push_back(token(QUESTION, currentLine, currentLine));
        tokenBuffer.push_back(token(GREATERTHAN, currentLine, currentLine));
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

        if (textStream.peek() == '<')
        {
            try
            {
                if (textStream.peekn(2) == "</")
                    tokenizeClosingTag();
                else if (textStream.peekn(9) == "<![CDATA[")
                {
                    // CDATA is exclusive to the body portion
                    token body = tokenizeBody();
                    return body;
                }
                else if (textStream.peekn(2) == "<?")
                    tokenizeIdentifier();
                else
                    tokenizeInnerTag();
            }
            catch (std::runtime_error e)
            {
                throw e;
            }
            // we call the function itself because, given success, the token buffer should have elements in it

            return getNextToken();
        }

        try
        {
            token body = tokenizeBody();
            return body;
        }
        catch (std::runtime_error e)
        {
            throw e;
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