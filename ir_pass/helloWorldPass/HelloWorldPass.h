#pragma once

#include "llvm/IR/PassManager.h"
#include <llvm/IR/Analysis.h>

using namespace llvm;

class HelloWorldPass : public llvm::PassInfoMixin<HelloWorldPass> {
public:
	PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);

};
