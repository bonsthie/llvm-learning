#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/SourceMgr.h>

using namespace llvm;

void build_func(LLVMContext &Ctxt, Module &Mod) {
	Type *Tvoid = Type::getVoidTy(Ctxt);
	Type *Tint32 = Type::getInt32Ty(Ctxt);
	Type *Tptr = PointerType::get(Ctxt, 0);

}

int main() {
	LLVMContext Context;

	Module MyModule("ExampleModule", Context);
	
	build_func(Context, MyModule);

}
