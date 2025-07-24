declare i32 @baz(...)
declare void @bar(i32 noundef)

define  void @foo(i32 %a, i32 %b) {
	%a.addr = alloca i32
	%b.addr = alloca i32
	%var = alloca i32

	store i32 %a, ptr %a.addr
	store i32 %b, ptr %b.addr

	%1 = load i32, ptr %a.addr
	%2 = load i32, ptr %b.addr

	%add = add i32 %1, %2
	store i32 %add, ptr %var

	%3 = load i32, ptr %var
	%cmp = icmp eq i32 %3, 255
	br i1 %cmp, label %if.then, label %if.end

if.then:
	%4 = load i32, ptr %var
	call void @bar(i32 %4)
	%call = call i32 @baz()
	store i32 %call, ptr %var
	br label %if.end

if.end:
	%i5 = load i32, ptr %var
	call i32 @bar(i32 %i5)
	ret void
}

define void @foo_optimise(i32 noundef %a, i32 noundef %b) {
entry:
	%var.addr = alloca i32, align 4
	%add = add i32 %a, %b
	store i32 %add, ptr %var.addr, align 4

	%var.load = load i32, ptr %var.addr, align 4
	%varcmp = icmp eq i32 %var.load, 255
	br i1 %varcmp, label %if.then, label %if.end

if.then:
	call void @bar(i32 noundef %var.load)
	%baz.ret = call i32 (...) @baz()
	store i32 %baz.ret, ptr %var.addr, align 4
	br label %if.end

if.end:
	%var.final = load i32, ptr %var.addr, align 4
	call void @bar(i32 noundef %var.final)
	ret void
}

define void @foo_optimise2(i32 noundef %a, i32 noundef %b) {
entry:
	%var = add i32 %a, %b
	%varcmpeq = icmp eq i32 %var, 255
	br i1 %varcmpeq, label %if.then, label %if.end

if.then:
	tail call void @bar(i32 noundef %var)
	%baz.ret = tail call i32 (...) @baz()
	br label %if.end

if.end:
	%var.final = phi i32 [ %baz.ret, %if.then ], [ %var, %entry ]
	tail call void @bar(i32 noundef %var.final)
	ret void
}
