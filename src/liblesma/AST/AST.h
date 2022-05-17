#pragma once

#include <iostream>
#include <map>
#include <utility>
#include <vector>

#include "liblesma/Common/Utils.h"
#include "liblesma/Token/Token.h"
#include "liblesma/Token/TokenType.h"
#include "nameof.hpp"

namespace lesma {
    class AST {
        llvm::SMRange Loc;

    public:
        explicit AST(llvm::SMRange Loc) : Loc(Loc) {}
        virtual ~AST() = default;

        [[nodiscard]] [[maybe_unused]] llvm::SMRange getSpan() const { return Loc; }
        [[nodiscard]] [[maybe_unused]] llvm::SMLoc getStart() const { return Loc.Start; }
        [[nodiscard]] [[maybe_unused]] llvm::SMLoc getEnd() const { return Loc.End; }

        virtual std::string toString(llvm::SourceMgr *srcMgr, int ind) {
            return fmt::format("{}AST[Line({}-{}):Col({}-{})]:\n",
                               std::string(ind, ' '),
                               srcMgr->getLineAndColumn(Loc.Start).first,
                               srcMgr->getLineAndColumn(Loc.End).first,
                               srcMgr->getLineAndColumn(Loc.Start).second,
                               srcMgr->getLineAndColumn(Loc.End).second);
        }
    };

    class Expression : public AST {
    public:
        explicit Expression(llvm::SMRange Loc) : AST(Loc) {}
        ~Expression() override = default;
    };

    class Statement : public AST {
    public:
        explicit Statement(llvm::SMRange Loc) : AST(Loc) {}
        ~Statement() override = default;
    };


    class Literal : public Expression {
        std::string value;
        TokenType type;

    public:
        Literal(llvm::SMRange Loc, std::string value, TokenType type) : Expression(Loc), value(std::move(value)),
                                                                        type(type) {}

        ~Literal() override = default;

        [[nodiscard]] [[maybe_unused]] std::string getValue() const { return value; }
        [[nodiscard]] [[maybe_unused]] TokenType getType() const { return type; }

        std::string toString(llvm::SourceMgr * /*srcMgr*/, int /*ind*/) override {
            if (type == TokenType::STRING)
                return '"' + value + '"';
            else if (type == TokenType::NIL || type == TokenType::INTEGER || type == TokenType::DOUBLE ||
                     type == TokenType::IDENTIFIER || type == TokenType::BOOL)
                return value;
            else
                return "Unknown literal";
        }
    };

    class Compound : public Statement {
        std::vector<Statement *> children;

    public:
        explicit Compound(llvm::SMRange Loc) : Statement(Loc) {}
        explicit Compound(llvm::SMRange Loc, std::vector<Statement *> children) : Statement(Loc), children(std::move(children)) {}
        ~Compound() override = default;

        [[nodiscard]] [[maybe_unused]] std::vector<Statement *> getChildren() const { return children; }

        void addChildren(Statement *ast) {
            this->children.push_back(ast);
        }

        std::string toString(llvm::SourceMgr *srcMgr, int ind) override {
            auto ret = fmt::format("{}Compound Statement[Line({}-{}):Col({}-{})]:\n",
                                   std::string(ind, ' '),
                                   srcMgr->getLineAndColumn(getStart()).first,
                                   srcMgr->getLineAndColumn(getEnd()).first,
                                   srcMgr->getLineAndColumn(getStart()).second,
                                   srcMgr->getLineAndColumn(getEnd()).second);
            for (auto child: children)
                ret += child->toString(srcMgr, ind + 2);
            return ret;
        }
    };

    class Type : public Expression {
        std::string name;
        TokenType type;
        std::vector<Type *> params;
        Type *ret;

    public:
        Type(llvm::SMRange Loc, std::string name, TokenType type) : Expression(Loc), name(std::move(name)), type(type) {}
        Type(llvm::SMRange Loc, std::string name, TokenType type, std::vector<Type *> params, Type *ret) : Expression(Loc), name(std::move(name)), type(type), params(std::move(params)), ret(ret) {}
        ~Type() override = default;

        [[nodiscard]] [[maybe_unused]] std::string getName() const { return name; }
        [[nodiscard]] [[maybe_unused]] TokenType getType() const { return type; }
        [[nodiscard]] [[maybe_unused]] std::vector<Type *> getParams() const { return params; }
        [[nodiscard]] [[maybe_unused]] Type *getReturnType() const { return ret; }

