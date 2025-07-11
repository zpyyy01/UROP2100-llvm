#ifndef LLVM_TRANSFORMS_FUNCTIONCOUNT_FUNCTIONCOUNT_H
#define LLVM_TRANSFORMS_FUNCTIONCOUNT_FUNCTIONCOUNT_H

#include "llvm/IR/PassManager.h"
#include <map>
#include <string>
#include <set>
#include <stack>

namespace llvm {

class FunctionCountPass : public PassInfoMixin<FunctionCountPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_FUNCTIONCOUNT_FUNCTIONCOUNT_H