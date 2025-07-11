#include "llvm/IR/Function.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

// 定义我们的 Function Pass
struct FunctionCountPass : public PassInfoMixin<FunctionCountPass> {
  int count = 0;

  // 每次处理一个函数时调用
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    count++;
    return PreservedAnalyses::all();
  }

  // 在整个模块处理结束时输出结果
  static void printStatistics(int Count) {
    errs() << "Number of functions: " << Count << "\n";
  }

  // 析构函数中打印统计信息（仅用于演示）
  ~FunctionCountPass() {
    printStatistics(count);
  }
};

// 注册到默认优化流水线开始位置
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "function-count", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineStartEPCallback(
                [&](ModulePassManager &MPM, auto) {
                  MPM.addPass(createModuleToFunctionPassAdaptor(FunctionCountPass()));
                });
          }};
}