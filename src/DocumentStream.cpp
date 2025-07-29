#include <DocumentStream.hpp>
#include <fstream>
#include <iostream>

// helper functions/macros
#define ISNEWLINE(ch) (ch == '\n' || ch == '\r')

namespace oxml
{
    void documentStream::incrementLineNumber()
    {
        lineNumber++;
        if (lineMapping.size() < lineNumber)
        {
            buf.get();
            lineMapping.push_back(buf.tellg());
            buf.unget();
        }
    }

    documentStream::documentStream(const char *path) : buf()
    {
        std::ifstream file{path};
        if (file.fail())
            throw std::runtime_error("File not found!");

        buf << file.rdbuf();
        file.close();
    }
    documentStream::documentStream(const std::ifstream &file) : buf()
    {
        buf << file.rdbuf();
    }

    const std::stringstream &documentStream::rdbuf()
    {
        return buf;
    }

    bool documentStream::eof()
    {
        return (buf.peek() == EOF) || buf.eof();
    }

    char documentStream::peek()
    {
        return buf.peek();
    }

    std::string documentStream::peekn(std::size_t n)
    {
        std::string out = "";
        std::streampos resetPos = buf.tellg();
        for (int i = 0; i < n && !eof(); i++)
        {
            char ch = buf.get();
            out.insert(out.end(), ch);
            if (ISNEWLINE(ch))
            {
                incrementLineNumber();
            }
        }

        buf.seekg(resetPos);
        return out;
    }

    void documentStream::ignore(std::streamsize n, int delimeter)
    {
        char ch = buf.peek();
        for (int i = 0; i < n && ch != delimeter && !eof(); i++)
        {
            if (ISNEWLINE(ch))
            {
                incrementLineNumber();
            }
            buf.ignore();
            ch = buf.peek();
        }
    }

    void documentStream::ignore(std::streamsize n, bool (*delimeter)(char))
    {
        char ch = buf.peek();
        for (int i = 0; i < n && !delimeter(ch) && !eof(); i++)
        {
            if (ISNEWLINE(ch))
            {
                incrementLineNumber();
            }
            buf.ignore();
            ch = buf.peek();
        }
    }

    void documentStream::ignore(){
        buf.ignore();
    }

    void documentStream::ignore(std::streamsize n, const char *delimeter)
    {
        int idx = 0;
        std::size_t len = strlen(delimeter);
        for (int i = 0; i < n && !eof() && idx < len; i++)
        {
            char ch = buf.get();
            if (ISNEWLINE(ch))
            {
                incrementLineNumber();
            }

            if (delimeter[idx] != ch)
            {
                idx = -1;
            }

            idx++;
        }
    }
    
    void documentStream::ignoreWS()
    {
        this->ignore(std::numeric_limits<std::streamsize>::max(), [](char ch)
                          { return !(isspace(ch)); });
    }

    std::size_t documentStream::tellLine()
    {
        return lineNumber;
    }

    bool documentStream::seekLine(std::size_t line)
    {
        if (lineMapping.size() >= line)
        {
            buf.seekg(lineMapping[line - 1]);
            lineNumber = line;
            return true;
        }

        buf.seekg(lineMapping.back());
        lineNumber = lineMapping.size();

        while (!eof())
        {
            char ch = buf.get();
            if (ISNEWLINE(ch))
            {
                incrementLineNumber();
            }

            if (lineNumber == line)
            {
                break;
            }
        }

        return (lineNumber == line);
    }

    char documentStream::get()
    {
        return buf.get();
    }

    std::string documentStream::getUntil(char delimeter)
    {
        char ch = buf.peek();
        std::string out = "";
        
        for (int i = 0; ch != delimeter && !eof(); i++)
        {
            out.insert(out.end(), ch);
            
            if (ISNEWLINE(ch))
            {
                incrementLineNumber();
            }

            buf.ignore();
            ch = buf.peek();
        }

        return out;
    }

    std::string documentStream::getUntil(bool (*delimeter)(char))
    {
        std::string out = "";
        char ch = buf.peek();
        while (!eof() && !delimeter(ch))
        {
            out.insert(out.end(), ch);
    
            if (ISNEWLINE(ch))
            {
                incrementLineNumber();
            }

            buf.ignore();
            ch = buf.peek();
        }
        return out;
    }

    std::string documentStream::getUntil(const char *delimeter)
    {
        std::string out = "";
        int idx = 0;
        std::size_t len = strlen(delimeter);
        for (int i = 0; !eof() && idx < len; i++)
        {
            char ch = buf.get();
            out.insert(out.end(), ch);

            if (ISNEWLINE(ch))
            {
                incrementLineNumber();
            }

            if (delimeter[idx] != ch)
            {
                idx = -1;
            }

            idx++;
        }

        return out;
    }
}