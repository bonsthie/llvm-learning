# LLVM IR & MIR Exploration

This repository demonstrates three different ways to lower a simple C function into LLVM’s intermediate representations:

1. **Hand‑written LLVM IR** at 3 optimization levels
2. **Programmatic IR emission** using the LLVM C++ API (`IRBuilder`)  
3. **Programmatic MIR emission** using the LLVM Machine IR API (`MachineIRBuilder`)

# llvm ir note

## noundef 
If it's not marked `noundef`, LLVM must assume it might be garbage or poison — so it can’t optimize as much.

## dso_local
tell that the function can be use without dynamic linking
by default `static` are `dso_local` but a function can if not exported if you don't put `dso_local` a function is `dso_preemptable` by default to say that he can be dynamicly replace.

## phi node
pick the right value depending on wich block your comming from.

## tail
`tail` marks a function call for potential tail call optimization 
it's valid when it's the last action in a block before a `ret`, `br`, or `unreachable`, and doesn’t require preserving the stack frame.

## callee caller

Caller = the function making the call
Callee = the function being called

## LLVM UB-Enabling Flags Reference

- **nsw** (No Signed Wrap): signed integer overflow ⇒ undefined behavior (UB).
- **nuw** (No Unsigned Wrap): unsigned integer overflow ⇒ undefined behavior (UB).
- **fast** (Fast-math): floating-point operations may be reassociated, ignore NaNs/Infs, etc. ⇒ relaxed IEEE semantics (UB-style optimizations).
- **reassoc**: allow reassociation of floating-point operations (e.g., transform `(x + a) + b` to `x + (a + b)`).

> **Note**: Default semantics are two's-complement wraparound for integers and strict IEEE-754 for floating-point. These flags are only present in IR when explicitly emitted (e.g., via Clang flags such as `-fstrict-overflow` or `-ffast-math`).


### Querying Flags in LLVM

- **LLVM IR level**:
  - `Instruction::hasNoSignedWrap()`, `Instruction::hasNoUnsignedWrap()`, `Instruction::hasNoNaNs()`, etc.
- **Machine IR level**:
  - `MachineInstr::getFlag(MIFlag Flag)` with `MIFlag::NoSignedWrap`, `MIFlag::NoUnsignedWrap`, `MIFlag::FmNoNaNs`, etc.


# Theorical note

## SSA 

`alloca` `load` `store` make the LLVM IR non-SSA because they allow reassigning values to the same memory location (i.e., variable). `mem2reg` and `createPromoteMemoryToRegisterPass`/`PromoteToReg` API` transform this in ssa form with register and phi node
`SSAUpdater` from `TransformUtils` lib.

## def-use use-def

* Def-Use	For a given definition, what are all the uses of that value
* Use-Def	For a given use, what is the definition it depends on

```llvm
define i32 @example() {
entry:
  %a = add i32 5, 2       ; [DEF a]
  br label %next

next:
  %b = add i32 %a, 3      ; [USE a] [DEF b]
  %c = mul i32 %b, 4      ; [USE b] [DEF c]
  ret i32 %c              ; [USE c]
}
```

## backedge
going back in the CFG (ex: loop going back in the CFG)

##  critical edge
An edge that goes from a block with multiple successors to a block with multiple predecessors. Such edges are “critical” for many optimizations and are typically split by inserting a new intermediate block.

## Irreducible graph
A CFG is irreducible if it contains one or more loops with multiple distinct entry points, making it impossible to transform into a hierarchy of single-entry loops.

## proxy (cost model)
A proxy cost model is a cheap heuristic that estimates instruction “cost” by counting or lightly weighting IR ops, letting early passes make decisions without invoking the full, target‑accurate backend model.

## InstCombine 
Is LLVM’s peephole optimization pass that matches small instruction patterns and rewrites them into more efficient or canonical forms (e.g., turning b * 2 into b << 1).

## liveness 
Is about whether a value needs to be kept around for future use.

## hoisting
Move something up in the cfg (ex: moving a block outside of a loop)

## sinking
Opposite of `hoisting` (ex : def of a value outside a if block inside)

## Folding
Is the compile‑time evaluation of constant expressions, replacing operations on known values with their computed literal results to simplify the IR.

## loops

```llvm
define void @loop_example() {
entry:
  br label %preheader

preheader:                                    ; preheader: dominates header, not in loop
  %i = alloca i32
  store i32 0, i32* %i
  br label %header

header:                                       ; header: first block in loop, also exiting block (branches outside)
  %iv = phi i32 [ 0, %preheader ], [ %iv_next, %latch ]
  %cmp = icmp slt i32 %iv, 10
  br i1 %cmp, label %exiting, label %exitblock

exiting:                                      ; exiting block: has a successor (%exitblock) outside the loop
  ; --- loop body ---
  %tmp = mul i32 %iv, 2
  br label %latch

latch:                                        ; latch: back‑edge to header
  %iv_next = add nsw i32 %iv, 1
  br label %header

exitblock:                                   ; exit block: outside the loop
  ret void
}
```