        std::string toString(llvm::SourceMgr * /*srcMgr*/, int /*ind*/) override {
            return name;
        }
    };

    class Enum : public Statement {
        std::string identifier;
        std::vector<std::string> values;

    public:
        Enum(llvm::SMRange Loc, std::string identifier, std::vector<std::string> values) : Statement(Loc), identifier(std::move(identifier)), values(std::move(values)){};
        ~Enum() override = default;

        [[nodiscard]] [[maybe_unused]] std::string getIdentifier() const { return identifier; }
        [[nodiscard]] [[maybe_unused]] std::vector<std::string> getValues() const { return values; }

        std::string toString(llvm::SourceMgr *srcMgr, int ind) override {
            std::ostringstream imploded;
            std::copy(values.begin(), values.end(),
                      std::ostream_iterator<std::string>(imploded, ", "));
            return fmt::format("{}Enum[Line({}-{}):Col({}-{})]: {} with: {}\n",
                               std::string(ind, ' '),
                               srcMgr->getLineAndColumn(getStart()).first,
                               srcMgr->getLineAndColumn(getEnd()).first,
                               srcMgr->getLineAndColumn(getStart()).second,
                               srcMgr->getLineAndColumn(getEnd()).second,
                               identifier,
                               imploded.str());
        }
    };

    class Import : public Statement {
        std::string file_path;
        std::string alias;
        bool std;

    public:
        Import(llvm::SMRange Loc, std::string file_path, std::string alias, bool std) : Statement(Loc), file_path(std::move(file_path)), alias(std::move(alias)), std(std){};
        ~Import() override = default;

        [[nodiscard]] [[maybe_unused]] std::string getFilePath() const { return file_path; }
        [[nodiscard]] [[maybe_unused]] std::string getAlias() const { return alias; }
        [[nodiscard]] [[maybe_unused]] bool isStd() const { return std; }

        std::string toString(llvm::SourceMgr *srcMgr, int ind) override {
            return fmt::format("{}Import[Line({}-{}):Col({}-{})]: {} as {} from {}\n",
                               std::string(ind, ' '),
                               srcMgr->getLineAndColumn(getStart()).first,
                               srcMgr->getLineAndColumn(getEnd()).first,
                               srcMgr->getLineAndColumn(getStart()).second,
                               srcMgr->getLineAndColumn(getEnd()).second,
                               file_path,
                               alias,
                               std ? "std" : "file");
        }
    };

    class VarDecl : public Statement {
        Literal *var;
        std::optional<Type *> type;
        std::optional<Expression *> expr;
        bool mutable_;

    public:
        VarDecl(llvm::SMRange Loc, Literal *var, std::optional<Type *> type, std::optional<Expression *> expr, bool readonly) : Statement(Loc), var(var), type(type), expr(expr), mutable_(readonly) {}
        ~VarDecl() override = default;

        [[nodiscard]] [[maybe_unused]] Literal *getIdentifier() const { return var; }
        [[nodiscard]] [[maybe_unused]] std::optional<Type *> getType() const { return type; }
        [[nodiscard]] [[maybe_unused]] std::optional<Expression *> getValue() const { return expr; }
        [[nodiscard]] [[maybe_unused]] bool getMutability() const { return mutable_; }

        std::string toString(llvm::SourceMgr *srcMgr, int ind) override {
            return fmt::format("{}VarDecl[Line({}-{}):Col({}-{})]: {}{}{}\n",
                               std::string(ind, ' '),
                               srcMgr->getLineAndColumn(getStart()).first,
                               srcMgr->getLineAndColumn(getEnd()).first,
                               srcMgr->getLineAndColumn(getStart()).second,
                               srcMgr->getLineAndColumn(getEnd()).second,
                               var->toString(srcMgr, ind),
                               (type.has_value() ? ": " + type.value()->toString(srcMgr, ind) : ""),
                               (expr.has_value() ? " = " + expr.value()->toString(srcMgr, ind) : ""));
        }
    };

    class If : public Statement {
        std::vector<Expression *> conds;
        std::vector<Compound *> blocks;

    public:
        If(llvm::SMRange Loc, std::vector<Expression *> conds, std::vector<Compound *> blocks) : Statement(Loc),
                                                                                                 conds(std::move(
                                                                                                         conds)),
                                                                                                 blocks(std::move(
                                                                                                         blocks)) {}

