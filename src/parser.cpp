#include "parser.h"
#include <stdexcept>

Parser::Parser(std::vector<Token> tokens) : tokens(std::move(tokens)) {}

Token Parser::peek() const { return pos + 1 < tokens.size() ? tokens[pos + 1] : tokens.back(); }

Token Parser::cur() const { return tokens[pos]; }

bool Parser::at(TokenType t) const { return cur().type == t; }

void Parser::expect(TokenType t, const char* msg) {
    // ete spasvac token-y chka uremn error
    if (!match(t)) throw std::runtime_error(msg);
}

void Parser::expectColon() { expect(TokenType::COLON, "expected ':' at end of statement"); }

bool Parser::match(TokenType t) {
    // ete token-y hamapatasxanuma uremn ancnuma hajordin
    if (at(t)) { ++pos; return true; }
    return false;
}

VarScope Parser::parseScopePrefix() {
    // variable-i scope-i voroshum
    if (match(TokenType::GLOBAL)) return VarScope::Global;
    if (match(TokenType::LOCAL)) return VarScope::Local;
    if (match(TokenType::STATIC_VAR)) return VarScope::Static;
    return VarScope::Local;
}

std::shared_ptr<ASTNode> Parser::parseFactor() {
    // number parse
    if (at(TokenType::NUMBER)) {
        int v = std::stoi(cur().value);
        ++pos;
        return std::make_shared<NumberNode>(v);
    }

    // identifier kam function call
    if (at(TokenType::IDENTIFIER)) {
        std::string name = cur().value;
        ++pos;

        // function call
        if (at(TokenType::LPAREN)) {
            ++pos;
            std::vector<std::shared_ptr<ASTNode>> args;

            // argument-neri parse
            if (!at(TokenType::RPAREN)) {
                args.push_back(parseExpression());
                while (match(TokenType::COMMA))
                    args.push_back(parseExpression());
            }

            expect(TokenType::RPAREN, "expected ')'");

            auto call = std::make_shared<CallNode>();
            call->name = name;
            call->args = std::move(args);

            return call;
        }

        // simple variable
        return std::make_shared<VariableNode>(name);
    }
    // expression 
    if (match(TokenType::LPAREN)) {
        auto e = parseExpression();
        expect(TokenType::RPAREN, "expected ')'");
        return e;
    }

    throw std::runtime_error("expected expression");
}

std::shared_ptr<ASTNode> Parser::parseTerm() {
    auto node = parseFactor();

    // *, /, % operation-ner
    while (at(TokenType::MUL) || at(TokenType::DIV) || at(TokenType::MOD)) {
        std::string op = cur().value;
        ++pos;
        node = std::make_shared<BinaryNode>(op, node, parseFactor());
    }

    return node;
}

std::shared_ptr<ASTNode> Parser::parseExpression() {
    auto node = parseTerm();

    // + ev - operation-ner
    while (at(TokenType::PLUS) || at(TokenType::MINUS)) {
        std::string op = cur().value;
        ++pos;
        node = std::make_shared<BinaryNode>(op, node, parseTerm());
    }

    return node;
}

std::shared_ptr<ASTNode> Parser::parseComparison() {
    auto node = parseExpression();

    // hamematutyan operator-ner
    if (at(TokenType::EQ) || at(TokenType::NEQ) || at(TokenType::LT) ||
        at(TokenType::GT) || at(TokenType::LTE) || at(TokenType::GTE)) {
        std::string op = cur().value;
        ++pos;
        node = std::make_shared<BinaryNode>(op, node, parseExpression());
    }

    return node;
}

std::shared_ptr<BlockNode> Parser::parseBlock() {
    expect(TokenType::LBRACE, "expected '{'");

    auto block = std::make_shared<BlockNode>();

    // block-i statement-neri parse
    while (!at(TokenType::RBRACE) && !at(TokenType::END))
        block->statements.push_back(parseStatement());

    expect(TokenType::RBRACE, "expected '}'");

    return block;
}

std::shared_ptr<ASTNode> Parser::parseIf() {
    expect(TokenType::IF, "expected 'if'");
    expect(TokenType::LPAREN, "expected '(' after if");

    auto cond = parseComparison();

    expect(TokenType::RPAREN, "expected ')'");

    auto thenB = parseBlock();
   std::shared_ptr<BlockNode> elseB;

    // else block-i parse
    if (match(TokenType::ELSE))
        elseB = parseBlock();

    auto node = std::make_shared<IfNode>();

    node->condition = cond;
    node->thenBlock = thenB;
    node->elseBlock = elseB;

    return node;
}

std::shared_ptr<ASTNode> Parser::parseWhile() {
    expect(TokenType::WHILE, "expected 'while'");
    expect(TokenType::LPAREN, "expected '('");

    auto cond = parseComparison();
    expect(TokenType::RPAREN, "expected ')'");
    auto node = std::make_shared<WhileNode>();
    node->condition = cond;
    node->body = parseBlock();

    return node;
}

std::shared_ptr<ASTNode> Parser::parseDoWhile() {
    expect(TokenType::DO, "expected 'do'");

    auto body = parseBlock();

    expect(TokenType::WHILE, "expected 'while'");
    expect(TokenType::LPAREN, "expected '('");

    auto cond = parseComparison();
    
    expect(TokenType::RPAREN, "expected ')'");
    expectColon();

    auto node = std::make_shared<DoWhileNode>();
    node->body = body;
    node->condition = cond;

    return node;
}

