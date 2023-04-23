#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <fmt/printf.h>

#include "liblesma/Backend/Codegen.h"
#include "liblesma/Common/Utils.h"
#include "liblesma/Driver/Driver.h"
#include "liblesma/Frontend/Lexer.h"
#include "liblesma/Frontend/Parser.h"

#include <filesystem>
#include <vector>

using namespace lesma;

std::string get_directory() {
    namespace fs = std::filesystem;
    fs::path current_file_path(__FILE__);
    fs::path repository_directory = current_file_path.parent_path();
    return repository_directory.string();
}

std::vector<std::string> collect_test_files(const std::string &test_folder) {
    std::vector<std::string> test_files;
    for (const auto &entry: std::filesystem::directory_iterator(test_folder)) {
        if (entry.is_regular_file()) {
            test_files.push_back(entry.path().string());
        }
    }
    return test_files;
}

std::shared_ptr<SourceMgr> initializeSrcMgr(const std::string &source) {
    // Configure Source Manager
    std::shared_ptr<SourceMgr> srcMgr = std::make_shared<SourceMgr>(SourceMgr());

    auto buffer = MemoryBuffer::getMemBuffer(source);
    srcMgr->AddNewSourceBuffer(std::move(buffer), llvm::SMLoc());

    return srcMgr;
}

Lexer *initializeLexer(const std::shared_ptr<SourceMgr> &srcMgr) {
    auto lexer = new Lexer(srcMgr);
    lexer->ScanAll();

    return lexer;
}

Parser *initializeParser(Lexer *lexer) {
    auto parser = new Parser(lexer->getTokens());
    parser->Parse();

    return parser;
}

Codegen *initializeCodegen(Parser *parser, std::shared_ptr<SourceMgr> srcMgr) {
    std::unique_ptr<Parser> parser_pointer;
    parser_pointer.reset(parser);
    auto codegen = new Codegen(std::move(parser_pointer), std::move(srcMgr), __FILE__, {}, true, true);
    codegen->Run();

    return codegen;
}

llvm::SMRange getRange(const std::string &source, int x, int y) {
    return {llvm::SMLoc::getFromPointer(source.c_str() + x), llvm::SMLoc::getFromPointer(source.c_str() + y)};
}

TEST_CASE("Lexer", "Tokens") {
    std::string source =
            "var y: int = 100\n"
            "y = 101\n"
            "exit(y)\n";

    auto srcMgr = initializeSrcMgr(source);
    auto lexer = initializeLexer(srcMgr);

    REQUIRE(lexer->getTokens().size() > 1);

    std::vector<Token *> tokens = {
            new Token{TokenType::VAR, "var", getRange(source, 0, 3)},
            new Token{TokenType::IDENTIFIER, "y", getRange(source, 4, 5)},
            new Token{TokenType::COLON, ":", getRange(source, 5, 6)},
            new Token{TokenType::INT_TYPE, "int", getRange(source, 7, 10)},
            new Token{TokenType::EQUAL, "=", getRange(source, 11, 12)},
            new Token{TokenType::INTEGER, "100", getRange(source, 13, 16)},
            new Token{TokenType::NEWLINE, "NEWLINE", getRange(source, 16, 17)},
            new Token{TokenType::IDENTIFIER, "y", getRange(source, 17, 18)},
            new Token{TokenType::EQUAL, "=", getRange(source, 19, 20)},
            new Token{TokenType::INTEGER, "101", getRange(source, 21, 24)},
            new Token{TokenType::NEWLINE, "NEWLINE", getRange(source, 24, 25)},
            new Token{TokenType::IDENTIFIER, "exit", getRange(source, 25, 29)},
            new Token{TokenType::LEFT_PAREN, "(", getRange(source, 29, 30)},
            new Token{TokenType::IDENTIFIER, "y", getRange(source, 30, 31)},
            new Token{TokenType::RIGHT_PAREN, ")", getRange(source, 31, 32)},
            new Token{TokenType::NEWLINE, "NEWLINE", getRange(source, 32, 33)},
            new Token{TokenType::EOF_TOKEN, "EOF", getRange(source, 33, 33)},
    };

    BENCHMARK("Lexer") {
        initializeLexer(srcMgr);
    };

    for (auto [a, b]: zip(tokens, lexer->getTokens()))
        REQUIRE(*a == *b);
}

// Currently, all statement and expressions use the abstract base class Statement or Expression,
// which makes it difficult to test
TEST_CASE("Parser", "AST") {
    std::string source =
            "var y: int = 100\n"
            "y = 101\n"
            "exit(y)\n";

    auto srcMgr = initializeSrcMgr(source);
    auto lexer = initializeLexer(srcMgr);
    auto parser = initializeParser(lexer);

    REQUIRE(parser->getAST()->getChildren().size() == 3);
    REQUIRE(parser->getAST()->getChildren().at(0)->toString(srcMgr.get(), "", true) == "└──VarDecl[Line(1-1):Col(1-17)]: y: int = 100\n");
    REQUIRE(parser->getAST()->getChildren().at(1)->toString(srcMgr.get(), "", true) == "└──Assignment[Line(2-2):Col(1-8)]: y EQUAL 101\n");
    REQUIRE(parser->getAST()->getChildren().at(2)->toString(srcMgr.get(), "", true) == "└──Expression[Line(3-3):Col(1-8)]: exit(y)\n");

    BENCHMARK("Parser") {
        initializeParser(lexer);
    };
}

// We cannot return from top-level, and the exit function just exits the whole process including the test
TEST_CASE("Codegen", "Run & Optimize") {
    std::string source =
            "var y: int = 100\n"
            "y = 101\n";

    auto srcMgr = initializeSrcMgr(source);
    auto lexer = initializeLexer(srcMgr);
    auto parser = initializeParser(lexer);
    auto codegen = initializeCodegen(parser, srcMgr);
    codegen->Optimize(OptimizationLevel::O3);
    int exit_code = codegen->JIT();

    REQUIRE(exit_code == 0);

    BENCHMARK("Codegen") {
        initializeCodegen(parser, srcMgr);
    };

    BENCHMARK("Codegen & Optimize") {
        auto cg = initializeCodegen(parser, srcMgr);
        cg->Optimize(OptimizationLevel::O3);
    };

    BENCHMARK("Codegen & JIT") {
        auto cg = initializeCodegen(parser, srcMgr);
        cg->JIT();
    };

    BENCHMARK("Codegen, Optimize & JIT") {
        auto cg = initializeCodegen(parser, srcMgr);
        cg->Optimize(OptimizationLevel::O3);
        cg->JIT();
    };
}

// TODO: Compilation doesn't seem to work if done too closely due to linker
TEST_CASE("Lesma Run test files", "[lesma]") {
    std::string directory = get_directory();
    auto success_test_files = collect_test_files(directory + "/lesma/success");
    auto failure_test_files = collect_test_files(directory + "/lesma/failure");

    for (const auto &test_file: success_test_files) {
        SECTION("Testing successful file: " + test_file) {
            auto options = std::make_unique<lesma::Options>(Options{lesma::FILE, test_file});

            int run_ret_value = lesma::Driver::Run(std::move(options));
            REQUIRE(run_ret_value == 0);
        }
    }

    for (const auto &test_file: failure_test_files) {
        SECTION("Testing failing file: " + test_file) {
            auto options = std::make_unique<lesma::Options>(Options{lesma::FILE, test_file});

            int run_ret_value = lesma::Driver::Run(std::move(options));
            REQUIRE(run_ret_value != 0);
        }
    }
}