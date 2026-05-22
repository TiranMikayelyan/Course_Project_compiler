#include "codegen.h" 
#include "isa.h" 
#include <stdexcept> // exception-neri hamar

static int16_t relOff(size_t insnPc, size_t targetPc) {
    // hashvum e relative jump-i heravorutyuny
    return static_cast<int16_t>(targetPc) - static_cast<int16_t>(insnPc + 1);
}

size_t CodeGen::emit(uint32_t w) {
    code_.push_back(w); // instruction-y avelacnum e code-i mej
    return code_.size() - 1; // veradardznum e instruction-i index-y
}

size_t CodeGen::pc() const {
    return code_.size(); // himikva instruction-i hascen
}

void CodeGen::patch(size_t at, uint32_t w) {
    code_[at] = w; // poxarum e instruction-y tvyal texum
}

int CodeGen::evalConstExpr(const std::shared_ptr<ASTNode>& node) {
    // constant expression-i hashvum

    if (auto n = std::dynamic_pointer_cast<NumberNode>(node))
        return n->value;

    if (auto b = std::dynamic_pointer_cast<BinaryNode>(node)) {

        int l = evalConstExpr(b->left); // dzakh masi arjeqy
        int r = evalConstExpr(b->right); // ?? masi arjeqy

        if (b->op == "+") return l + r;
        if (b->op == "-") return l - r;
        if (b->op == "*") return l * r;
        if (b->op == "/") return r == 0 ? 0 : l / r; // bajanum 0-i vra stugumov
    }

    return 0;
}

void CodeGen::analyzeBlock(const std::shared_ptr<BlockNode>& block) {

    // ancnum e block-i bolor statement-nerov
    for (auto& st : block->statements) {

        if (auto d = std::dynamic_pointer_cast<VarDeclNode>(st)) {

            // default local e
            StorageKind k = StorageKind::Local;

            if (d->scope == VarScope::Global)
                k = StorageKind::Global;

            else if (d->scope == VarScope::Static)
                k = StorageKind::Static;

            // haytararum e popoxakany symbol table-um
            syms_.declareLocal(d->name, k, evalConstExpr(d->init));

        }
        else if (auto iw = std::dynamic_pointer_cast<IfNode>(st)) {

            analyzeBlock(iw->thenBlock); // then block-i analiz

            if (iw->elseBlock)
                analyzeBlock(iw->elseBlock); // else block-i analiz

        }
        else if (auto ww = std::dynamic_pointer_cast<WhileNode>(st)) {

            analyzeBlock(ww->body);

        }
        else if (auto dw = std::dynamic_pointer_cast<DoWhileNode>(st)) {

            analyzeBlock(dw->body);

        }
        else if (auto fr = std::dynamic_pointer_cast<ForNode>(st)) {

            // for-i init masi analiz
            if (fr->init) {

                if (auto d = std::dynamic_pointer_cast<VarDeclNode>(fr->init)) {

                    StorageKind k = StorageKind::Local;

                    if (d->scope == VarScope::Global)
                        k = StorageKind::Global;

                    else if (d->scope == VarScope::Static)
                        k = StorageKind::Static;

                    syms_.declareLocal(d->name, k, evalConstExpr(d->init));
                }
            }

            analyzeBlock(fr->body); // for body-i analiz
        }
    }
}

void CodeGen::analyze(const std::shared_ptr<ProgramNode>& prog) {

    // arajin ancum - globalner ev funkcianer
    for (auto& item : prog->items) {

        if (auto d = std::dynamic_pointer_cast<VarDeclNode>(item)) {

            StorageKind k =
                d->scope == VarScope::Static
                ? StorageKind::Static
                : StorageKind::Global;

            syms_.declareGlobal(d->name, k, evalConstExpr(d->init));

        }
        else if (auto f = std::dynamic_pointer_cast<FunctionNode>(item)) {

            // funkcian grancum e
            syms_.registerFunction(f->name, f->params.size());
        }
    }

    // erkrord ancum - funkcianeri body-ner
    for (auto& item : prog->items) {

        if (auto f = std::dynamic_pointer_cast<FunctionNode>(item)) {

            syms_.beginFunction(f->name);

            int reg = 2;

            // parametrern enq register-nerin k?um
            for (auto& p : f->params) {
                syms_.declareParam(p, reg);
                ++reg;
            }

            analyzeBlock(f->body);

            syms_.endFunction();
        }
    }
}