std::shared_ptr<ASTNode> Parser::parseForInit() {
    // variable declaration
    if (at(TokenType::GLOBAL) || at(TokenType::LOCAL) || at(TokenType::STATIC_VAR)) {
        VarScope scope = parseScopePrefix();
        std::string name = cur().value;
        expect(TokenType::IDENTIFIER, "expected identifier");
        expect(TokenType::ASSIGN, "expected '='");
        auto init = parseExpression();
        return std::make_shared<VarDeclNode>(scope, name, init);
    }

    // assignment
    if (at(TokenType::IDENTIFIER) && peek().type == TokenType::ASSIGN) {
        std::string name = cur().value;
        pos += 2;
        auto val = parseExpression();
        return std::make_shared<AssignNode>(name, val);
    }

    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseForUpdate() {
    // for-i update masy petq e assignment lini
    if (at(TokenType::IDENTIFIER) && peek().type == TokenType::ASSIGN) {
        std::string name = cur().value;
        pos += 2;
        auto val = parseExpression();
        return std::make_shared<AssignNode>(name, val);
    }

    throw std::runtime_error("for update must be an assignment");
}

std::shared_ptr<ASTNode> Parser::parseFor() {
    expect(TokenType::FOR, "expected 'for'");
    expect(TokenType::LPAREN, "expected '('");

    auto init = parseForInit();
    expect(TokenType::COLON, "expected ':' after for init");
    auto cond = parseComparison();
    expect(TokenType::COLON, "expected ':' after for condition");

    auto update = parseForUpdate();
    expect(TokenType::RPAREN, "expected ')'");
    auto node = std::make_shared<ForNode>();
    node->init = init;
    node->condition = cond;
    node->update = update;
    node->body = parseBlock();

    return node;
}

std::shared_ptr<ASTNode> Parser::parseStatement() {
    // variable declaration
    if (at(TokenType::GLOBAL) || at(TokenType::LOCAL) || at(TokenType::STATIC_VAR)) {
        VarScope scope = parseScopePrefix();
        std::string name = cur().value;

        expect(TokenType::IDENTIFIER, "expected identifier");
        expect(TokenType::ASSIGN, "expected '='");
        auto init = parseExpression();
        expectColon();
        return std::make_shared<VarDeclNode>(scope, name, init);
    }

    // print(...)
    if (at(TokenType::PRINT)) {
        ++pos;
        expect(TokenType::LPAREN, "expected '('");
        auto e = parseExpression();
        expect(TokenType::RPAREN, "expected ')'");
        expectColon();
        return std::make_shared<PrintNode>(e);
    }

    // input(...)
    if (at(TokenType::INPUT)) {
        ++pos;
        expect(TokenType::LPAREN, "expected '('");
        std::string name = cur().value;
        expect(TokenType::IDENTIFIER, "expected identifier");
        expect(TokenType::RPAREN, "expected ')'");
        expectColon();
        return std::make_shared<InputNode>(name);
    }

    // return statement
    if (at(TokenType::RETURN)) {
        ++pos;
        auto v = parseExpression();
        expectColon();
        return std::make_shared<ReturnNode>(v);
    }

    if (at(TokenType::IF)) return parseIf();
    if (at(TokenType::WHILE)) return parseWhile();
    if (at(TokenType::DO)) return parseDoWhile();
    if (at(TokenType::FOR)) return parseFor();

    // function call statement
    if (at(TokenType::IDENTIFIER) && peek().type == TokenType::LPAREN) {
        auto call = std::dynamic_pointer_cast<CallNode>(parseFactor());
        expectColon();
        return call;
    }

    // assignment statement
    if (at(TokenType::IDENTIFIER) && peek().type == TokenType::ASSIGN) {
        std::string name = cur().value;
        pos += 2;
        auto val = parseExpression();
        expectColon();
        return std::make_shared<AssignNode>(name, val);
    }

    throw std::runtime_error("unexpected statement");
}

std::shared_ptr<FunctionNode> Parser::parseFunction() {
    expect(TokenType::FUNC, "expected 'func'");
    std::string name = cur().value;
    expect(TokenType::IDENTIFIER, "expected function name");
    expect(TokenType::LPAREN, "expected '('");
    std::vector<std::string> params;

    // function parameter-eri parse
    if (!at(TokenType::RPAREN)) {
        params.push_back(cur().value);
        expect(TokenType::IDENTIFIER, "expected param");
        while (match(TokenType::COMMA)) {
            params.push_back(cur().value);

            expect(TokenType::IDENTIFIER, "expected param");
        }
    }

    expect(TokenType::RPAREN, "expected ')'");
    auto body = parseBlock();
    auto fn = std::make_shared<FunctionNode>();
    fn->name = name;
    fn->params = params;
    fn->body = body;

    return fn;
}

std::shared_ptr<ASTNode> Parser::parseTopLevel() {
    // function parse
    if (at(TokenType::FUNC)) return parseFunction();

    // global/static variable parse
    if (at(TokenType::GLOBAL) || at(TokenType::STATIC_VAR)) {
        VarScope scope = parseScopePrefix();
        std::string name = cur().value;
        expect(TokenType::IDENTIFIER, "expected identifier");
        expect(TokenType::ASSIGN, "expected '='");
        auto init = parseExpression();
        expectColon();
        return std::make_shared<VarDeclNode>(scope, name, init);
    }

    throw std::runtime_error("expected func, global, or static at top level");
}

std::shared_ptr<ProgramNode> Parser::parseProgram() {
    auto prog = std::make_shared<ProgramNode>();

    // parse anum e bolor top-level item-nery
    while (!at(TokenType::END))
        prog->items.push_back(parseTopLevel());

    return prog;
}