        ~If() override = default;

        [[nodiscard]] [[maybe_unused]] std::vector<Expression *> getConds() const { return conds; }
        [[nodiscard]] [[maybe_unused]] std::vector<Compound *> getBlocks() const { return blocks; }

        std::string toString(llvm::SourceMgr *srcMgr, int ind) override {
            auto ret = fmt::format("{}If[Line({}-{}):Col({}-{})]:\n",
                                   std::string(ind, ' '),
                                   srcMgr->getLineAndColumn(getStart()).first,
                                   srcMgr->getLineAndColumn(getEnd()).first,
                                   srcMgr->getLineAndColumn(getStart()).second,
                                   srcMgr->getLineAndColumn(getEnd()).second);
            for (unsigned long i = 0; i < conds.size(); i++)
                ret += fmt::format("{}Cond: {}\n{}",
                                   std::string(ind + 2, ' '),
                                   conds[i]->toString(srcMgr, ind + 2),
                                   blocks[i]->toString(srcMgr, ind + 2));

            return ret;
        }
    };

    class While : public Statement {
        Expression *cond;
        Compound *block;

    public:
        While(llvm::SMRange Loc, Expression *cond, Compound *block) : Statement(Loc), cond(cond), block(block) {}
        ~While() override = default;

        [[nodiscard]] [[maybe_unused]] Expression *getCond() const { return cond; }
        [[nodiscard]] [[maybe_unused]] Compound *getBlock() const { return block; }

        std::string toString(llvm::SourceMgr *srcMgr, int ind) override {
            return fmt::format("{}While[Line({}-{}):Col({}-{})]:\n{}Cond: {}\n{}",
                               std::string(ind, ' '),
                               srcMgr->getLineAndColumn(getStart()).first,
                               srcMgr->getLineAndColumn(getEnd()).first,
                               srcMgr->getLineAndColumn(getStart()).second,
                               srcMgr->getLineAndColumn(getEnd()).second,
                               std::string(ind + 2, ' '),
                               cond->toString(srcMgr, ind + 2),
                               block->toString(srcMgr, ind + 2));
        }
    };

    class FuncDecl : public Statement {
        std::string name;
        Type *return_type;
        std::vector<std::pair<std::string, Type *>> parameters;
        Compound *body;

    public:
        FuncDecl(llvm::SMRange Loc, std::string name, Type *return_type,
                 std::vector<std::pair<std::string, Type *>> parameters, Compound *body) : Statement(Loc), name(std::move(name)), return_type(return_type), parameters(std::move(parameters)),
                                                                                           body(body) {}

        ~FuncDecl() override = default;

        [[nodiscard]] [[maybe_unused]] std::string getName() const { return name; }
        [[nodiscard]] [[maybe_unused]] Type *getReturnType() const { return return_type; }
        [[nodiscard]] [[maybe_unused]] std::vector<std::pair<std::string, Type *>> getParameters() const { return parameters; }
        [[nodiscard]] [[maybe_unused]] Compound *getBody() const { return body; }

        std::string toString(llvm::SourceMgr *srcMgr, int ind) override {
            auto ret = fmt::format("{}FuncDecl[Line({}-{}):Col({}-{})]: {}(",
                                   std::string(ind, ' '),
                                   srcMgr->getLineAndColumn(getStart()).first,
                                   srcMgr->getLineAndColumn(getEnd()).first,
                                   srcMgr->getLineAndColumn(getStart()).second,
                                   srcMgr->getLineAndColumn(getEnd()).second,
                                   name);
            for (auto &param: parameters) {
                ret += param.first + ": " + param.second->toString(srcMgr, ind);
                if (parameters.back() != param) ret += ", ";
            }
            ret += fmt::format(") -> {}\n{}", return_type->toString(srcMgr, ind), body->toString(srcMgr, ind + 2));
            return ret;
        }
    };

    class ExternFuncDecl : public Statement {
        std::string name;
        Type *return_type;
        std::vector<std::pair<std::string, Type *>> parameters;

    public:
        ExternFuncDecl(llvm::SMRange Loc, std::string name, Type *return_type,
                       std::vector<std::pair<std::string, Type *>> parameters) : Statement(Loc), name(std::move(name)), return_type(return_type), parameters(std::move(parameters)) {}

