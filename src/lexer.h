#pragma once 

#include "token.h" 
#include <string>
#include <vector>

class Lexer {
public:

    explicit Lexer(std::string src); // constructor source text-ov

    std::vector<Token> tokenize(); // source text-y token-neri a veracum

private:

    std::string src; // mutq text-y
    size_t pos = 0; // himikva position-y text-i mej

    char cur() const; // veradardznuma himikva symbol-y
    void advance(); // ancnuma hajord symbolin
    void skipWs(); // baca toxnum whitespace-nery
    void skipComment(); // baca toxnum comment-nery
    bool match(const char* s); // stuguma toxy hamapatasxanum e te che
};