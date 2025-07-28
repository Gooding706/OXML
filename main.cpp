#include <iostream>
#include <Lexer.hpp>
#include <DocumentStream.hpp>

int main(int argc, char **argv)
{
    if (argc <= 1)
    {
        std::cerr << "Bad arguments please provide path\nUsage: ./build Path\n";
        return -1;
    }
    char *path = argv[1];

    std::cout << "hello world!, with path: " << path << '\n';

    // oxml::lexer lex(path);
    // while(!lex.eof())
    // {
    //     oxml::token t = lex.getNextToken();
    //     std::cout << t << " : " << t.getTokenContext().lineNumber << '\n';
    // }

    oxml::documentStream strm{path};
    std::cout << strm.tellLine() << strm.peekn(4) << '\n';
    strm.ignore(std::numeric_limits<std::streamsize>::max(), "</");
    std::cout << strm.tellLine()  << strm.peekn(4) << '\n';
    strm.seekLine(3);
    std::cout << strm.tellLine()  << strm.peekn(10) << '\n';
}