        ~ExternFuncDecl() override = default;

        [[nodiscard]] [[maybe_unused]] std::string getName() const { return name; }
        [[nodiscard]] [[maybe_unused]] Type *getReturnType() const { return return_type; }
        [[nodiscard]] [[maybe_unused]] std::vector<std::pair<std::string, Type *>> getParameters() const { return parameters; }

        std::string toString(llvm::SourceMgr *srcMgr, int ind) override {
            auto ret = fmt::format("{}FuncDecl[Line({}-{}):Col({}-{})]: {}(",
                                   std::string(ind, ' '),
                                   srcMgr->getLineAndColumn(getStart()).first,
                                   srcMgr->getLineAndColumn(getEnd()).first,
                                   srcMgr->getLineAndColumn(getStart()).second,
                                   srcMgr->getLineAndColumn(getEnd()).second,
                                   name);
            for (auto &param: parameters) {
                ret += param.first + ": " + param.second->toString(srcMgr, ind);
                if (parameters.back() != param) ret += ", ";
            }
            ret += fmt::format(") -> {}\n", return_type->toString(srcMgr, ind));
            return ret;
        }
    };

    class FuncCall : public Expression {
        std::string name;
        std::vector<Expression *> arguments;

    public:
        FuncCall(llvm::SMRange Loc, std::string name, std::vector<Expression *> arguments) : Expression(Loc), name(std::move(name)), arguments(std::move(arguments)) {}

        ~FuncCall() override = default;

        [[nodiscard]] [[maybe_unused]] std::string getName() const { return name; }
        [[nodiscard]] [[maybe_unused]] std::vector<Expression *> getArguments() const { return arguments; }

        std::string toString(llvm::SourceMgr *srcMgr, int ind) override {
            auto ret = name + "(";
            for (auto param: arguments) {
                ret += param->toString(srcMgr, ind);
                if (arguments.back() != param) ret += ", ";
            }
            ret += ")";
            return ret;
        }
    };

    class Assignment : public Statement {
        Literal *var;
        TokenType op;
        Expression *expr;

    public:
        Assignment(llvm::SMRange Loc, Literal *var, TokenType op, Expression *expr) : Statement(Loc), var(var), op(op), expr(expr) {}

        ~Assignment() override = default;

        [[nodiscard]] [[maybe_unused]] Literal *getIdentifier() const { return var; }
        [[nodiscard]] [[maybe_unused]] TokenType getOperator() const { return op; }
        [[nodiscard]] [[maybe_unused]] Expression *getExpression() const { return expr; }

        std::string toString(llvm::SourceMgr *srcMgr, int ind) override {
            return fmt::format("{}Assignment[Line({}-{}):Col({}-{})]: {} {} {}\n",
                               std::string(ind, ' '),
                               srcMgr->getLineAndColumn(getStart()).first,
                               srcMgr->getLineAndColumn(getEnd()).first,
                               srcMgr->getLineAndColumn(getStart()).second,
                               srcMgr->getLineAndColumn(getEnd()).second,
                               var->toString(srcMgr, ind),
                               std::string{NAMEOF_ENUM(op)},
                               expr->toString(srcMgr, ind));
        }
    };

    class ExpressionStatement : public Statement {
        Expression *expr;

    public:
        ExpressionStatement(llvm::SMRange Loc, Expression *expr) : Statement(Loc), expr(expr) {}
        ~ExpressionStatement() override = default;

        [[nodiscard]] [[maybe_unused]] Expression *getExpression() const { return expr; }

        std::string toString(llvm::SourceMgr *srcMgr, int ind) override {
            return fmt::format("{}Expression[Line({}-{}):Col({}-{})]: {}\n",
                               std::string(ind, ' '),
                               srcMgr->getLineAndColumn(getStart()).first,
                               srcMgr->getLineAndColumn(getEnd()).first,
                               srcMgr->getLineAndColumn(getStart()).second,
                               srcMgr->getLineAndColumn(getEnd()).second,
                               expr->toString(srcMgr, ind));
        }
    };

    class BinaryOp : public Expression {
        Expression *left;
        TokenType op;
        Expression *right;

    public:
        BinaryOp(llvm::SMRange Loc, Expression *left, TokenType op, Expression *right) : Expression(Loc), left(left),
                                                                                         op(op), right(right) {}

        ~BinaryOp() override = default;