uint8_t CodeGen::compileExpr(const std::shared_ptr<ASTNode>& node, uint8_t out) {

    // tiv
    if (auto n = std::dynamic_pointer_cast<NumberNode>(node)) {

        int v = n->value;

        emit(isa::encode(
            isa::MOV_IMM,
            out,
            0,
            0,
            static_cast<int16_t>(v & 0xFFF)
        ));

        return out;
    }

    // popoxakan
    if (auto v = std::dynamic_pointer_cast<VariableNode>(node)) {

        Symbol* s = syms_.lookup(v->name);

        if (!s)
            throw std::runtime_error("undefined variable: " + v->name);

        // parameter register-ic
        if (s->kind == StorageKind::Param) {

            if (out != static_cast<uint8_t>(s->regIndex))

                emit(isa::encode(
                    isa::MOV_REG,
                    out,
                    static_cast<uint8_t>(s->regIndex),
                    0,
                    0
                ));

        }
        // global/static memory-ic
        else if (
            s->kind == StorageKind::Global ||
            s->kind == StorageKind::Static
            )

            emit(isa::encode(
                isa::LOAD_DATA,
                out,
                0,
                0,
                static_cast<int16_t>(s->dataOffset)
            ));

        // local stack-ic
        else

            emit(isa::encode(
                isa::LOAD_LOCAL,
                out,
                0,
                0,
                static_cast<int16_t>(s->stackOffset)
            ));

        return out;
    }

    // funkciai kanch
    if (auto c = std::dynamic_pointer_cast<CallNode>(node)) {

        if (!syms_.hasFunction(c->name))
            throw std::runtime_error("undefined function: " + c->name);

        int reg = 2 + static_cast<int>(c->args.size()) - 1;

        // argumentneri compile
        for (auto it = c->args.rbegin(); it != c->args.rend(); ++it) {

            compileExpr(*it, static_cast<uint8_t>(reg));

            --reg;
        }

        emit(isa::encode(isa::PUSH_BP, 0, 0, 0, 0)); // pahpanum e BP
        emit(isa::encode(isa::PUSH_NEXT_PC, 0, 0, 0, 0)); // pahpanum e return hascen

        size_t site =
            emit(isa::encode(isa::CALL, 0, 0, 0, 0));

        // heto patch anelu hamar
        callPatches_.push_back({ site, c->name });

        // return value-n texapoxum e out register
        emit(isa::encode(isa::MOV_REG, out, 1, 0, 0));

        return out;
    }

    // binary expression
    if (auto b = std::dynamic_pointer_cast<BinaryNode>(node)) {

        // hamematutyunnery argelel expression context-um
        if (
            b->op == "==" ||
            b->op == "!=" ||
            b->op == "<" ||
            b->op == ">" ||
            b->op == "<=" ||
            b->op == ">="
            )
            throw std::runtime_error(
                "comparison not allowed in expression context"
            );

        uint8_t l = compileExpr(b->left, out);

        uint8_t r =
            compileExpr(
                b->right,
                (out == 1) ? 2 : 1
            );

        if (b->op == "+")
            emit(isa::encode(isa::ADD, out, l, r, 0));

        else if (b->op == "-")
            emit(isa::encode(isa::SUB, out, l, r, 0));

        else if (b->op == "*")
            emit(isa::encode(isa::MUL, out, l, r, 0));

        else if (b->op == "/")
            emit(isa::encode(isa::DIV, out, l, r, 0));

        return out;
    }

    throw std::runtime_error("invalid expression");
}

void CodeGen::compileBranch(const std::string& op, size_t at, size_t truePc, size_t falsePc) {
    // hamematutyunneri hamar branch instrukcianer
    isa::Op br = isa::BR_EQ;
    if (op == "==") br = isa::BR_EQ;
    else if (op == "!=") br = isa::BR_NE;
    else if (op == "<") br = isa::BR_LT;
    else if (op == ">") br = isa::BR_GT;
    else if (op == "<=") br = isa::BR_LTE;
    else if (op == ">=") br = isa::BR_GTE;

    patch(at, isa::encode(br, 0, 0, 0, relOff(at, truePc)));
    patch(at + 1, isa::encode(isa::JMP, 0, 0, 0, relOff(at + 1, falsePc)));
}

