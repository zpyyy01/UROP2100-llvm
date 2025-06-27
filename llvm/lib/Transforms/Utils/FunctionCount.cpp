//===-- HelloWorld.cpp - Example Transformations --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/FunctionCount.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"


using namespace llvm;

class MyFuncState {
public:
  int InstructionCount;
  int BlockCount;
  //int CallCount;
  MyFuncState() : InstructionCount(0), BlockCount(0) {}
  //print
  void print() const {
    errs() << "Instruction Count: " << InstructionCount
           << ", Block Count: " << BlockCount << "\n";
  }

};

PreservedAnalyses FunctionCountPass::run(Module &M, ModuleAnalysisManager &MAM) {
  std::map<std::string, MyFuncState> FunctionCounts;
  //outputs function's basic information, like function name, instruction count, block count,
  for(const Function &F : M) {
    MyFuncState &State = FunctionCounts[F.getName().str()];
    State.BlockCount = F.size(); // Number of basic blocks
    for (const BasicBlock &BB : F) {
      State.InstructionCount += BB.size(); // Number of instructions in the block
    }
  }
  for (const auto &Entry : FunctionCounts) {
    errs() << "Function: " << Entry.first <<  "\n";
    Entry.second.print();
  }
  return PreservedAnalyses::all();
}
 