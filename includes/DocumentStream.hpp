#pragma once
#include <sstream>

namespace oxml{
    //A thin wrapper of std::stringStream in order to keep track of line number information
    class documentStream{
        public: 
            documentStream(const char* path);
            documentStream(const std::ifstream& file);

            const std::stringstream& rdbuf();
            bool eof();
            char peek();

            //peak n character and return the corresponding string
            std::string peekn(std::size_t n);

            /*these functions are overloaded to provide three definitons of delimeter,
            this is to fit the needs of various lexing operations
            */
            void ignore(std::streamsize n, int delimeter);
            void ignore(std::streamsize n, bool(*delimeter)(char));
            void ignore(std::streamsize n, const char* delimeter);
            void ignore();

            void ignoreWS();

            std::size_t tellLine();
            //tries to jump to line, if the line is out of bounds or otherwise can't be found returns false
            bool seekLine(std::size_t line);

            char get();
            std::string getUntil(char delimeter);
            std::string getUntil(bool(*delimeter)(char));
            std::string getUntil(const char* delimeter);

        private:
            std::stringstream buf;

            std::size_t lineNumber{1};

            //vector that maps such that lineMapping[line-1] = std::streampos of the start of line
            std::vector<std::streampos> lineMapping{0};

        private:
            void incrementLineNumber();
    };
}