include "llvm/Option/OptParser.td"

class SingleCharFlag<string letter, string help> : Flag<["-"], letter>, HelpText<help>;
class WordFlag<string name, string help> : Flag<["--"], name>, HelpText<help>;

multiclass BB<string name, string help1, string help2> {
	def NAME : Flag<["--"], name>, HelpText<help1>;
	def no_ # NAME: Flag<["--"], "no-" # name>, HelpText<help2>;
}

defm param : BB<"param", "with param", "no param">;
