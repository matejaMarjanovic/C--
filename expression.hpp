#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <utility>
#include <map>
#include <stack>
using namespace std;

#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
using namespace llvm;

enum Operators {
    Plus,
    Minus,
    Asterisk,
    Slash,
    Greater,
    Less,
    GreaterEq,
    LessEq,
    Equal,
    NotEqual
};

enum Types {
    Int,
    Double
};

class ExprAST {
public:
    virtual Value* codegen() const = 0;
    virtual ~ExprAST() {
        
    }
    virtual Types type() = 0;
protected:
    Types m_type;
};

class BinOpExprAST : public ExprAST {
public:
    BinOpExprAST(ExprAST* lhs, ExprAST* rhs, Operators op)
        :m_lhs{lhs},
         m_rhs{rhs},
         m_op{op} {
        m_type = type();
    }
    Types type();
    Value* codegen() const;
    ~BinOpExprAST() {
        delete m_lhs;
        delete m_rhs;
    }
private:
    ExprAST* m_lhs;
    ExprAST* m_rhs;
    Operators m_op;
};

class VariableExprAST : public ExprAST {
public:
    VariableExprAST(const string &name) 
        :m_name{name} {
            
    }
    Types type();
    Value* codegen() const;
private:
    string m_name;
};

class NumericExprAST : public ExprAST {
public:
    NumericExprAST(int intVal, Types type)
        :m_intVal{intVal} {
        m_type = type;     
    }
    NumericExprAST(double doubleVal, Types type)
        :m_doubleVal{doubleVal} {
        m_type = type;
    }
    Types type();
    Value* codegen() const;
private:
    int m_intVal;
    double m_doubleVal;
};

// class FuncCallExprAST : public ExprAST {
// public:
//     FuncCallExprAST(const string &name, const vector<ExprAST*> &args)
//         :m_name{name},
//          m_args{args} {
//              
//     }
//     void type() const;
//     Value* codegen() const;
//     ~FuncCallExprAST() {
//         for(auto &e : m_args) {
//             delete e;
//         }
//     }
// private:
//     string m_name;
//     vector<ExprAST*> m_args;
// };

void initializeModuleAndPassManager();
