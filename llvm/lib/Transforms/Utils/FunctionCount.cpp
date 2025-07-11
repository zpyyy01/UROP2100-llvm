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
  // calculate dominance frontier for each block and insert phi functions
  for (Function &F : M) {
    if (F.isDeclaration()) continue;
    
    errs() << "\n=== Dominance Frontier for " << F.getName() << " ===\n";
    
    // Build dominator tree
    DominatorTree DT(F);
    
    // Build dominance frontier
    DominanceFrontier DF;
    DF.analyze(DT);
    
    // // Print dominance frontier for each block
    // for (BasicBlock &BB : F) {
    //   errs() << "Block " << BB.getName() << " dominance frontier: ";
      
    //   auto DFSet = DF.find(&BB);
    //   if (DFSet != DF.end()) {
    //     for (BasicBlock *DFBlock : DFSet->second) {
    //       errs() << DFBlock->getName() << " ";
    //     }
    //   }
    //   errs() << "\n";
      
    //   // Also show immediate dominator
    //   BasicBlock *IDom = DT.getNode(&BB)->getIDom() ? 
    //                      DT.getNode(&BB)->getIDom()->getBlock() : nullptr;
    //   errs() << "  Immediate dominator: " << (IDom ? IDom->getName() : "none") << "\n";
    // }
    //errs() << "=== End Dominance Frontier for " << F.getName() << " ===\n";
    
    //insert phi functions

    std::set<AllocaInst*> variables;
    for (BasicBlock &BB : F) {
      for (Instruction &I : BB) {
        if (auto *AI = dyn_cast<AllocaInst>(&I)) {
          variables.insert(AI);
        }
      }
    }
    errs() << "Variables in function " << F.getName() << ":\n";

    std::map<AllocaInst*, std::set<BasicBlock*>> def_blocks;
    for (AllocaInst *var : variables) {
      for (User *user : var->users()) {
        if (auto *store = dyn_cast<StoreInst>(user)) {
          if (store->getPointerOperand() == var) {
            def_blocks[var].insert(store->getParent());
          }
        }
      }
    }

    std::map<AllocaInst*, std::set<BasicBlock*>> phi_blocks;
    for (AllocaInst *var : variables) {
      std::stack<BasicBlock*> worklist;
      std::set<BasicBlock*> visited;

      for (BasicBlock *def_bb : def_blocks[var]) {
        worklist.push(def_bb);
        visited.insert(def_bb);
      }

      while (!worklist.empty()) {
        BasicBlock *bb = worklist.top();
        worklist.pop();

        auto df_iter = DF.find(bb);
        if (df_iter != DF.end()) {
          for (BasicBlock *df_bb : df_iter->second) {
          if (phi_blocks[var].find(df_bb) == phi_blocks[var].end()) {
              //insert phi function
              IRBuilder<> builder(&df_bb->front());
              Type *var_type = var->getAllocatedType();
              PHINode *phi_node = builder.CreatePHI(var_type, 0, var->getName() + ".phi");

              for (auto it = pred_begin(df_bb); it != pred_end(df_bb); ++it) {
                BasicBlock *pred = *it;
                // Use UndefValue as placeholder - this will be fixed in the renaming phase
                phi_node->addIncoming(UndefValue::get(var_type), pred);
              }

              phi_blocks[var].insert(df_bb);

              errs() << "Inserted PHI node for variable " << var->getName()
                   << " in block " << df_bb->getName() << "\n";

              if (visited.find(df_bb) == visited.end()) {
                worklist.push(df_bb);
                visited.insert(df_bb);
              }
            }
          }
        }
      }
    }
  }
  
  //Rename Variables
  for (Function &F : M) {
    if (F.isDeclaration()) continue;

    errs() << "\n=== Renaming Variables in " << F.getName() << " ===\n";
    DominatorTree DT(F);

    std::map<AllocaInst*, std::stack<Value*>> var_stack;
    std::map<AllocaInst*, int> var_count;
    std::set<AllocaInst*> variables;

    for (BasicBlock &BB : F) {
      for (Instruction &I : BB) {
        if (auto *AI = dyn_cast<AllocaInst>(&I)) {
          variables.insert(AI);
          var_stack[AI] = std::stack<Value*>();
          var_count[AI] = 0;
        }
      }
    }

    
  }


  return PreservedAnalyses::all();
}