void CodeGen::compileStmt(const std::shared_ptr<ASTNode>& st) {
    // popoxakani haytararutyun
    if (auto d = std::dynamic_pointer_cast<VarDeclNode>(st)) {
        uint8_t r = compileExpr(d->init, 1);
        Symbol* s = syms_.lookup(d->name);
        if (!s) throw std::runtime_error("var not found: " + d->name);

        if (s->kind == StorageKind::Global || s->kind == StorageKind::Static)
            emit(isa::encode(isa::STORE_DATA, 0, 0, r, static_cast<int16_t>(s->dataOffset)));
        else
            emit(isa::encode(isa::STORE_LOCAL, 0, 0, r, static_cast<int16_t>(s->stackOffset)));
        return;
    }

    // nakhatesvac arjeq
    if (auto a = std::dynamic_pointer_cast<AssignNode>(st)) {
        uint8_t r = compileExpr(a->value, 1);
        Symbol* s = syms_.lookup(a->name);
        if (!s) throw std::runtime_error("undefined: " + a->name);

        if (s->kind == StorageKind::Global || s->kind == StorageKind::Static)
            emit(isa::encode(isa::STORE_DATA, 0, 0, r, static_cast<int16_t>(s->dataOffset)));
        else
            emit(isa::encode(isa::STORE_LOCAL, 0, 0, r, static_cast<int16_t>(s->stackOffset)));
        return;
    }

    // print statement
    if (auto p = std::dynamic_pointer_cast<PrintNode>(st)) {
        compileExpr(p->expr, 1);
        emit(isa::encode(isa::PRINT, 0, 1, 0, 0));
        return;
    }

    // input statement
    if (auto in = std::dynamic_pointer_cast<InputNode>(st)) {
        emit(isa::encode(isa::INPUT, 1, 0, 0, 0));
        Symbol* s = syms_.lookup(in->name);
        if (!s) throw std::runtime_error("undefined: " + in->name);

        if (s->kind == StorageKind::Global || s->kind == StorageKind::Static)
            emit(isa::encode(isa::STORE_DATA, 0, 0, 1, static_cast<int16_t>(s->dataOffset)));
        else
            emit(isa::encode(isa::STORE_LOCAL, 0, 0, 1, static_cast<int16_t>(s->stackOffset)));
        return;
    }

    // return statement
    if (auto r = std::dynamic_pointer_cast<ReturnNode>(st)) {
        compileExpr(r->value, 1);
        emit(isa::encode(isa::RET, 0, 1, 0, 0));
        return;
    }

    // funkciai kanch statement
    if (auto c = std::dynamic_pointer_cast<CallNode>(st)) {
        compileExpr(c, 1);
        return;
    }

    // if statement
    if (auto iw = std::dynamic_pointer_cast<IfNode>(st)) {
        size_t brAt = 0, jmpAt = 0;

        if (auto b = std::dynamic_pointer_cast<BinaryNode>(iw->condition)) {
            compileExpr(b->left, 2);
            compileExpr(b->right, 3);
            emit(isa::encode(isa::CMP, 0, 2, 3, 0));
            brAt = emit(isa::encode(isa::BR_EQ, 0, 0, 0, 0));
            jmpAt = emit(isa::encode(isa::JMP, 0, 0, 0, 0));

            size_t thenPc = pc();
            compileBlock(iw->thenBlock);

            if (iw->elseBlock) {
                size_t jmpEnd = emit(isa::encode(isa::JMP, 0, 0, 0, 0));
                size_t elsePc = pc();
                compileBlock(iw->elseBlock);
                size_t endPc = pc();
                compileBranch(b->op, brAt, thenPc, elsePc);
                patch(jmpEnd, isa::encode(isa::JMP, 0, 0, 0, relOff(jmpEnd, endPc)));
            } else {
                size_t endPc = pc();
                compileBranch(b->op, brAt, thenPc, endPc);
            }
            (void)jmpAt;
        } else {
            compileExpr(iw->condition, 1);
            emit(isa::encode(isa::CMP, 0, 1, 0, 0));
            brAt = emit(isa::encode(isa::BR_NE, 0, 0, 0, 0));
            jmpAt = emit(isa::encode(isa::JMP, 0, 0, 0, 0));
            size_t thenPc = pc();
            compileBlock(iw->thenBlock);
            size_t endPc = pc();
            compileBranch("!=", brAt, thenPc, endPc);
            (void)jmpAt;
        }
        return;
    }

    // while loop
    if (auto ww = std::dynamic_pointer_cast<WhileNode>(st)) {
        if (auto b = std::dynamic_pointer_cast<BinaryNode>(ww->condition)) {
            size_t head = pc();
            compileExpr(b->left, 2);
            compileExpr(b->right, 3);
            emit(isa::encode(isa::CMP, 0, 2, 3, 0));
            size_t brAt = emit(isa::encode(isa::BR_EQ, 0, 0, 0, 0));
            size_t jmpExit = emit(isa::encode(isa::JMP, 0, 0, 0, 0));
            size_t bodyPc = pc();
            compileBlock(ww->body);
            emit(isa::encode(isa::JMP, 0, 0, 0, relOff(pc(), head)));
            size_t exitPc = pc();
            compileBranch(b->op, brAt, bodyPc, exitPc);
            patch(jmpExit, isa::encode(isa::JMP, 0, 0, 0, relOff(jmpExit, exitPc)));
        }
        return;
    }

    // do-while loop
    if (auto dw = std::dynamic_pointer_cast<DoWhileNode>(st)) {
        size_t head = pc();
        compileBlock(dw->body);
        if (auto b = std::dynamic_pointer_cast<BinaryNode>(dw->condition)) {
            compileExpr(b->left, 2);
            compileExpr(b->right, 3);
            emit(isa::encode(isa::CMP, 0, 2, 3, 0));
            size_t brAt = emit(isa::encode(isa::BR_EQ, 0, 0, 0, 0));
            emit(isa::encode(isa::JMP, 0, 0, 0, 0));
            size_t exitPc = pc();
            compileBranch(b->op, brAt, head, exitPc);
        }
        return;
    }

    // for loop
    if (auto fr = std::dynamic_pointer_cast<ForNode>(st)) {
        if (fr->init) compileStmt(fr->init);
        if (auto b = std::dynamic_pointer_cast<BinaryNode>(fr->condition)) {
            size_t head = pc();
            compileExpr(b->left, 2);
            compileExpr(b->right, 3);
            emit(isa::encode(isa::CMP, 0, 2, 3, 0));
            size_t brAt = emit(isa::encode(isa::BR_EQ, 0, 0, 0, 0));
            size_t jmpExit = emit(isa::encode(isa::JMP, 0, 0, 0, 0));
            size_t bodyPc = pc();
            compileBlock(fr->body);
            compileStmt(fr->update);
            emit(isa::encode(isa::JMP, 0, 0, 0, relOff(pc(), head)));
            size_t exitPc = pc();
            compileBranch(b->op, brAt, bodyPc, exitPc);
            patch(jmpExit, isa::encode(isa::JMP, 0, 0, 0, relOff(jmpExit, exitPc)));
        }
        return;
    }
}

