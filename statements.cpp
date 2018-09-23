#include "statements.hpp"

extern LLVMContext TheContext;
extern map<string, pair<Value*, Types>> namedValues;
extern IRBuilder<> Builder;
extern Module* TheModule;
extern legacy::FunctionPassManager* TheFPM;

Value* AssignementStatAST::codegen() const {
    auto iter = namedValues.find(m_lhs);
    if(iter != namedValues.end()) {
        if(m_rhs->type() != namedValues[m_lhs].second) {
            cerr << "Invalid type" << endl;
            return nullptr;
        }
        namedValues[m_lhs].first = m_rhs->codegen();
    }
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
//     iterating through the vector of assignements
    for(auto e : m_asgs) {
//         check does that variable name already exist
        if(namedValues.find(e->lhs()) != namedValues.end()) {
            cerr << "Variable " << e->lhs() << " already exists.";
            return nullptr;
        }
        pair<Value*, Types> tmp(e->rhs()->codegen(), m_type);
        namedValues[e->lhs()] = tmp;
    }
    return ConstantInt::get(Type::getInt32Ty(TheContext), 0, true);
}

Value* ExpressionStatAST::codegen() const {
    return m_expr->codegen();
}

Value* ReturnStatAST::codegen() const {
    return m_retExpr->codegen();
}

Value* IfElseStatAST::codegen() const {
    Value* CondV = m_if->codegen();
    if (CondV == nullptr)
        return nullptr;
    if(m_if->type() == Double) {
        CondV = Builder.CreateFCmpONE(CondV, ConstantFP::get(TheContext, APFloat(0.0)), "ifcond");
    } else if(m_if->type() == Int) {
        CondV = Builder.CreateFCmpONE(CondV, ConstantInt::get(Type::getInt32Ty(TheContext), 0, true), "ifcond");
    }
    Function *TheFunction = Builder.GetInsertBlock()->getParent();

    BasicBlock *ThenBB =
    BasicBlock::Create(TheContext, "then", TheFunction);
    BasicBlock *ElseBB = BasicBlock::Create(TheContext, "else");
    BasicBlock *MergeBB = BasicBlock::Create(TheContext, "ifcont");

    Builder.CreateCondBr(CondV, ThenBB, ElseBB);

    Builder.SetInsertPoint(ThenBB);
    Value* ThenV = m_then->codegen();
    if (ThenV == nullptr)
        return nullptr;
    Builder.CreateBr(MergeBB);
    ThenBB = Builder.GetInsertBlock();

    TheFunction->getBasicBlockList().push_back(ElseBB);
    Builder.SetInsertPoint(ElseBB);
    Value* ElseV = m_else->codegen();
    if (ElseV == nullptr)
        return nullptr;
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

