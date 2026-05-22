#include "lexer.h"
#include <cctype>
#include <unordered_map>

Lexer::Lexer(std::string src) : src(std::move(src)) {}

char Lexer::cur() const { return pos < src.size() ? src[pos] : '\0'; }

void Lexer::advance() { if (pos < src.size()) ++pos; }

void Lexer::skipWs() {
    while (isspace(static_cast<unsigned char>(cur()))) advance();
}

void Lexer::skipComment() {

    // // comment-i skip
    if (cur() == '/' && pos + 1 < src.size() && src[pos + 1] == '/') {
        while (cur() && cur() != '\n') advance();
    }
}

bool Lexer::match(const char* s) {

    // stugum e string-i hamapatasxanutyuny
    size_t i = 0;

    while (s[i]) {
        if (pos + i >= src.size() || src[pos + i] != s[i]) return false;
        ++i;
    }

    pos += i;
    return true;
}

// reserved keyword-ner
static const std::unordered_map<std::string, TokenType> KEYWORDS = {
    {"func", TokenType::FUNC},
    {"if", TokenType::IF}, {"elif", TokenType::ELIF}, {"else", TokenType::ELSE},
    {"for", TokenType::FOR},
    {"while", TokenType::WHILE}, {"do", TokenType::DO},
    {"return", TokenType::RETURN},
    {"print", TokenType::PRINT}, {"input", TokenType::INPUT},
    {"global", TokenType::GLOBAL}, {"local", TokenType::LOCAL},
    {"static", TokenType::STATIC_VAR},
};

std::vector<Token> Lexer::tokenize() {

    std::vector<Token> out;

    while (cur()) {

        skipWs();
        skipComment();

        if (!cur()) break;

        // number token
        if (isdigit(static_cast<unsigned char>(cur()))) {

            std::string n;

            while (isdigit(static_cast<unsigned char>(cur()))) {
                n += cur();
                advance();
            }

            out.emplace_back(TokenType::NUMBER, n);

        }

        // identifier kam keyword
        else if (isalpha(static_cast<unsigned char>(cur())) || cur() == '_') {

            std::string w;

            while (isalnum(static_cast<unsigned char>(cur())) || cur() == '_') {
                w += cur();
                advance();
            }

            auto it = KEYWORDS.find(w);

            // ete keyworda uremn keyword token
            // ete che uremn identifier
            out.emplace_back(it != KEYWORDS.end() ? it->second : TokenType::IDENTIFIER, w);

        }

        else if (match("==")) out.emplace_back(TokenType::EQ, "==");
        else if (match("!=")) out.emplace_back(TokenType::NEQ, "!=");
        else if (match("<=")) out.emplace_back(TokenType::LTE, "<=");
        else if (match(">=")) out.emplace_back(TokenType::GTE, ">=");

        else if (cur() == '+') { out.emplace_back(TokenType::PLUS, "+"); advance(); }
        else if (cur() == '-') { out.emplace_back(TokenType::MINUS, "-"); advance(); }
        else if (cur() == '*') { out.emplace_back(TokenType::MUL, "*"); advance(); }
        else if (cur() == '/') { out.emplace_back(TokenType::DIV, "/"); advance(); }
        else if (cur() == '%') { out.emplace_back(TokenType::MOD, "%"); advance(); }
        else if (cur() == '=') { out.emplace_back(TokenType::ASSIGN, "="); advance(); }
        else if (cur() == '<') { out.emplace_back(TokenType::LT, "<"); advance(); }
        else if (cur() == '>') { out.emplace_back(TokenType::GT, ">"); advance(); }
        else if (cur() == '(') { out.emplace_back(TokenType::LPAREN, "("); advance(); }
        else if (cur() == ')') { out.emplace_back(TokenType::RPAREN, ")"); advance(); }
        else if (cur() == '{') { out.emplace_back(TokenType::LBRACE, "{"); advance(); }
        else if (cur() == '}') { out.emplace_back(TokenType::RBRACE, "}"); advance(); }
        else if (cur() == ',') { out.emplace_back(TokenType::COMMA, ","); advance(); }
        else if (cur() == ':') { out.emplace_back(TokenType::COLON, ":"); advance(); }

        else advance(); //  ete chgiti skip
    }

    // file-i avarti token
    out.emplace_back(TokenType::END, "");

    return out;
}