
#include <llvm/CodeGen/GlobalISel/Utils.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/SourceMgr.h>

#include "llvm/CodeGen/CodeGenTargetMachineImpl.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/Register.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"

#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h" // MachineIrBuilder
#include "llvm/CodeGen/MachineFrameInfo.h"            // For CreateStackObject.
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h" // For InitializeAllTargets

using namespace llvm;

MachineFunction *PopulateMachineIR(MachineModuleInfo &MMI, Function &Foo,
                                   Register W0, Register W1) {
  MachineFunction &MF = MMI.getOrCreateMachineFunction(Foo);
  // The type for bool.
  LLT I1 = LLT::scalar(1);
  // The type of var.
  LLT I32 = LLT::scalar(32);

  // To use to create load and store for var.
  MachinePointerInfo PtrInfo;
  Align VarStackAlign(4);

  // The type for the address of var.
  LLT VarAddrLLT = LLT::pointer(/*AddressSpace=*/0, /*SizeInBits=*/64);

  // The stack slot for var.
  int FrameIndex = MF.getFrameInfo().CreateStackObject(32, VarStackAlign,
                                                       /*IsSpillSlot=*/false);

  MachineBasicBlock *BBEntry = MF.CreateMachineBasicBlock();
  MF.push_back(BBEntry);
  MachineBasicBlock *BBIf_then = MF.CreateMachineBasicBlock();
  MF.push_back(BBIf_then);
  MachineBasicBlock *BBIf_end = MF.CreateMachineBasicBlock();
  MF.push_back(BBIf_end);

  // config the CFG
  BBEntry->addSuccessor(BBIf_then);
  BBEntry->addSuccessor(BBIf_end);
  BBIf_then->addSuccessor(BBIf_end);

  // Entry
  MachineIRBuilder MIRBuilder(*BBEntry, BBEntry->end());

  Register A = MIRBuilder.buildCopy(I32, W0).getReg(0);
  Register B = MIRBuilder.buildCopy(I32, W1).getReg(0);

  Register VarStackAddr =
      MIRBuilder.buildFrameIndex(VarAddrLLT, FrameIndex).getReg(0);

  Register Add = MIRBuilder.buildAdd(I32, A, B).getReg(0);
  MIRBuilder.buildStore(Add, VarStackAddr, PtrInfo, VarStackAlign);
  Register ReloadedVar0 =
      MIRBuilder.buildLoad(I32, VarStackAddr, PtrInfo, VarStackAlign).getReg(0);

  Register Const0xFF = MIRBuilder.buildConstant(I32, 0xFF).getReg(0);
  Register Cmp =
      MIRBuilder.buildICmp(CmpInst::ICMP_EQ, I1, ReloadedVar0, Const0xFF)
          .getReg(0);
  MIRBuilder.buildBrCond(Cmp, *BBIf_then);
  MIRBuilder.buildBr(*BBIf_end);

  // if.then
  MIRBuilder.setInsertPt(*BBIf_then, BBIf_then->end());
  // store in W0 for call
  Register ReloadedVar1 =
      MIRBuilder.buildLoad(I32, VarStackAddr, PtrInfo, VarStackAlign).getReg(0);
  MIRBuilder.buildCopy(W0, ReloadedVar1);
  //
  // Fake call to bar.
  MIRBuilder.buildInstr(TargetOpcode::INLINEASM, {}, {})
      .addExternalSymbol("bl @bar")
      .addImm(0)
      .addReg(W0, RegState::Implicit);
  // Fake call to baz.
  MIRBuilder.buildInstr(TargetOpcode::INLINEASM, {}, {})
      .addExternalSymbol("bl @baz")
      .addImm(0)
      .addReg(W0, RegState::Implicit | RegState::Define);
  // Copy the result of baz to var.
  Register ResOfBaz = MIRBuilder.buildCopy(I32, W0).getReg(0);
  MIRBuilder.buildStore(ResOfBaz, VarStackAddr, PtrInfo, VarStackAlign);

  // if end
  MIRBuilder.setInsertPt(*BBIf_end, BBIf_end->end());
  // Put var in W0 for the call to bar.
  Register ReloadedVar2 =
      MIRBuilder.buildLoad(I32, VarStackAddr, PtrInfo, VarStackAlign).getReg(0);
  MIRBuilder.buildCopy(W0, ReloadedVar2);
  // Fake call to bar.
  MIRBuilder.buildInstr(TargetOpcode::INLINEASM, {}, {})
      .addExternalSymbol("bl @bar")
      .addImm(0)
      .addReg(W0, RegState::Implicit);
  // End of the function, return void;
  MIRBuilder.buildInstr(TargetOpcode::INLINEASM, {}, {})
      .addExternalSymbol("ret")
      .addImm(0);

  return &MF;
}

///
///
/// BASE
///
///

bool checkFunctionCorrectness(MachineFunction *Res, Register W0, Register W1) {
  // Take care of the liveness since we did not explain how to do that.
  MachineBasicBlock *EntryBB = Res->empty() ? nullptr : &*Res->begin();
  if (EntryBB) {
    EntryBB->addLiveIn(W0);
    EntryBB->addLiveIn(W1);
  }
  Res->print(errs());
  if (!Res->verify()) {
    errs() << Res->getName() << " does not verify\n";
    return false;
  }

  return true;
}

int main() {
  // We have to initialize all the targets to get the registry initialized.
  InitializeAllTargets();
  // We need the MC layer as well to query the register information.
  InitializeAllTargetMCs();

  auto TT(Triple::normalize("aarch64--"));
  std::string Error;
  const Target *TheTarget = TargetRegistry::lookupTarget(TT, Error);
  if (!TheTarget) {
    errs() << TT << " is not available with this build of LLVM\n";
    return -1;
  }
  auto *LLVMTM = static_cast<CodeGenTargetMachineImpl *>(
      TheTarget->createTargetMachine(TT, "", "", TargetOptions(), std::nullopt,
                                     std::nullopt, CodeGenOptLevel::Default));
  MachineModuleInfoWrapperPass MMIWP(LLVMTM);
  LLVMContext Context;
  Module MyModule("MyModule", Context);
  MyModule.setDataLayout(LLVMTM->createDataLayout());

  Function *Foo = Function::Create(
      FunctionType::get(Type::getVoidTy(Context), /*IsVarArg=*/false),
      Function::ExternalLinkage, "foo", MyModule);
  const TargetSubtargetInfo *STI = LLVMTM->getSubtargetImpl(*Foo);
  const TargetRegisterInfo *TRI = STI->getRegisterInfo();

  // Find the indices for W0 and W1.
  // Since we are not in AArch64 library we don't have access to the AArch64::W0
  // enums.
  StringRef W0Str = "W0";
  StringRef W1Str = "W1";
  Register W0 = 0;
  Register W1 = 0;
  for (unsigned i = 1, e = TRI->getNumRegs(); i != e && (!W0 || !W1); ++i) {
    if (!W0 && W0Str == TRI->getName(i)) {
      W0 = i;
      continue;
    }
    if (!W1 && W1Str == TRI->getName(i)) {
      W1 = i;
      continue;
    }
  }

  if (!W0 || !W1) {
    errs() << "Failed to found physical registers w0 and w1\n";
    return -1;
  }

  MachineFunction *YourTurnRes =
      PopulateMachineIR(MMIWP.getMMI(), *Foo, W0, W1);

  bool yourTurnIsCorrect = checkFunctionCorrectness(YourTurnRes, W0, W1);

  return yourTurnIsCorrect;
}
