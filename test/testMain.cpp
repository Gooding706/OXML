#include <DocumentStream.hpp>
#include <Lexer.hpp>
#include <iostream>

#define assertOr(e, str) if(!(e)) std::cerr << str << '\n';

bool testPeekN(oxml::documentStream& strm, std::size_t n, const char* expect){
    strm.seekLine(1);
    return strm.peekn(n) == expect;
}

bool testIgnore(oxml::documentStream& strm, char d, char expect){
    strm.seekLine(1);
    strm.ignore(std::numeric_limits<std::streamsize>::max(), d);
    return strm.peek() == expect;
}

bool testIgnore(oxml::documentStream& strm, bool (*d)(char), char expect){
    strm.seekLine(1);
    strm.ignore(std::numeric_limits<std::streamsize>::max(), d);
    return strm.peek() == expect;
}

bool testIgnore(oxml::documentStream& strm, const char* d, char expect){
    strm.seekLine(1);
    strm.ignore(std::numeric_limits<std::streamsize>::max(), d);
    return strm.peek() == expect;
}

bool testIgnore(oxml::documentStream& strm, char expect){
    strm.seekLine(1);
    strm.ignore();
    return strm.peek() == expect;
}

bool testTellLine(oxml::documentStream& strm, std::size_t expect){
   return  strm.tellLine() == expect;
}

bool testSeekLine(oxml::documentStream& strm, std::size_t lineNum, const char * expectLineContent){
    strm.seekLine(lineNum);
    return strm.getUntil('\n') == expectLineContent;
}

bool testGet(oxml::documentStream& strm, char expect){
    strm.seekLine(1);
    return strm.get() == expect;
}

bool testGetUntil(oxml::documentStream& strm, char d, const char* expect){
    strm.seekLine(1);
    return strm.getUntil(d) == expect;
}

int main(){
    oxml::documentStream strm{"res/lines.txt"};
    assertOr(testPeekN(strm, 4, "this"), "failed peekn");
    assertOr(testIgnore(strm, ' ', ' '), "failed ignore with char delimeter");
    assertOr(testIgnore(strm, [](char ch){return (bool)isnumber(ch);}, '1'), "failed ignore with func delimeter");
    assertOr(testIgnore(strm, "this is line ", '1'), "failed ignore with string delimeter");
    assertOr(testIgnore(strm, 'h'), "failed ignore with no delimeter");
    assertOr(testTellLine(strm, 1), "failed test tell line");
    assertOr(testSeekLine(strm, 4, "this is line 4"), "failed seekline");
    assertOr(testGet(strm, 't'), "failed get");
    assertOr(testGetUntil(strm, '\n', "this is line 1"), "failed get until with char delimeter");

    while (!strm.eof())
    {
       std::cout << strm.getUntil('\n') << '\n';
       strm.ignore();
    }
    
    int i = 3;
    int j = 6;
    for(; i <= j; i++){
        strm.seekLine(i);
        std::cout << strm.getUntil('\n') << '\n';
    }
}