#include "statements.hpp"

extern LLVMContext TheContext;
extern map<string, pair<AllocaInst*, Types>> namedValues;
extern IRBuilder<> Builder;
extern Module* TheModule;
extern legacy::FunctionPassManager* TheFPM;
extern bool error;

Value* AssignementStatAST::codegen() const {
    Value* tmp = nullptr;
    if(namedValues.find(m_lhs) != namedValues.end()) { 
        if(namedValues[m_lhs].second != m_rhs->type()) {
            cerr << "Pogresan tip za promenljivu: " << m_lhs << endl;
            error = true;
            return nullptr;
        }
        tmp = m_rhs->codegen();
        if(tmp == nullptr) {
            cout << "ne moze da se izgenerise desna strana naredbe dodele" << endl;
            error = true;
            return nullptr;
        }
        AllocaInst* Alloca = namedValues[m_lhs].first;
        Builder.CreateStore(tmp, Alloca);
        return tmp;
    }
    else {
        Function* TheFunction = Builder.GetInsertBlock()->getParent();
        AllocaInst* Alloca = CreateEntryBlockAlloca(TheFunction, m_lhs, m_type);
        if (m_rhs != nullptr) {
            tmp = m_rhs->codegen();
        } else {
            if(m_type == Double) {
                tmp = ConstantFP::get(TheContext, APFloat(0.0));
            } else if(m_type == Int) {
                tmp = ConstantInt::get(Type::getInt32Ty(TheContext), 0, true);
            }
        }
        if (tmp == nullptr) {
            cout << "greska u deklaraciji sa desne strane" << endl;
            error = true;
            return nullptr;
        }
        
        namedValues[m_lhs] = pair<AllocaInst*, Types>(Alloca, m_type);
        if(namedValues[m_lhs].second == Double) {
            Builder.CreateAlignedStore(tmp, Alloca, 8);
        } 
        else if(namedValues[m_lhs].second == Int) {
            Builder.CreateAlignedStore(tmp, Alloca, 4);
        }
        
        return tmp;
    }
}

Value* ListVarsStatAST::codegen() const {
    Value* tmp;
    for(auto e : m_asgs) {
        tmp = e->codegen();
    }
    return tmp;
}

Value* DeclarationStatAST::codegen() const {
    Value* tmp;
    for(auto &e : m_asgs) {
        e->setType(m_type);
        tmp = e->codegen();
    }
    return tmp;
}

Value* ExpressionStatAST::codegen() const {
    return m_expr->codegen();
}

Value* ReturnStatAST::codegen() const {
    return Builder.CreateRet(m_retExpr->codegen());
}

