
define  i32 @foo_not_nsw(i32 %a) {
	%1 = add i32 0, 5
	%2 = add i32 0, 3
	%3 = add i32 %1, %2
	%4 = mul i32 %a, %3
	ret i32 %4
}

define  i32 @foo_nsw(i32 %a) {
	%1 = add nsw i32 0, 5
	%2 = add nsw i32 0, 3
	%3 = add nsw i32 %1, %2
	%4 = mul nsw i32 %a, %3
	ret i32 %4
}

define  i32 @foo_nsw_partial(i32 %a) {
	%1 = add nsw i32 0, 5
	%2 = add nsw i32 0, 3
	%3 = add i32 %1, %2
	%4 = mul  i32 %a, %3
	ret i32 %4
}
