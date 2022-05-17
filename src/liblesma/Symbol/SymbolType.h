#pragma once

#include <algorithm>
#include <string>
#include <vector>

enum SymbolSuperType {
    TY_INVALID,
    TY_INT,
    TY_FLOAT,
    TY_STRING,
    TY_BOOL,
    TY_FUNCTION,
    TY_STRUCT,
};

class SymbolType {
    SymbolSuperType baseSuperType;

public:
    // Constructors
    explicit SymbolType(SymbolSuperType superType) : baseSuperType(superType) {}

    // Public methods
    [[nodiscard]] bool is(SymbolSuperType superType) const;
    [[nodiscard]] bool isPrimitive() const;
    [[nodiscard]] bool isOneOf(const std::vector<SymbolSuperType> &superTypes) const;
    [[nodiscard]] SymbolSuperType getSuperType() const;
    friend bool operator==(const SymbolType &lhs, const SymbolType &rhs);
    friend bool operator!=(const SymbolType &lhs, const SymbolType &rhs);
};