#pragma once

#include "statements.hpp"
#include "expression.hpp"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/FileSystem.h"
using namespace llvm;
using namespace std;

class PrototypeAST;
class FunctionAST;

class PrototypeAST {
public:
    PrototypeAST(string a, vector<pair<Types, string>> b, Types c)
        :m_name(a), 
         m_params(b),
         m_type{c} {
             
    }
    Function* codegen() const;
    string name() const {
        return m_name;
    }
    vector<pair<Types, string>> params() const {
        return m_params;
    }
private:
    string m_name;
    vector<pair<Types, string>> m_params;
    Types m_type;
};

class FunctionAST {
public:
    FunctionAST(PrototypeAST a, BlockStatAST* b)
        :m_proto(a), 
         m_body(b) {
         
    }
    Function* codegen() const;
    ~FunctionAST() {
        delete m_body;
    }
    FunctionAST(const FunctionAST &f) = delete;
    FunctionAST& operator=(const FunctionAST &f) = delete;
private:
    PrototypeAST m_proto;
    BlockStatAST* m_body;
};
