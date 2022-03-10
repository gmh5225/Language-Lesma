#include "Analyzer.h"

using namespace lesma;

Analyzer::Analyzer(std::unique_ptr<Parser> parser) {
    Parser_ = std::move(parser);
}

void Analyzer::Run() {
    rootScope = currentScope = new SymbolTable(nullptr);

    visit(Parser_->getAST());
}

void Analyzer::visit(Expression *node) {
    if (dynamic_cast<FuncCall *>(node))
        return visit(dynamic_cast<FuncCall *>(node));
    else if (dynamic_cast<BinaryOp *>(node))
        return visit(dynamic_cast<BinaryOp *>(node));
    else if (dynamic_cast<CastOp *>(node))
        return visit(dynamic_cast<CastOp *>(node));
    else if (dynamic_cast<UnaryOp *>(node))
        return visit(dynamic_cast<UnaryOp *>(node));
    else if (dynamic_cast<Literal *>(node))
        return visit(dynamic_cast<Literal *>(node));
    else if (dynamic_cast<Else *>(node))
        return visit(dynamic_cast<Else *>(node));

    throw AnalyzerError(node->getSpan(), "Unknown Expression: {}", node->toString(0));
}

void Analyzer::visit(Statement *node) {
    if (dynamic_cast<VarDecl *>(node))
        return visit(dynamic_cast<VarDecl *>(node));
    else if (dynamic_cast<If *>(node))
        return visit(dynamic_cast<If *>(node));
    else if (dynamic_cast<While *>(node))
        return visit(dynamic_cast<While *>(node));
    else if (dynamic_cast<FuncDecl *>(node))
        return visit(dynamic_cast<FuncDecl *>(node));
    else if (dynamic_cast<Import *>(node))
        return visit(dynamic_cast<Import *>(node));
    else if (dynamic_cast<ExternFuncDecl *>(node))
        return visit(dynamic_cast<ExternFuncDecl *>(node));
    else if (dynamic_cast<Assignment *>(node))
        return visit(dynamic_cast<Assignment *>(node));
    else if (dynamic_cast<Break *>(node))
        return visit(dynamic_cast<Break *>(node));
    else if (dynamic_cast<Continue *>(node))
        return visit(dynamic_cast<Continue *>(node));
    else if (dynamic_cast<Return *>(node))
        return visit(dynamic_cast<Return *>(node));
    else if (dynamic_cast<Defer *>(node))
        return visit(dynamic_cast<Defer *>(node));
    else if (dynamic_cast<ExpressionStatement *>(node))
        return visit(dynamic_cast<ExpressionStatement *>(node));
    else if (dynamic_cast<Compound *>(node))
        return visit(dynamic_cast<Compound *>(node));

    throw AnalyzerError(node->getSpan(), "Unknown Statement:\n{}", node->toString(0));
}

void Analyzer::visit(Compound *node) {
    for (auto elem: node->getChildren())
        visit(elem);
}

void Analyzer::visit(VarDecl *node) {

}

void Analyzer::visit(If *node) {

}

void Analyzer::visit(While *node) {

}

void Analyzer::visit(Import *node) {

}

void Analyzer::visit(FuncDecl *node) {

}

void Analyzer::visit(ExternFuncDecl *node) {

}

void Analyzer::visit(Assignment *node) {

}

void Analyzer::visit(Break *node) {

}

void Analyzer::visit(Continue *node) {

}

void Analyzer::visit(Return *node) {

}

void Analyzer::visit(Defer *node) {

}

void Analyzer::visit(ExpressionStatement *node) {

}

void Analyzer::visit(Type *node) {

}

void Analyzer::visit(FuncCall *node) {

}

void Analyzer::visit(BinaryOp *node) {

}

void Analyzer::visit(CastOp *node) {

}

void Analyzer::visit(UnaryOp *node) {

}

void Analyzer::visit(Literal *node) {

}

void Analyzer::visit(Else *node) {

}