#include "llvm/Option/Arg.h"
#include "llvm/Option/OptTable.h"
#include "llvm/Option/ArgList.h"
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

namespace {
enum ID {
  OPT_INVALID = 0, // This is not an option ID.
#define OPTION(...) LLVM_MAKE_OPT_ID(__VA_ARGS__),
#include "ArgTest.inc"
#undef OPTION
};

#define OPTTABLE_STR_TABLE_CODE
#include "ArgTest.inc"
#undef OPTTABLE_STR_TABLE_CODE

#define OPTTABLE_PREFIXES_TABLE_CODE
#include "ArgTest.inc"
#undef OPTTABLE_PREFIXES_TABLE_CODE

using namespace llvm::opt;
static constexpr opt::OptTable::Info InfoTable[] = {
#define OPTION(...) LLVM_CONSTRUCT_OPT_INFO(__VA_ARGS__),
#include "ArgTest.inc"
#undef OPTION
};

class TestOptionTable : public opt::GenericOptTable {
public:
  TestOptionTable()
      : opt::GenericOptTable(OptionStrTable, OptionPrefixesTable, InfoTable) {}
};

}; // namespace

int main(int ac, char **av) {
  TestOptionTable opt;
  SmallVector<const char *, 16> ArgsVec(av + 1, av + ac);
  unsigned MissingArgIndex, MissingArgCount;

  InputArgList Args = opt.ParseArgs(ArgsVec, MissingArgIndex, MissingArgCount);

	if (Args.hasArg(OPT_param)) {
		llvm::outs() << "has args" << "\n";
	} else if  (Args.hasArg(OPT_no_param)) {
		llvm::outs() << "has no args" << "\n";
	}
}
