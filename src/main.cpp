#include <vector>

#include "plf_nanotimer.h"

#include "Backend/Codegen.h"
#include "Common/Utils.h"
#include "Frontend/Lexer.h"
#include "Frontend/Parser.h"
#include "Backend/Analyzer.h"

using namespace lesma;

#define TIMEIT(debug_operation, statements)   \
    timer.start();                            \
    statements                                \
            results = timer.get_elapsed_ms(); \
    total += results;                         \
    if (options->timer)                       \
        print(DEBUG, "{} -> {:.2f} ms\n", debug_operation, results);

int main(int argc, char **argv) {
    // Configure Timer
    plf::nanotimer timer;
    double results, total = 0;

    // CLI Parsing
    TIMEIT("CLI", auto options = parseCLI(argc, argv);)

    try {
        // Read Source
        TIMEIT("File read", auto source = readFile(options->file);)

        // Lexer
        TIMEIT("Lexer scan",
               auto lexer = std::make_unique<Lexer>(source, options->file.substr(options->file.find_last_of("/\\") + 1));
               lexer->ScanAll();)

        if (options->debug) {
            print(DEBUG, "TOKENS: \n");
            for (const auto &tok: lexer->getTokens())
                print("Token: {}\n", tok->Dump());
        }

        // Parser
        TIMEIT("Parsing",
               auto parser = std::make_unique<Parser>(lexer->getTokens());
               parser->Parse();)

        if (options->debug)
            print(DEBUG, "AST:\n{}", parser->getAST()->toString(0));

        // Analyzer
        TIMEIT("Analyzing",
               auto analyzer = std::make_unique<Analyzer>(std::move(parser));
               analyzer->Run();)

        // Codegen
        TIMEIT("Compiling",
               auto codegen = std::make_unique<Codegen>(analyzer->getParser(), options->file, options->jit, true);
               codegen->Run();)

        // Optimization
        TIMEIT("Optimizing", codegen->Optimize(llvm::PassBuilder::OptimizationLevel::O3);)

        if (options->debug) {
            print(DEBUG, "LLVM IR: \n");
            codegen->Dump();
        }

        int exit_code = 0;
        if (!options->jit) {
            // Compile to Object File
            TIMEIT("Writing Object File", codegen->WriteToObjectFile(options->output);)

            // Link Object File
            TIMEIT("Linking Object File", codegen->LinkObjectFile(fmt::format("{}.o", options->output));)
        } else {
            // Executing
            TIMEIT("Execution", exit_code = codegen->JIT();)
        }

        if (options->timer)
            print(DEBUG, "Total -> {:.2f} ms\n", total);

        return exit_code;
    } catch (const LesmaError &err) {
        if (err.getSpan() == Span{})
            print(ERROR, err.what());
        else
            showInline(err.getSpan(), err.what(), options->file, true);
        return err.exit_code;
    }
}