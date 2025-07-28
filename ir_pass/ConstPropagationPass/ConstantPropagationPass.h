#pragma once

#include <llvm/IR/PassManager.h>
#include <llvm/IR/Analysis.h>

using namespace llvm;

class ConstantPropagationPass : public PassInfoMixin<ConstantPropagationPass> {
public :
	PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};
