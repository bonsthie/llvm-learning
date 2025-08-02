
declare i32 @foo()
declare void @rhaaaa()

define void @bar(i32 noundef %yes) {
entry:
	ret void
}

define i32 @foobar() {
entry:
	%tmp = call i32 @foo()
	ret i32 %tmp
}
