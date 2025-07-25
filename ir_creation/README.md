# llvm ir note

## noundef 
If it's not marked `noundef`, LLVM must assume it might be garbage or poison — so it can’t optimize as much.

## dso_local
tell that the function can be use without dynamic linking
by default `static` are `dso_local` but a function can if not exported if you don't put `dso_local` a function is `dso_preemptable` by default to say that he can be dynamicly replace

## phi node
pick the right value depending on wich block your comming from

## tail
`tail` marks a function call for potential tail call optimization 
it's valid when it's the last action in a block before a `ret`, `br`, or `unreachable`, and doesn’t require preserving the stack frame.

## callee caller

Caller = the function making the call
Callee = the function being called
