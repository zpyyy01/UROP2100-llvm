//===-- HelloWorld.cpp - Example Transformations --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/FunctionCount.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Analysis/DominanceFrontier.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/SSAUpdater.h"



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
      for (const Instruction &I : BB) {
        for (const Use &U : I.operands()) {
          if (auto *Callee = dyn_cast<Function>(U.get())) {
            // If the operand is a function, we could count calls here
            // State.CallCount++;
            errs() << "Function " << F.getName() << " calls " << Callee->getName() << "\n";
          }
        }
      }
    }
  }
  for (const auto &Entry : FunctionCounts) {
    errs() << "Function: " << Entry.first <<  "\n";
    Entry.second.print();
  }

  //Construct CFG for each function
  // This class should be used like this:
// {
//   ReversePostOrderTraversal<Function*> RPOT(FuncPtr); // Expensive to create
//   for (rpo_iterator I = RPOT.begin(); I != RPOT.end(); ++I) {
//      ...
//   }
//   for (rpo_iterator I = RPOT.begin(); I != RPOT.end(); ++I) {
//      ...
//   }
// }
  // construct CFG for each function
  for (Function &F : M) {
        if (F.isDeclaration()) continue;
        errs() << "\n=== CFG Traversal for " << F.getName() << " ===\n";
        ReversePostOrderTraversal<const Function *> RPOT(&F);
        for (auto I = RPOT.begin(), E = RPOT.end(); I != E; ++I) {
        const BasicBlock *BB = *I;
        errs() << "Basic Block: " << BB->getName() << "\n";
        
        // Show predecessors and successors
        errs() << "  Predecessors: ";
        for (const BasicBlock *Pred : predecessors(BB)) {
            errs() << Pred->getName() << " ";
        }
        errs() << "\n  Successors: ";
        for (const BasicBlock *Succ : successors(BB)) {
            errs() << Succ->getName() << " ";
        }
        errs() << "\n";
        
        for (const Instruction &Inst : *BB) {
            errs() << "  Instruction: " << Inst << "\n";
        }
        errs() << "=== End of CFG Traversal for " << F.getName() << " ===\n";
      }

  }
  // calculate dominance frontier for each block
  for (Function &F : M) {
    if (F.isDeclaration()) continue;
    
    errs() << "\n=== Dominance Frontier for " << F.getName() << " ===\n";
    
    // Build dominator tree
    DominatorTree DT(F);
    
    // Build dominance frontier
    DominanceFrontier DF;
    DF.analyze(DT);
    
    // Print dominance frontier for each block
    for (BasicBlock &BB : F) {
      errs() << "Block " << BB.getName() << " dominance frontier: ";
      
      auto DFSet = DF.find(&BB);
      if (DFSet != DF.end()) {
        for (BasicBlock *DFBlock : DFSet->second) {
          errs() << DFBlock->getName() << " ";
        }
      }
      errs() << "\n";
      
      // Also show immediate dominator
      BasicBlock *IDom = DT.getNode(&BB)->getIDom() ? 
                         DT.getNode(&BB)->getIDom()->getBlock() : nullptr;
      errs() << "  Immediate dominator: " << (IDom ? IDom->getName() : "none") << "\n";
    }
    
    errs() << "=== End Dominance Frontier for " << F.getName() << " ===\n";
  }

  return PreservedAnalyses::all();
}
 