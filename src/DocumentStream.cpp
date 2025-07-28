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
            lineMapping.push_back(buf.tellg());
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
        return buf.eof();
    }

    char documentStream::peek()
    {
        return buf.peek();
    }

    std::string documentStream::peekn(std::size_t n)
    {
        std::string out = "";
        std::streampos resetPos = buf.tellg();
        for (int i = 0; i < n && !buf.eof(); i++)
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
        char ch = buf.get();
        for (int i = 0; i < n && ch != delimeter && !buf.eof(); i++)
        {
            ch = buf.get();
            if (ISNEWLINE(ch))
            {
                incrementLineNumber();
            }
        }
    }

    void documentStream::ignore(std::streamsize n, bool (*delimeter)(char))
    {
        char ch = buf.get();
        for (int i = 0; i < n && !delimeter(ch) && !buf.eof(); i++)
        {
            ch = buf.get();
            if (ISNEWLINE(ch))
            {
                incrementLineNumber();
            }
        }
    }

    void documentStream::ignore(std::streamsize n, const char *delimeter)
    {
        int idx = 0;
        std::size_t len = strlen(delimeter);
        for (int i = 0; i < n && !buf.eof() && idx < len; i++)
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
        
        while (!buf.eof())
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
        char ch = buf.get();
        std::string out = "";
        for (int i = 0; ch != delimeter && !buf.eof(); i++)
        {
            out.insert(out.end(), ch);
            ch = buf.get();
            if (ISNEWLINE(ch))
            {
                incrementLineNumber();
            }
        }

        return out;
    }

    std::string documentStream::getUntil(bool (*delimeter)(char))
    {
        std::string out = "";
        char ch = buf.get();
        while (!buf.eof() && !delimeter(ch))
        {
            out.insert(out.end(), ch);
            ch = buf.get();

            if (ISNEWLINE(ch))
            {
                incrementLineNumber();
            }
        }
        return out;
    }

    std::string documentStream::getUntil(const char *delimeter)
    {
        std::string out = "";
        int idx = 0;
        std::size_t len = strlen(delimeter);
        for (int i = 0; !buf.eof() && idx < len; i++)
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