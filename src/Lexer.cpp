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

    token lexer::tokenizeBody()
    {
        std::size_t tokenStart = textStream.tellLine();
        std::string content = "";

        // are these kinds of loops bad practice?
        while (!textStream.eof())
        {
            
            textStream.ignoreWS();

            content.append(textStream.getUntil('<'));
            if (eof())
                throw generateException("Body text with no terminal tag found during lexing",
                                        token(ERR, tokenStart, textStream.tellLine()));
            else if (textStream.peekn(2) == "<!")
            {
                if (textStream.peekn(4) == "<!--")
                    textStream.ignore(std::numeric_limits<std::streamsize>::max(), "-->");
                // else --> try and handle CDATA block
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
            throw generateException("attribute with name of length 0",
                                    token(ERR, textStream.tellLine(), textStream.tellLine()));
        else
            tokenBuffer.push_back(token(TEXT, attributeName, textStream.tellLine(), textStream.tellLine()));

        textStream.ignoreWS();
        if (textStream.get() != '=')
            throw generateException("Expected token =, following attribute declaration",
                                    token(ERR, textStream.tellLine(), textStream.tellLine()));
        else
            tokenBuffer.push_back(token(EQUAL, textStream.tellLine(), textStream.tellLine()));

        textStream.ignoreWS();
        if (textStream.get() != '"')
            throw generateException("Expected token \", following =",
                                    token(ERR, textStream.tellLine(), textStream.tellLine()));
        else
            tokenBuffer.push_back(token(QUOTE_OPEN, textStream.tellLine(), textStream.tellLine()));

        std::size_t tokenStart = textStream.tellLine();
        std::string attributeValue = textStream.getUntil([](char ch)
                                                         { return ch == '"'; });
        if (textStream.eof())
            throw generateException("Unterminated string literal",
                                    token(ERR, tokenStart, textStream.tellLine()));
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
            throw generateException("Tag with name of length 0",
                                    token(ERR, tokenStart, textStream.tellLine()));

        tokenBuffer.push_back(token(TEXT, tagName, tokenStart, textStream.tellLine()));


        textStream.ignoreWS();

        while (textStream.peek() != '>')
        {
            tokenizeInnerTagAttribute();
            if(textStream.eof())
                throw generateException("Expected tag to end with >, found eof",
                                        token(ERR, textStream.tellLine(), textStream.tellLine()));
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
            throw generateException("Expected tag to end with >",
                                    token(ERR, currentLine, textStream.tellLine()));
        else
            tokenBuffer.push_back(token(GREATERTHAN, tagName, currentLine, currentLine));
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
        catch (std::exception e)
        {
            throw e;
        }
    }

    bool lexer::eof() {return textStream.eof() && tokenBuffer.empty(); }

    // FIXME: this modifies the global state of the text stream object, is that ok?
    std::runtime_error lexer::generateException(const char *errBody, token t)
    {
        std::stringstream ss;
        ss << errBody << '\n';

        int i = t.getTokenStart();
        int j = t.getTokenEnd();
        for (; i <= j; i++)
        {
            textStream.seekLine(i);
            ss << i << " | " << textStream.getUntil('\n') << '\n';
        }

        return std::runtime_error(ss.str());
    }
}