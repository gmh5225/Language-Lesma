#pragma once

#include "AST/ExprVisitor.h"
#include "AST/StmtVisitor.h"
#include "Frontend/Parser.h"
#include "Symbol/SymbolTable.h"


namespace lesma {
    class AnalyzerError : public LesmaErrorWithExitCode<EX_DATAERR> {
    public:
        using LesmaErrorWithExitCode<EX_DATAERR>::LesmaErrorWithExitCode;
    };

    class Analyzer : public ExprVisitor<void, void>, public StmtVisitor<void> {
        std::unique_ptr<Parser> Parser_;
        SymbolTable *rootScope;
        SymbolTable *currentScope;

    public:
        explicit Analyzer(std::unique_ptr<Parser> parser);
        void Run();

        std::unique_ptr<Parser> getParser() { return std::move(Parser_); }

    protected:
        void visit(Statement *node) override;
        void visit(Compound *node) override;
        void visit(VarDecl *node) override;
        void visit(If *node) override;
        void visit(While *node) override;
        void visit(Import *node) override;
        void visit(FuncDecl *node) override;
        void visit(ExternFuncDecl *node) override;
        void visit(Assignment *node) override;
        void visit(Break *node) override;
        void visit(Continue *node) override;
        void visit(Return *node) override;
        void visit(Defer *node) override;
        void visit(ExpressionStatement *node) override;
        void visit(lesma::Type *node) override;
        void visit(Expression *node) override;
        void visit(FuncCall *node) override;
        void visit(BinaryOp *node) override;
        void visit(CastOp *node) override;
        void visit(UnaryOp *node) override;
        void visit(Literal *node) override;
        void visit(Else *node) override;
    };
}// namespace lesma