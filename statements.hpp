#pragma once

#include "expression.hpp"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
using namespace llvm;
using namespace std;

class StatAST;
class AssignementStatAST;
class ListVarsStatAST;
class DeclarationStatAST;
class BlockStatAST;
class IfElseStatAST;
class ReturnStatAST;
class ExpressionStatAST;

class StatAST {
public:
    virtual Value* codegen() const = 0; 
    virtual ~StatAST() {
        
    }
};

class AssignementStatAST : public StatAST {
public:
    AssignementStatAST(string a, ExprAST* b)
        :m_lhs{a},
         m_rhs{b} {
             
    }
    Value* codegen() const;
    string lhs() const {
        return m_lhs;
    }
    ExprAST* rhs() const {
        return m_rhs;
    }
    void setType(Types a) {
        m_type = a;
    }
    ~AssignementStatAST() {
        delete m_rhs;
    }
private:  
    string m_lhs;
    ExprAST* m_rhs;
    Types m_type;
};

class ListVarsStatAST : public StatAST {
public:
    ListVarsStatAST(const vector<AssignementStatAST*> &b)
        :m_asgs{b} {
             
    }
    Value* codegen() const;
    ~ListVarsStatAST() {
        for(auto &e : m_asgs) {
            delete e;
        }
    }
private:
    vector<AssignementStatAST*> m_asgs;
};

class DeclarationStatAST : public StatAST {
public:
    DeclarationStatAST(Types a, const vector<AssignementStatAST*> &b)
        :m_type{a},
         m_asgs{b} {
             
    }
    Value* codegen() const;
    ~DeclarationStatAST() {
        for(auto &e : m_asgs) {
            delete e;
        }
    }
private:    
    Types m_type;
    vector<AssignementStatAST*> m_asgs;
};

class ExpressionStatAST : public StatAST {
public:
    ExpressionStatAST(ExprAST* expr)
        :m_expr{expr} {
            
    }
    Value* codegen() const;
    ~ExpressionStatAST() {
        delete m_expr;
    }
private:
    ExprAST* m_expr;
};

class ReturnStatAST : public StatAST {
public:
    ReturnStatAST(ExprAST* a)
        :m_retExpr{a} {
            
    }
    Value* codegen() const;
    ~ReturnStatAST() {
        delete m_retExpr;
    }
private:
    ExprAST* m_retExpr;
};

class IfElseStatAST : public StatAST {
public:
    IfElseStatAST(ExprAST* a, BlockStatAST* b, BlockStatAST* c)
        :m_if{a},
         m_then{b},
         m_else{c} {
             
    }
    Value* codegen() const;
    ~IfElseStatAST() {
        delete m_if;
        delete m_then;
        delete m_else;
    }
private:
    ExprAST* m_if;
    BlockStatAST* m_then;
    BlockStatAST* m_else;
};

class BlockStatAST : public StatAST {
public:
    BlockStatAST(vector<StatAST*> a)
        :m_stats{a} {
            
    }
    Value* codegen() const;
    ~BlockStatAST() {
        for(auto &e : m_stats) {
            delete e;
        }
    }
private:
    vector<StatAST*> m_stats;
};

AllocaInst *CreateEntryBlockAlloca(Function *TheFunction, const string &VarName);
