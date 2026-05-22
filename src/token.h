#pragma once
#include <string>

enum class TokenType {
    NUMBER,
    IDENTIFIER,
    FUNC,
    IF, ELIF, ELSE,
    FOR,
    WHILE, DO, RETURN,
    PRINT, INPUT,
    GLOBAL, LOCAL, STATIC_VAR,
    PLUS, MINUS, MUL, DIV, MOD,
    ASSIGN,
    EQ, NEQ, LT, GT, LTE, GTE,
    LPAREN, RPAREN,
    LBRACE, RBRACE,
    COMMA, COLON,
    END
};

struct Token {
    TokenType type;
    std::string value;
    Token(TokenType t, std::string v = "") : type(t), value(std::move(v)) {}
};
