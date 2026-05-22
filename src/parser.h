#pragma once
#include "ast.h"
#include "token.h"
#include <memory>
#include <vector>

class Parser {
public:
    explicit Parser(std::vector<Token> tokens); // token-neri save anelu hamar
    std::shared_ptr<ProgramNode> parseProgram();

private:
    std::vector<Token> tokens;
    size_t pos = 0;

    Token peek() const;
    Token cur() const;
    bool at(TokenType t) const; // stugum e himikva token-i tesaky
    void expect(TokenType t, const char* msg);
    void expectColon(); // ete hamapatasxanum e -> ancnum e hajordin
    bool match(TokenType t); // ete hamapatasxanum e -> ancnum e hajordin

    std::shared_ptr<ASTNode> parseTopLevel();
    std::shared_ptr<FunctionNode> parseFunction();
    std::shared_ptr<BlockNode> parseBlock();
    std::shared_ptr<ASTNode> parseStatement();
    std::shared_ptr<ASTNode> parseIf();
    std::shared_ptr<ASTNode> parseWhile();
    std::shared_ptr<ASTNode> parseDoWhile();
    std::shared_ptr<ASTNode> parseFor();
    std::shared_ptr<ASTNode> parseForInit();
    std::shared_ptr<ASTNode> parseForUpdate();
    VarScope parseScopePrefix();  // global/local/static scope-i parse

    std::shared_ptr<ASTNode> parseExpression();
    std::shared_ptr<ASTNode> parseComparison();
    std::shared_ptr<ASTNode> parseTerm();
    std::shared_ptr<ASTNode> parseFactor();
};
