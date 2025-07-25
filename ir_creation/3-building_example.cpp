#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/SourceMgr.h>

using namespace llvm;

void build_func(LLVMContext &Ctxt, Module &Mod) {
	Type *Tyvoid = Type::getVoidTy(Ctxt);
	Type *Tyint32 = Type::getInt32Ty(Ctxt);
	Type *Typtr = PointerType::get(Ctxt, 0);

	FunctionType *BazTy = FunctionType::get(/* ret */ Tyint32, false);
	Function *BazFunc = cast<Function>(Mod.getOrInsertFunction("baz", BazTy).getCallee());

	FunctionType *BarTy = FunctionType::get( /* ret */ Tyvoid, /* args */ ArrayRef(Tyint32), false);
	Function *BarFunc = cast<Function>(Mod.getOrInsertFunction("bar", BarTy).getCallee());

	FunctionType *FooTy = FunctionType::get(Tyvoid, ArrayRef({Tyint32, Tyint32}), false);
	Function *FooFunc = cast<Function>(Mod.getOrInsertFunction("foo", FooTy).getCallee());

	BasicBlock *BBEntry = BasicBlock::Create(Ctxt, "entry", FooFunc);
	BasicBlock *BBIf_then = BasicBlock::Create(Ctxt, "if.then", FooFunc);
	BasicBlock *BBIf_end = BasicBlock::Create(Ctxt, "if.end", FooFunc);

	// entry:
	IRBuilder Builder(BBEntry);

	Value *Va_addr = Builder.CreateAlloca(Tyint32, 0, nullptr,"a.addr");
	Value *Vb_addr = Builder.CreateAlloca(Tyint32, 0, nullptr,"b.addr");
	Value *Vvar = Builder.CreateAlloca(Tyint32, 0, nullptr,"var");

	Value *Va = FooFunc->getArg(0);
	Va->setName("a");
	Value *Vb = FooFunc->getArg(1);
	Vb->setName("b");

	Builder.CreateStore(Va, Va_addr);
	Builder.CreateStore(Vb, Vb_addr);

	Value *V1 = Builder.CreateLoad(Tyint32, Va_addr);
	Value *V2 = Builder.CreateLoad(Tyint32, Vb_addr);

	Value *Vadd = Builder.CreateAdd(V1, V2);
	Builder.CreateStore(Vadd, Vvar);

	Value *V3 = Builder.CreateLoad(Tyint32, Vvar);
	Value *Vconst255 = ConstantInt::get(Tyint32, 0xFF);
	Value *Vcmp = Builder.CreateICmpEQ(V3, Vconst255);
	Builder.CreateCondBr(Vcmp, BBIf_then, BBIf_end);

	// if.then:
	Builder.SetInsertPoint(BBIf_then);
	Value *V4 = Builder.CreateLoad(Tyint32, Vvar);
	Builder.CreateCall(BarFunc->getFunctionType(), BarFunc, ArrayRef(V4));
	Value *Vcall = Builder.CreateCall(BazFunc->getFunctionType(),  BazFunc);
	Builder.CreateStore(Vcall, Vvar);
	Builder.CreateBr(BBIf_end);

	// if.end
	Builder.SetInsertPoint(BBIf_end);
	Value *V5 = Builder.CreateLoad(Tyint32, Vvar);
	Builder.CreateCall(BarFunc->getFunctionType(), BarFunc, ArrayRef(V5));
	Builder.CreateRetVoid();
}

int main() {
	LLVMContext Context;

	Module MyModule("ExampleModule", Context);
	
	build_func(Context, MyModule);

	MyModule.print(outs(), nullptr);
}
