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
  
  
  // Rename Variables - Second phase of SSA construction
  for (Function &F : M) {
    if (F.isDeclaration()) continue;
    
    errs() << "\n=== Variable Renaming for " << F.getName() << " ===\n";
    
    // Rebuild dominator tree for renaming phase
    DominatorTree DT(F);
    
    // Data structures for renaming
    std::map<AllocaInst*, std::stack<Value*>> var_stack;
    std::map<AllocaInst*, int> var_counter;
    
    // Collect all variables again
    std::set<AllocaInst*> variables;
    for (BasicBlock &BB : F) {
      for (Instruction &I : BB) {
        if (auto *AI = dyn_cast<AllocaInst>(&I)) {
          variables.insert(AI);
        }
      }
    }
    
    // Initialize stacks and counters
    for (AllocaInst *var : variables) {
      var_counter[var] = 0;
      var_stack[var] = std::stack<Value*>();
    }
    
    // Helper function to rename a basic block and its dominated children
    std::function<void(BasicBlock*)> renameBlock = [&](BasicBlock *BB) {
      errs() << "Renaming block: " << BB->getName() << "\n";
      
      // Track how many variables we push in this block (for cleanup)
      std::map<AllocaInst*, int> pushed_count;
      for (AllocaInst *var : variables) {
        pushed_count[var] = 0;
      }
      
      // Step 1: Handle PHI nodes first
      for (Instruction &I : *BB) {
        if (PHINode *phi = dyn_cast<PHINode>(&I)) {
          // Find which variable this PHI corresponds to
          for (AllocaInst *var : variables) {
            if (phi->getName().starts_with(var->getName())) {
              // Create new version for this PHI
              var_counter[var]++;
              var_stack[var].push(phi);
              pushed_count[var]++;
              
              // Rename the PHI node itself
              phi->setName(var->getName() + "." + std::to_string(var_counter[var]));
              
              errs() << "  PHI renamed to: " << phi->getName() << "\n";
              break;
            }
          }
        }
      }
      
      // Step 2: Process other instructions
      std::vector<Instruction*> toErase; // Collect instructions to erase later

      for (Instruction &I : *BB) {
        if (isa<PHINode>(&I)) continue; // Already handled
        
        // Handle loads (uses of variables)
        if (LoadInst *load = dyn_cast<LoadInst>(&I)) {
          if (AllocaInst *var = dyn_cast<AllocaInst>(load->getPointerOperand())) {
            if (variables.count(var) && !var_stack[var].empty()) {
              // Replace load with current version
              Value *current_val = var_stack[var].top();
              load->replaceAllUsesWith(current_val);
              toErase.push_back(load); // Mark for deletion
              errs() << "  Replaced load of " << var->getName() 
                    << " with " << current_val->getName() << "\n";
              continue;
            }
          }
        }
      
        // Handle stores (definitions of variables)
        if (StoreInst *store = dyn_cast<StoreInst>(&I)) {
          if (AllocaInst *var = dyn_cast<AllocaInst>(store->getPointerOperand())) {
            if (variables.count(var)) {
              // Create new version
              Value *stored_val = store->getValueOperand();
              var_counter[var]++;
              
              // Create a new name for the stored value if it doesn't have one
              if (stored_val->getName().empty()) {
                stored_val->setName(var->getName() + "." + std::to_string(var_counter[var]));
              }
              
              var_stack[var].push(stored_val);
              pushed_count[var]++;
              
              // Mark store for deletion
              toErase.push_back(store);
              
              errs() << "  Store to " << var->getName() 
                    << " creates version " << stored_val->getName() << "\n";
              continue;
            }
          }
        }
      }
      for (Instruction *inst : toErase) {
        inst->eraseFromParent();
      }

      
      // Step 3: Fill in PHI node operands in successor blocks
      for (BasicBlock *succ : successors(BB)) {
        for (Instruction &I : *succ) {
          if (PHINode *phi = dyn_cast<PHINode>(&I)) {
            // Find which variable this PHI corresponds to
            for (AllocaInst *var : variables) {
              if (phi->getName().starts_with(var->getName())) {
                // Find the incoming edge from BB and update it
                for (unsigned i = 0; i < phi->getNumIncomingValues(); ++i) {
                  if (phi->getIncomingBlock(i) == BB) {
                    if (!var_stack[var].empty()) {
                      Value *current_val = var_stack[var].top();
                      phi->setIncomingValue(i, current_val);
                      errs() << "  Updated PHI " << phi->getName() 
                             << " incoming from " << BB->getName() 
                             << " with " << current_val->getName() << "\n";
                    }
                  }
                }
                break;
              }
            }
          } else {
            break; // PHI nodes are always at the beginning
          }
        }
      }
      
      // Step 4: Recursively rename dominated children
      for (DomTreeNode *childNode : DT.getNode(BB)->children()) {
        renameBlock(childNode->getBlock());
      }
      
      // Step 5: Pop variables that were pushed in this block
      for (AllocaInst *var : variables) {
        for (int i = 0; i < pushed_count[var]; ++i) {
          if (!var_stack[var].empty()) {
            var_stack[var].pop();
          }
        }
      }
    };
    
    // Start renaming from the entry block
    BasicBlock *entry = &F.getEntryBlock();
    renameBlock(entry);
    
    //Clean up: remove original alloca instructions
    std::vector<AllocaInst*> allocasToErase;
    for (AllocaInst *var : variables) {
      if (var->use_empty()) {
        allocasToErase.push_back(var);
      }
    }

    // Now safely erase them
    for (AllocaInst *var : allocasToErase) {
      errs() << "Removed alloca for " << var->getName() << "\n";
      var->eraseFromParent();
    }
    
    errs() << "=== End Variable Renaming for " << F.getName() << " ===\n";
  }
  // Print modified module
  M.print(errs(), nullptr);
  return PreservedAnalyses::all();
}