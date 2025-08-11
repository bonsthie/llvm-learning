include "llvm/Target/Target.td"

def : HwMode<"", []>;
def myii: InstrInfo;
def YourTurn : Target {
  let InstructionSet = myii;
}

let Namespace = "yourTurn" in {
// Complete this description to describe a register hierarchy that looks like:
//               +-----------+-----------+-----------+
// 128-bit quad  |    q0     |    q1     |     q2    |
//               +-----+-----+-----+-----+-----------+
// 64-bit double |  d0 | d1  |  d2 |     |           |
//               +--+--+--+--+-----+-----+-----------+
// 32-bit single |s0|s1|s2|  |           |           |
//               +--+--+--+--+-----------+-----------+
//
// Empty cells mean that no architectural register exists for that cell (it
// is not addressable.)

def sub32_low: SubRegIndex<32>;
def sub32_high: SubRegIndex<32, 32>;

def sub64_low: SubRegIndex<64>;
def sub64_high: SubRegIndex<64, 64>;

def s0 : Register<"s0">;
def s1 : Register<"s1">;
def s2 : Register<"s2">;

def d0 : Register<"d0"> {
		let SubRegIndices = [sub32_low, sub32_high];
		let SubRegs = [ s0, s1 ];
		let CoveredBySubRegs = true;
}

def d1 : Register<"d1"> {
		let SubRegIndices = [sub32_low];
		let SubRegs = [ s2 ];
		let CoveredBySubRegs = false;

}

def d2 : Register<"d2">;

def q0 : Register<"q0"> {
		let SubRegIndices = [sub64_low, sub64_high];
		let SubRegs = [ d0, d1 ];
		let CoveredBySubRegs = true;
}

def q1 : Register<"q1"> {
		let SubRegIndices = [sub64_low];
		let SubRegs = [ d2 ];
		let CoveredBySubRegs = false;
}

def q2 : Register<"q2">;

} // end namespace yourTurn.

// Bonus point, create 1 register class per level.
// They would map respectively to f128, f64, and f32.
def SINGLES : RegisterClass<"yourTurn", [f32], 32, (sequence "s%u", 0, 2)>;
def DOUBLES : RegisterClass<"yourTurn", [f64], 64, (sequence "d%u", 0, 2)>;
def QUADS : RegisterClass<"yourTurn", [f128], 128, (sequence "q%u", 0, 2)>;