Value* IfElseStatAST::codegen() const {
    Value* CondV = m_if->codegen();
    if (CondV == nullptr) {
        cout << "uslovni izraz ne moze da se izracuna" << endl;
        error = true;
        return nullptr;
    }
    if(m_if->type() == Double) {
        CondV = Builder.CreateFCmpONE(CondV, ConstantFP::get(TheContext, APFloat(0.0)), "ifcond");
    } else if(m_if->type() == Int) {
        CondV = Builder.CreateICmpNE(CondV, ConstantInt::get(Type::getInt32Ty(TheContext), 0, true), "ifcond");
    }
    Function *TheFunction = Builder.GetInsertBlock()->getParent();

    BasicBlock *ThenBB =
    BasicBlock::Create(TheContext, "then", TheFunction);
    BasicBlock *ElseBB;
    ElseBB = BasicBlock::Create(TheContext, "else");
    BasicBlock *MergeBB = BasicBlock::Create(TheContext, "ifcont");

    Builder.CreateCondBr(CondV, ThenBB, ElseBB);

    Builder.SetInsertPoint(ThenBB);
    Value* ThenV = m_then->codegen();
    if (ThenV == nullptr) {
        cout << "if blok ne moze da se izgenerise" << endl;
        error = true;
        return nullptr;
    }
    Builder.CreateBr(MergeBB);
    ThenBB = Builder.GetInsertBlock();

    Value* ElseV = nullptr;
    TheFunction->getBasicBlockList().push_back(ElseBB);
    Builder.SetInsertPoint(ElseBB);
    if(m_else) {
        ElseV = m_else->codegen();
        if (ElseV == nullptr) {
            cout << "else blok ne moze da se izgenerise" << endl;
            error = true;
            return nullptr;
        }
    } else {
        if(m_if->type() == Double) {
            ElseV = ConstantFP::get(TheContext, APFloat(0.0));
        } else if(m_if->type() == Int) {
            ElseV = ConstantInt::get(Type::getInt32Ty(TheContext), 0, true);
        }
    }
    Builder.CreateBr(MergeBB);
    ElseBB = Builder.GetInsertBlock();

    TheFunction->getBasicBlockList().push_back(MergeBB);
    Builder.SetInsertPoint(MergeBB);
    PHINode* PN = nullptr;
    if(m_if->type() == Double) {
        PN = Builder.CreatePHI(Type::getDoubleTy(TheContext), 2, "iftmp");
    } else if(m_if->type() == Int) {
        PN = Builder.CreatePHI(Type::getInt32Ty(TheContext), 2, "iftmp");
    }
    PN->addIncoming(ThenV, ThenBB);
    PN->addIncoming(ElseV, ElseBB);
    
    return PN;
}

Value* WhileStatAST::codegen() const {
    Function* theFunction = Builder.GetInsertBlock()->getParent();
    BasicBlock* loopBB = BasicBlock::Create(TheContext, "loop", theFunction);
    BasicBlock* afterLoopBB = BasicBlock::Create(TheContext, "after");
    
    Value* condV = m_cond->codegen();
    if(!condV) {
        return nullptr;
    }
    if(m_cond->type() == Double) {
        condV = Builder.CreateFCmpONE(condV, ConstantFP::get(TheContext, APFloat(0.0)), "loopcond");
    } else if(m_cond->type() == Int) {
        condV = Builder.CreateICmpNE(condV, ConstantInt::get(Type::getInt32Ty(TheContext), 0, true), "loopcond");
    }
    if(!condV) {
        return nullptr;
    }
    Builder.CreateCondBr(condV, loopBB, afterLoopBB);
    
    Builder.SetInsertPoint(loopBB);
    
    Value* bodyV = m_body->codegen();
    if(!bodyV) {
        return nullptr;
    }
    condV = m_cond->codegen();
    if(!condV) {
        return nullptr;
    }
    if(m_cond->type() == Double) {
        condV = Builder.CreateFCmpONE(condV, ConstantFP::get(TheContext, APFloat(0.0)), "loopcond");
    } else if(m_cond->type() == Int) {
        condV = Builder.CreateICmpNE(condV, ConstantInt::get(Type::getInt32Ty(TheContext), 0, true), "loopcond");
    }
    if(!condV) {
        return nullptr;
    }
    Builder.CreateCondBr(condV, loopBB, afterLoopBB);
    
    theFunction->getBasicBlockList().push_back(afterLoopBB);
    Builder.SetInsertPoint(afterLoopBB);
    return ConstantFP::get(TheContext, APFloat(0.0));
}

Value* BlockStatAST::codegen() const {
    Value* tmp;
    for(auto e : m_stats) {
        tmp = e->codegen();
    }
    return tmp;
}

AllocaInst *CreateEntryBlockAlloca(Function *TheFunction, const string &VarName, Types type) {
    IRBuilder<> tmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
    if(type == Double) {
        return tmpB.CreateAlloca(Type::getDoubleTy(TheContext), 0, VarName.c_str());
    } else if(type == Int) {
        return tmpB.CreateAlloca(Type::getInt32Ty(TheContext), 0, VarName.c_str());
    }
}
