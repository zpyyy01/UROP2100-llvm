#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Demangle/Demangle.h"
#include <llvm/ADT/StringRef.h>
#include "llvm/IR/Module.h"
#include <iostream>
#include <iomanip>

using Result = llvm::MapVector<const llvm::Function *, unsigned>;

using namespace llvm;

struct Fun_Call_Count : public PassInfoMixin<Fun_Call_Count>{

    PreservedAnalyses run(Module &M,ModuleAnalysisManager &MAM);

    Result run_Module(Module& M);
};