define signext i32 @f(ptr %p, ptr %q, ptr %r) {
entry:
  br label %bb1

bb1:
  %p.addr = alloca ptr, align 8
  %q.addr = alloca ptr, align 8
  %r.addr = alloca ptr, align 8
  %i = alloca i32, align 4
  store ptr %p, ptr %p.addr, align 8
  store ptr %q, ptr %q.addr, align 8
  store ptr %r, ptr %r.addr, align 8
  store i32 0, ptr %i, align 4
  %0 = load ptr, ptr %r.addr, align 8
  %1 = load i32, ptr %0, align 4
  %tobool = icmp ne i32 %1, 0
  br i1 %tobool, label %if.then, label %if.else

if.then:
  store i32 1, ptr %i, align 4
  br label %bb2

bb2:
  br label %if.end

if.else:
  call void @g()
  br label %if.end

if.end:
  %2 = load ptr, ptr %q.addr, align 8
  %3 = load i32, ptr %2, align 4
  %add = add nsw i32 %3, 1
  %4 = load ptr, ptr %p.addr, align 8
  store i32 %add, ptr %4, align 4
  %5 = load i32, ptr %i, align 4
  %tobool1 = icmp ne i32 %5, 0
  br i1 %tobool1, label %if.then2, label %if.end3

if.then2:
  %6 = load i32, ptr %i, align 4
  %inc = add nsw i32 %6, 1
  br label %bb3

bb3:
  store i32 %inc, ptr %i, align 4
  br label %if.end3

if.end3:
  br label %bb4

bb4:
  %7 = load ptr, ptr %q.addr, align 8
  %8 = load i32, ptr %7, align 4
  ret i32 %8
}

declare void @g(...)