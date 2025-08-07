
#include "llvm/Support/Compiler.h" // LLVM_EXTERNAL_VISIBILITY
#include "llvm/MC/TargetRegistry.h"
#include "llvm/TextAPI/Target.h"   // Target class

using namespace llvm;

Target &llvm::getTheH2BLBTarget() {
  static Target TheH2BLBTarget;
  return TheH2BLBTarget;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeH2BLBTargetInfo() {
	RegisterTarget<Triple::h2blb, /* has JIT */ false> X (
		getTheH2BLBTarget(), /* Name= */ "h2blb",
		/* Desc= */ "How To build an llvm backend by example",
	);
}
