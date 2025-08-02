#include "ConstantPropagationPass.h"

#include <llvm/ADT/APInt.h>
#include <llvm/ADT/PostOrderIterator.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/Analysis.h>
#include <llvm/IR/CFG.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>

#define CASE_OPCODE(opcode, func_core)                                         \
  case opcode:                                                                 \
    NewConst = visitBinary(                                                    \
        I, Ctxt,                                                               \
        [](const APInt &a, const APInt &b) -> std::optional<APInt> func_core); \
    break;

static Value *visitBinary(Instruction &Instr, LLVMContext &Ctxt,
                          std::optional<APInt> (*Computation)(const APInt &,
                                                              const APInt &)) {
  assert(isa<BinaryOperator>(Instr) && "This is meant for binary instruction");

  auto LHS = dyn_cast<ConstantInt>(Instr.getOperand(0));
  auto RHS = dyn_cast<ConstantInt>(Instr.getOperand(1));
  if (!LHS || !RHS)
    return nullptr;

  auto Res = Computation(LHS->getValue(), RHS->getValue());
  return ConstantInt::get(Ctxt, *Res);
}

PreservedAnalyses ConstantPropagationPass::run(Function &F,
                                               FunctionAnalysisManager &) {
  if (F.empty())
    return PreservedAnalyses::all();

  LLVMContext &Ctxt = F.getContext();
  bool DidChange = false;
  ReversePostOrderTraversal<Function *> RPOT(&F);

  for (BasicBlock *BB : RPOT) {
    // safe iteration while modifying the list (deleting the curent node
    // at the end)
    for (Instruction &I : make_early_inc_range(*BB)) {
      Value *NewConst = nullptr;
      switch (I.getOpcode()) {
        CASE_OPCODE(Instruction::Add, { return a + b; })
        CASE_OPCODE(Instruction::Sub, { return a - b; })
        CASE_OPCODE(Instruction::Mul, { return a * b; })
        CASE_OPCODE(Instruction::Shl, { return a.shl(b); })
        CASE_OPCODE(Instruction::LShr, { return a.lshr(b); })
        CASE_OPCODE(Instruction::AShr, { return a.ashr(b); })
        CASE_OPCODE(Instruction::Xor, { return a ^ b; })
        CASE_OPCODE(Instruction::And, { return a & b; })
        CASE_OPCODE(Instruction::Or, { return a | b; })
        CASE_OPCODE(Instruction::UDiv, {
          if (b.isZero())
            return std::nullopt;
          return a.udiv(b);
        })
        CASE_OPCODE(Instruction::SDiv, {
          if (b.isZero())
            return std::nullopt;
          return a.sdiv(b);
        })
      }
      if (NewConst) {
        I.replaceAllUsesWith(NewConst);
        I.eraseFromParent();
        DidChange = true;
      }
    }
  }

  return DidChange ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
