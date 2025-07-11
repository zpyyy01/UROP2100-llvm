#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
  struct Test01 : public FunctionPass {
    static char ID; // Pass 标识符
    Test01() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override {
      errs() << "Test01: Hello from function " << F.getName() << "\n";
      return false; // 不修改 IR，返回 false
    }
  };
}

char Test01::ID = 0;
static RegisterPass<Test01> X("test01", "Test Pass 01", false, false);