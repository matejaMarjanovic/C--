#include "expression.hpp"

LLVMContext TheContext;
map<string, pair<Value*, Types>> namedValues;
IRBuilder<> Builder(TheContext);
Module* TheModule;
legacy::FunctionPassManager* TheFPM;

Value* BinOpExprAST::codegen() const {
    Value* lhs = m_lhs->codegen();
    Value* rhs = m_rhs->codegen();
    
    if(m_type == Int && m_op == Slash) {
        Value* Dividend = lhs;
        Value* Divisor = rhs;
        ConstantInt *Shift = Builder.getInt32(31);
        
        Value *DividendSign = Builder.CreateAShr(Dividend, Shift);
        Value *DivisorSign  = Builder.CreateAShr(Divisor, Shift);
        Value *DvdXor       = Builder.CreateXor(Dividend, DividendSign);
        Value *DvsXor       = Builder.CreateXor(Divisor, DivisorSign);
        Value *UDividend    = Builder.CreateSub(DvdXor, DividendSign);
        Value *UDivisor     = Builder.CreateSub(DvsXor, DivisorSign);
        Value *URem         = Builder.CreateURem(UDividend, UDivisor);
        Value *Xored        = Builder.CreateXor(URem, DividendSign);
        Value *SRem         = Builder.CreateSub(Xored, DividendSign);
        
        if (Instruction *URemInst = dyn_cast<Instruction>(URem))
            Builder.SetInsertPoint(URemInst);                    
        
        return SRem;
    } 
    
    switch(m_type) {
        case Int:
            switch(m_op) {
                case Plus:
                    return Builder.CreateAdd(lhs, rhs, "addtmp");
                case Minus:
                    return Builder.CreateSub(lhs, rhs, "subtmp");
                case Asterisk:
                    return Builder.CreateMul(lhs, rhs, "multmp");
                case Greater:
                    return Builder.CreateICmpSGT(lhs, rhs, "gttmp");
                case GreaterEq:
                    return Builder.CreateICmpSGE(lhs, rhs, "geqtmp");
                case Less:
                    return Builder.CreateICmpSLT(lhs, rhs, "lttmp");
                case LessEq:
                    return Builder.CreateICmpSLE(lhs, rhs, "leqtmp");
                case Equal:
                    return Builder.CreateICmpEQ(lhs, rhs, "eqtmp");
                case NotEqual:
                    return Builder.CreateICmpNE(lhs, rhs, "netmp");
            }   
            break;
        case Double:
            switch(m_op) {
                case Plus:
                    return Builder.CreateFAdd(lhs, rhs, "addtmp");
                case Minus:
                    return Builder.CreateFSub(lhs, rhs, "subtmp");
                case Asterisk:
                    return Builder.CreateFMul(lhs, rhs, "multmp");
                case Slash:
                    return Builder.CreateFDiv(lhs, rhs, "divtmp");
                case Greater:
                    return Builder.CreateFCmpUGT(lhs, rhs, "gttmp");
                case GreaterEq:
                    return Builder.CreateFCmpUGE(lhs, rhs, "lttmp");
                case Less:
                    return Builder.CreateFCmpULT(lhs, rhs, "lttmp");
                case LessEq:
                    return Builder.CreateFCmpULE(lhs, rhs, "lttmp");
                case Equal:
                    return Builder.CreateFCmpUEQ(lhs, rhs, "eqtmp");
                case NotEqual:
                    return Builder.CreateFCmpUNE(lhs, rhs, "netmp");
            }   
            break;
    }
}

Types BinOpExprAST::type() {
    Types lhsType = m_lhs->type();
    Types rhsType = m_rhs->type();
    if(lhsType == rhsType) {
        m_type = lhsType;
    } else if (lhsType == Double || rhsType == Double) {
        m_type = Double;
    }
    return m_type;
}

Value* VariableExprAST::codegen() const {
    pair<Value*, Types>* a = new pair<Value*, Types>(namedValues[m_name]);
    if(a == nullptr) {
        cerr << "Promenljiva " << m_name << " nije definisana" << endl;
    }
    return a->first;
}

Types VariableExprAST::type() {
    pair<Value*, Types>* a = new pair<Value*, Types>(namedValues[m_name]);
    if(a == nullptr) {
        cerr << "Promenljiva " << m_name << " nije definisana" << endl;
    }
    return a->second;
}

Value* NumericExprAST::codegen() const {
    switch (m_type) {
        case Int:
            return ConstantInt::get(Type::getInt32Ty(TheContext), m_intVal, true);
        case Double:
            return ConstantFP::get(TheContext, APFloat(m_doubleVal));
    }
}

Types NumericExprAST::type() {
    return m_type;
}

// Value* FuncCallExprAST::codegen() const {
    
// }


void initializeModuleAndPassManager() {
    TheModule = new Module("my_module", TheContext);
    
    TheFPM = new legacy::FunctionPassManager(TheModule);
    
//     TheFPM->add(createInstructionCombiningPass());
//     TheFPM->add(createReassociatePass());
//     TheFPM->add(createGVNPass());
//     TheFPM->add(createCFGSimplificationPass());
    
    TheFPM->doInitialization();
}
