#include "statements.hpp"

extern LLVMContext TheContext;
extern map<string, pair<AllocaInst*, Types>> namedValues;
extern IRBuilder<> Builder;
extern Module* TheModule;
extern legacy::FunctionPassManager* TheFPM;

Value* AssignementStatAST::codegen() const {
    Function* TheFunction = Builder.GetInsertBlock()->getParent();
    AllocaInst* Alloca = CreateEntryBlockAlloca(TheFunction, m_lhs);
    Value* Tmp = nullptr;
    if(namedValues.find(m_lhs) == namedValues.end()) {
        namedValues[m_lhs] = pair<AllocaInst*, Types>(Alloca, m_type);
    }
    if (m_rhs != nullptr) {
        Tmp = m_rhs->codegen();
    } else {
        if(namedValues[m_lhs].second == Double) {
            Tmp = ConstantFP::get(TheContext, APFloat(0.0));
        } else if(namedValues[m_lhs].second == Int) {
            Tmp = ConstantInt::get(Type::getInt32Ty(TheContext), 0, true);
        }
    }
    if (Tmp == nullptr) {
        return nullptr;
    }
    namedValues[m_lhs] = pair<AllocaInst*, Types>(Alloca, m_rhs->type());
    Builder.CreateStore(Tmp, Alloca);
    
    return namedValues[m_lhs].first;
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
        return nullptr;
    }
    if(m_if->type() == Double) {
        CondV = Builder.CreateFCmpONE(CondV, ConstantFP::get(TheContext, APFloat(0.0)), "ifcond");
    } else if(m_if->type() == Int) {
        CondV = Builder.CreateFCmpONE(CondV, ConstantInt::get(Type::getInt32Ty(TheContext), 0, true), "ifcond");
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
        return nullptr;
    }
    Builder.CreateBr(MergeBB);
    ThenBB = Builder.GetInsertBlock();

    Value* ElseV;
    TheFunction->getBasicBlockList().push_back(ElseBB);
    Builder.SetInsertPoint(ElseBB);
    if(m_else) {
        ElseV = m_else->codegen();
        if (ElseV == nullptr) {
            return nullptr;
        }
    } else {
        ElseV = ConstantInt::get(Type::getInt32Ty(TheContext), 0, true);
    }
    Builder.CreateBr(MergeBB);
    ElseBB = Builder.GetInsertBlock();

    TheFunction->getBasicBlockList().push_back(MergeBB);
    Builder.SetInsertPoint(MergeBB);
    PHINode* PN;
    if(m_if->type() == Double) {
        PN = Builder.CreatePHI(Type::getDoubleTy(TheContext), 2, "iftmp");
    } else if(m_if->type() == Int) {
        PN = Builder.CreatePHI(Type::getInt32Ty(TheContext), 2, "iftmp");
    }
    PN->addIncoming(ThenV, ThenBB);
    PN->addIncoming(ElseV, ElseBB);
    return PN;
}

Value* BlockStatAST::codegen() const {
    Value* tmp;
    for(auto e : m_stats) {
        tmp = e->codegen();
    }
    return tmp;
}

AllocaInst *CreateEntryBlockAlloca(Function *TheFunction, const string &VarName) {
    IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
    return TmpB.CreateAlloca(Type::getDoubleTy(TheContext), 0, VarName.c_str());
}