        [[nodiscard]] [[maybe_unused]] Expression *getLeft() const { return left; }
        [[nodiscard]] [[maybe_unused]] TokenType getOperator() const { return op; }
        [[nodiscard]] [[maybe_unused]] Expression *getRight() const { return right; }

        std::string toString(llvm::SourceMgr *srcMgr, int ind) override {
            return left->toString(srcMgr, ind) + " " + std::string{NAMEOF_ENUM(op)} + " " + right->toString(srcMgr, ind);
        }
    };

    class CastOp : public Expression {
        Expression *expr;
        Type *type;

    public:
        CastOp(llvm::SMRange Loc, Expression *expr, Type *type) : Expression(Loc), expr(expr), type(type) {}

        ~CastOp() override = default;

        [[nodiscard]] [[maybe_unused]] Expression *getExpression() const { return expr; }
        [[nodiscard]] [[maybe_unused]] Type *getType() const { return type; }

        std::string toString(llvm::SourceMgr *srcMgr, int ind) override {
            return expr->toString(srcMgr, ind) + " as " + type->toString(srcMgr, ind);
        }
    };

    class UnaryOp : public Expression {
        TokenType op;
        Expression *expr;

    public:
        UnaryOp(llvm::SMRange Loc, TokenType op, Expression *expr) : Expression(Loc), op(op), expr(expr) {}
        ~UnaryOp() override = default;

        [[nodiscard]] [[maybe_unused]] TokenType getOperator() const { return op; }
        [[nodiscard]] [[maybe_unused]] Expression *getExpression() const { return expr; }

        std::string toString(llvm::SourceMgr *srcMgr, int ind) override {
            return std::string{NAMEOF_ENUM(op)} + expr->toString(srcMgr, ind);
        }
    };

    class DotOp : public Expression {
        Expression *left;
        TokenType op;
        Expression *right;

    public:
        DotOp(llvm::SMRange Loc, Expression *left, TokenType op, Expression *right) : Expression(Loc), left(left),
                                                                                      op(op), right(right) {}

        ~DotOp() override = default;

        [[nodiscard]] [[maybe_unused]] Expression *getLeft() const { return left; }
        [[nodiscard]] [[maybe_unused]] TokenType getOperator() const { return op; }
        [[nodiscard]] [[maybe_unused]] Expression *getRight() const { return right; }

        std::string toString(llvm::SourceMgr *srcMgr, int ind) override {
            return left->toString(srcMgr, ind) + "." + right->toString(srcMgr, ind);
        }
    };

    class Else : public Expression {
    public:
        explicit Else(llvm::SMRange Loc) : Expression(Loc) {}
        ~Else() override = default;

        std::string toString(llvm::SourceMgr * /*srcMgr*/, int /*ind*/) override {
            return "Else";
        }
    };

    class Break : public Statement {
    public:
        explicit Break(llvm::SMRange Loc) : Statement(Loc) {}
        ~Break() override = default;

        std::string toString(llvm::SourceMgr * /*srcMgr*/, int ind) override {
            return std::string(ind, ' ') + "Break\n";
        }
    };

    class Continue : public Statement {
    public:
        explicit Continue(llvm::SMRange Loc) : Statement(Loc) {}
        ~Continue() override = default;

        std::string toString(llvm::SourceMgr * /*srcMgr*/, int ind) override {
            return std::string(ind, ' ') + "Continue\n";
        }
    };

    class Return : public Statement {
        Expression *value;

    public:
        Return(llvm::SMRange Loc, Expression *value) : Statement(Loc), value(value) {}
        ~Return() override = default;

        [[nodiscard]] [[maybe_unused]] Expression *getValue() const { return value; }

        std::string toString(llvm::SourceMgr *srcMgr, int ind) override {
            return std::string(ind, ' ') + "Return " + value->toString(srcMgr, ind) + '\n';
        }
    };

    class Defer : public Statement {
        Statement *stmt;

    public:
        Defer(llvm::SMRange Loc, Statement *stmt) : Statement(Loc), stmt(stmt) {}
        ~Defer() override = default;

        [[nodiscard]] [[maybe_unused]] Statement *getStatement() const { return stmt; }

        std::string toString(llvm::SourceMgr *srcMgr, int ind) override {
            return std::string(ind, ' ') + "Defer " + stmt->toString(srcMgr, 0);
        }
    };
}// namespace lesma