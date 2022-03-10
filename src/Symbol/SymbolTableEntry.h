#pragma once

#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include "Common/Utils.h"

namespace lesma {
    class SymbolTableEntry {
    public:
        explicit SymbolTableEntry(std::string name, llvm::Value *value, llvm::Type *type) : name(std::move(name)),
                                                                                            value(value), type(type),
                                                                                            mutable_(false) {}
        explicit SymbolTableEntry(std::string name, llvm::Value *value, llvm::Type *type, bool mutable_) : name(std::move(name)),
                                                                                                           value(value), type(type),
                                                                                                           mutable_(mutable_) {}

        [[nodiscard]] std::string getName() { return name; }
        [[nodiscard]] llvm::Value *getValue() { return value; }
        [[nodiscard]] llvm::Type *getType() { return type; }
        [[nodiscard]] bool getUsed() { return used; }
        [[nodiscard]] bool getMutability() const { return mutable_; }
        void setUsed() { used = true; }

        std::string toString() {
            std::string type_str, value_str;
            llvm::raw_string_ostream rso(type_str), rso2(value_str);
            type->print(rso);
            value->print(rso2);
            return name + ": " + type_str + " = " + value_str;
        }

    private:
        std::string name;
        llvm::Value *value;
        llvm::Type *type;
        bool mutable_;
        bool used = false;
    };
}// namespace lesma