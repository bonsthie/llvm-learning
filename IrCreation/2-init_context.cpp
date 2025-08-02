
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/AsmParser/Parser.h>

using namespace llvm;

int main() {
	LLVMContext Context; // LLVMContext
	SMDiagnostic Err;  // SourceMgr

	// create new module
	Module MyModule("Module", Context);

	// create module form file <llvm/IRReader/IRReader.h>
	std::unique_ptr<Module> MyModuleFromFile = parseIRFile("MyModuleFromFile.ll", Err, Context);

	// create form string <llvm/AsmParser/Parser.h>
	std::unique_ptr<Module> MyModuleFromString = parseAssemblyString("declare i32 foo(...)", Err, Context);

}
