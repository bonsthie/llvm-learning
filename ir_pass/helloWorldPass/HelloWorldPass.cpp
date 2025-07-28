#include "HelloWorldPass.h"
#include <llvm/IR/Analysis.h>
#include <llvm/IR/Function.h>


using namespace llvm;

PreservedAnalyses HelloWorldPass::run(Function &F, FunctionAnalysisManager &AM) {
	errs() << F.getName() << "\n";
	return PreservedAnalyses::all();
}
