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

    oxml::lexer lex(path);
    while (!lex.eof())
    {
        try
        {
            oxml::token t = lex.getNextToken();
            std::cout << t << '\n';
        }
        catch (std::runtime_error e)
        {
            std::cerr << e.what();
        }
    }
}