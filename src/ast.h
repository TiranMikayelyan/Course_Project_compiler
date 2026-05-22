#pragma once 

#include <memory> // shared_ptr hamar
#include <string> 
#include <vector> 

enum class VarScope { Local, Global, Static }; // popoxakanneri scope-eri tesakner

struct ASTNode {
    virtual ~ASTNode() = default; // bazayin destructor vor jisht jnjen zangvac node-ery
};

struct NumberNode : ASTNode {
    int value; 
    explicit NumberNode(int v) : value(v) {} 
};

struct VariableNode : ASTNode {
    std::string name; 
    explicit VariableNode(std::string n) : name(std::move(n)) {} 
};

struct BinaryNode : ASTNode {
    std::string op; // gortsoghutyuny (+, -, *, /)
    std::shared_ptr<ASTNode> left, right; //stacox aj ev dzakh node-ery

    BinaryNode(std::string o, std::shared_ptr<ASTNode> l, std::shared_ptr<ASTNode> r)
        : op(std::move(o)), left(std::move(l)), right(std::move(r)) {} 
};

struct VarDeclNode : ASTNode {
    VarScope scope; 
    std::string name; 
    std::shared_ptr<ASTNode> init; // skzbakan arjeqy

    VarDeclNode(VarScope s, std::string n, std::shared_ptr<ASTNode> i)
        : scope(s), name(std::move(n)), init(std::move(i)) {} 
};

struct AssignNode : ASTNode {
    std::string name; // popoxakani anuny
    std::shared_ptr<ASTNode> value; // veragrvox arjeqy

    AssignNode(std::string n, std::shared_ptr<ASTNode> v)
        : name(std::move(n)), value(std::move(v)) {} 
};

struct PrintNode : ASTNode {
    std::shared_ptr<ASTNode> expr;

    explicit PrintNode(std::shared_ptr<ASTNode> e)
        : expr(std::move(e)) {} 
};

struct InputNode : ASTNode {
    std::string name; 

    explicit InputNode(std::string n)
        : name(std::move(n)) {} 
};

struct ReturnNode : ASTNode {
    std::shared_ptr<ASTNode> value; 

    explicit ReturnNode(std::shared_ptr<ASTNode> v)
        : value(std::move(v)) {} 
};

struct CallNode : ASTNode {
    std::string name;
    std::vector<std::shared_ptr<ASTNode>> args; 
};

struct BlockNode : ASTNode {
    std::vector<std::shared_ptr<ASTNode>> statements; // hramanneri block
};

struct ForNode : ASTNode {
    std::shared_ptr<ASTNode> init; // for-i skzbakan mas
    std::shared_ptr<ASTNode> condition; // cikli paymany
    std::shared_ptr<ASTNode> update; // amen ciklic heto poxvox mas
    std::shared_ptr<BlockNode> body; // for-i body
};

struct IfNode : ASTNode {
    std::shared_ptr<ASTNode> condition; // if-i paymany
    std::shared_ptr<BlockNode> thenBlock; // ete paymany chisht
    std::shared_ptr<BlockNode> elseBlock; // hakarak depqum
};

struct WhileNode : ASTNode {
    std::shared_ptr<ASTNode> condition; // while-i paymany
    std::shared_ptr<BlockNode> body; // cikli body
};

struct DoWhileNode : ASTNode {
    std::shared_ptr<BlockNode> body; // do-while-i body
    std::shared_ptr<ASTNode> condition; // cikli paymany
};

struct FunctionNode : ASTNode {
    std::string name; // funkciayi anuny
    std::vector<std::string> params; // parametreri cucak
    std::shared_ptr<BlockNode> body; // funkciayi body
};

struct ProgramNode : ASTNode {
    std::vector<std::shared_ptr<ASTNode>> items; // cragri bolor masery
};