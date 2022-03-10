#pragma once

#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <map>
#include <utility>

#include "Common/Utils.h"
#include "SymbolTableEntry.h"

namespace lesma {
    class SymbolTable {
    public:
        explicit SymbolTable(SymbolTable *parent) : parent(parent){};

        SymbolTableEntry *lookup(const std::string &symbolName);

        void insertSymbol(const std::string &name, llvm::Value *value, llvm::Type *type);
        void insertSymbol(const std::string &name, llvm::Value *value, llvm::Type *type, bool mutable_);

        SymbolTable *createChildBlock(const std::string &blockName);
        SymbolTable *getParent();
        SymbolTable *getChild(const std::string &tableName);

        std::string toString(int ind) {
            std::string res;
            for (auto symbol: symbols) {
                res += std::string(ind, ' ') + "Symbols: \n";
                res += std::string(ind + 2, ' ') + symbol.second->toString() + '\n';
            }
            for (auto child: children) {
                res += std::string(ind, ' ') + "Tables: \n";
                res += std::string(ind + 2, ' ') + child.first + ": \n";
                res += child.second->toString(ind + 2) + '\n';
            }

            return res;
        }

    private:
        SymbolTable *parent;
        std::map<std::string, SymbolTable *> children;
        std::map<std::string, SymbolTableEntry *> symbols;
    };
}// namespace lesma