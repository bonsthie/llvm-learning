class MyClass<string _alias=""> {
	string alias = _alias;
}

let alias = "let from out" in
def A: MyClass<> {}
def B: MyClass<> {
	let alias = "let from body";
}
def C: MyClass<"from arg">;
let alias = "alias from bigger scope" in {
	let alias = "let from out" in
		def D: MyClass<"from arg"> {
			let alias = "let from body";
	}
	def E: MyClass<"will be overridden">;
} // end "alias from bigger scope"

let alias = "oooooo" in {
	def O1: MyClass<> {}
	def O2: MyClass<> {}
	let alias = "innnn" in
		def O3: MyClass<> {}
}
def O4: MyClass<> {}