void CodeGen::compileBlock(const std::shared_ptr<BlockNode>& block) {
    for (auto& st : block->statements)
        compileStmt(st);
}

void CodeGen::compileFunction(const std::shared_ptr<FunctionNode>& fn) {
    currentFunc_ = fn->name;
    syms_.beginFunction(fn->name);
    funcPc_[fn->name] = pc();

    int locals = syms_.localBytesFor(fn->name);
    if (locals > 0)
        emit(isa::encode(isa::ALLOC_STACK, 0, 0, 0, static_cast<int16_t>(locals & 0xFFF)));

    compileBlock(fn->body);
    emit(isa::encode(isa::MOV_IMM, 1, 0, 0, 0));
    emit(isa::encode(isa::RET, 0, 1, 0, 0));
}

CompiledProgram CodeGen::compile(const std::shared_ptr<ProgramNode>& prog) {
    analyze(prog);

    // skizb: kanchum e main, heto EXIT
    emit(isa::encode(isa::PUSH_BP, 0, 0, 0, 0));
    emit(isa::encode(isa::PUSH_NEXT_PC, 0, 0, 0, 0));
    size_t callSite = emit(isa::encode(isa::CALL, 0, 0, 0, 0));
    emit(isa::encode(isa::EXIT, 0, 0, 0, 0));

    for (auto& item : prog->items) {
        if (auto f = std::dynamic_pointer_cast<FunctionNode>(item))
            compileFunction(f);
    }

    if (!funcPc_.count("main"))
        throw std::runtime_error("program must define func main()");

    patch(callSite, isa::encode(isa::CALL, 0, 0, 0, relOff(callSite, funcPc_["main"])));
    for (auto& p : callPatches_) {
        if (!funcPc_.count(p.second))
            throw std::runtime_error("undefined function: " + p.second);
        patch(p.first, isa::encode(isa::CALL, 0, 0, 0, relOff(p.first, funcPc_[p.second])));
    }

    CompiledProgram out;
    out.code = code_;
    out.data = syms_.dataImage();
    for (auto& kv : funcPc_) out.functions[kv.first] = static_cast<uint32_t>(kv.second);
    out.entryPc = 0;
    return out;
}