#include "function.hpp"

extern LLVMContext TheContext;
extern map<string, pair<AllocaInst*, Types>> namedValues;
extern IRBuilder<> Builder;
extern Module* TheModule;
extern legacy::FunctionPassManager* TheFPM;

Function* PrototypeAST::codegen() const {
    vector<Type*> Parameters;
    for(auto e : m_params) {
        if(e.first == Double) {
            Parameters.push_back(Type::getDoubleTy(TheContext));
        } else if(e.first == Int) {
            Parameters.push_back(Type::getInt32Ty(TheContext));
        }
    }
    FunctionType *FT;
    if(m_type == Double) {
        FT = FunctionType::get(Type::getDoubleTy(TheContext), Parameters, false);
    } else if(m_type == Int) {
        FT = FunctionType::get(Type::getInt32Ty(TheContext), Parameters, false);
    }

    Function *F = Function::Create(FT, Function::ExternalLinkage, m_name, TheModule);

    unsigned Idx = 0;
    for (auto &Arg : F->args())
        Arg.setName(m_params[Idx++].second);

    return F;
}

Function* FunctionAST::codegen() const {
    Function *TheFunction = TheModule->getFunction(m_proto.name());

    if (!TheFunction)
        TheFunction = m_proto.codegen();

    if (!TheFunction)
        return nullptr;

    if (!TheFunction->empty()) {
        cerr << "Nije dozvoljeno predefinisanje fje " << m_proto.name() << endl;
        return nullptr;
    }
    
    // Create a new basic block to start insertion into.
    BasicBlock *BB = BasicBlock::Create(TheContext, "entry", TheFunction);
    Builder.SetInsertPoint(BB);

    // Record the function arguments in the namedValues map.
    map<string, pair<AllocaInst*, Types>> tmpNamedValues(namedValues);
    namedValues.clear();
    
    vector<Types> Parameters;
    for(auto e : m_proto.params()) {
        Parameters.push_back(e.first);
    }
    
    int i = 0;
    for (auto &Arg : TheFunction->args()) {
        AllocaInst* Alloca = CreateEntryBlockAlloca(TheFunction, Arg.getName(), Parameters[i]);
        namedValues[Arg.getName()] = pair<AllocaInst*, Types>(Alloca, Parameters[i++]);
        Builder.CreateStore(&Arg, Alloca);
    }
    
    Value *RetVal = m_body->codegen();
    if(RetVal != nullptr) {
        verifyFunction(*TheFunction);
        cout << "pusi kurac" << endl;
        TheFPM->run(*TheFunction);
        namedValues.clear();
        namedValues = tmpNamedValues;
        return TheFunction;
    }
    TheFunction->eraseFromParent();
    return nullptr;
}
