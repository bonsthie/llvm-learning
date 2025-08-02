

just a repo where i put execie form Quentin Colombet's book [LLVM Code Generation: A deep dive into compiler backend development](https://www.oreilly.com/library/view/llvm-code-generation/9781837637782/)
i also have note in my [obsidian vault](https://github.com/bonsthie/obsidian-vault) :)

# NOTE

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

## Type
Type syntax:
* Vector: `<N x Ty>`
* Array: `[N x Ty]`
* strcut : `%MyStruct = type { i32, float, [4 x i8], {Ty1 , Ty2}}`
* anonymous/unnamed sturct : `Struct: {Ty1, Ty2, ...}`

### bfloat16 / vs IEEE-754 half bit-f16
**bfloat**
*1-bit sign
*8-bit exponent
*7-bit mantissa

**IEEE-754 half bit-f16**
*1-bit sign
*5-bit exponent
*10-bit mantissa

### addrspace
In LLVM IR, address spaces let you distinguish different “kinds” of memory—global vs. local vs. GPU‐shared vs. constant, etc.—at the type level. A pointer type is always in some address space, and by default it uses address space 0.
```
i8* addrspace(1)
<T>* addrspace(N)
```
- 0	“Normal” host memory
- 1	Often “global” device memory
- 2	“Constant” memory (read-only)
- 3	GPU local/shared memory (CUDA)

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
in llvm check if a node is critical with `isCriticalEdge`

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

## topological traversal of a CFG

* Depth-First Preorder: visit a block before any of its successors.
* Depth-First Postorder: visit all successors (and their descendants) before the block itself.
* Reverse Postorder: reverse of postorder; approximates a topological order in reducible CFGs.
* Breadth-First Traversal: visit blocks level by level (by “distance” from the entry).
* Topological Sort: true topological order on the DAG of SCCs (requires condensing loops first).

## use object and User object
A User is anything that holds operands (like an instruction), and a Use is one of those operand slots linking the User to a Value.

# LLVM lib/class

## TargetLibraryInfo
Target library info is used by the vectorize pass to know if the target has a vectorized version of the instruction, to vectorize this part of the code using
```cpp
using TargetLoweringInfo::isFunctionVectorizable(StringRef F, const ElementCount &VF)
```

## DataLayout
This class is useful to know information on structs, like its size, with
```cpp
DL.getTypeSizeInBits(Ty)
DL.getABITypeAlignment(Ty)
DL.getTypeAllocSize(Ty)
```

## BlockFrequencyInfo
This class provides execution-frequency estimates for LLVM IR basic blocks—useful for guiding IR-level optimizations (e.g., hot-path inlining or block placement)
```cpp
// Get the relative frequency of a specific BasicBlock
BlockFrequency BFI.getBlockFreq(const BasicBlock *BB);

// Get an estimated profile count (absolute hits) if profile data is available
std::optional<uint64_t> BFI.getBlockProfileCount(const BasicBlock *BB);
```
You can see the BlockFrequencyInfo class on the LLVM IR by using the following command:
```sh
opt -passes='print<block-freq>' input.ll
```


# PASS Manager

* **`-enable-new-pm` / `-disable-new-pm`**
  Toggle the new PassManager in `opt` and Clang. By default, the new PM is enabled in recent releases.

* **`-passes=<pipeline>`**
  Specify a custom pipeline of passes (new PM syntax). Example: `-passes=funcearly-cse,instcombine,gvn`.

* **`-legacy-pass-manager`**
  Force use of the legacy PassManager even if the new PM is available.

* **`-print-before-all` / `-print-after-all`**
  Print the IR before or after every pass to stdout for inspection.

* **`-debug-pass=<Structure|Arguments>`**
  Emit debug output about pass scheduling:

  * `Structure`: shows the pass hierarchy.
  * `Arguments`: lists pass command‑line arguments.

* **`-verify-each`**
  Run LLVM’s verifier after every pass to catch IR invariant violations early.

* **`-time-passes`**
  Measure and report the execution time of each pass.

* **`-stats`**
  Print summary statistics collected by passes (e.g., number of transformations).

* **`-opt-bisect-limit=<n>`**
  Perform a bisect on the pass pipeline to isolate a problematic pass by limiting the first N passes.

* **`-disable-output`**
  Suppress writing any transformed IR or bitcode to disk (useful when only debugging passes).

* **`-view-cfg-after=<PassName>` / `-view-cfg-before=<PassName>`**
  Launch Graphviz to display the CFG after or before a specific pass.


# TablenGen
## warning 
the is multiple TablenGen compiler this change the TablenGen backend
* llvm-tablegen
* clang-tablegen
* mlir-tablegen

## Example
### input

  ```
// Superclass with arguments and default fields
class BaseClass<string baseName, int baseValue> {
  string baseField = baseName;
  int    baseNum   = baseValue;
}

// Another superclass for demonstration
class Flags<int flagBits> {
  int bits = flagBits;
}

// Now a subclass using everything
class MyClass<string name, int size> : BaseClass<name, size>, Flags<size * 2> {
  string myName = name;
  int    mySize = size;
  bit    isActive = 1;
}
  ```
### result

```
def Example {
  string baseField = "Widget";
  int    baseNum   = 8;
  int    bits      = 16;
  string myName    = "Widget";
  int    mySize    = 8;
  bit    isActive  = 1;
}
```

## Types

int : 64 bits int
bit : bool
bits<size> : sequence of 0 and 1
string : string
list<type> : table of type
dag : type is a special construct used to encode tree-like data structure

### dag
```
class DAGHolder<dag P> {
  dag pat = P;
}

def WithDAG : DAGHolder<(mul $a, (add $b, $c))>;
```
you can type it too
```
(add $lhs, $rhs)   // unnamed operands
(add GPR:$lhs, GPR:$rhs) // typed and named
```

## let
let : let you (aha) redefine a var in the next line `def` or in the next `scope {}`

## operator
! bang bang into the room !
 
