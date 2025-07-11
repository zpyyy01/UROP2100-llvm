define i32 @test(i1 %cond) {
entry:
  ; 1. Allocate memory on the stack for variable 'x'.
  %x.addr = alloca i32, align 4

  ; 2. First assignment: store 1 into 'x'.
  store i32 1, ptr %x.addr, align 4
  br i1 %cond, label %if.then, label %if.end

if.then:
  ; 3. Second assignment: store 2 into 'x'.
  store i32 2, ptr %x.addr, align 4
  br label %if.end

if.end:
  ; 4. Load the final value of 'x' from memory to return it.
  %retval = load i32, ptr %x.addr, align 4
  ret i32 %retval
}