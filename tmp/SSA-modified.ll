define i32 @test(i1 %cond) {
entry:
  br i1 %cond, label %if.then, label %if.end

if.then:
  br label %if.end

if.end:
  ; The PHI node selects which value to use for %x.
  ; It takes value 1 if control comes from 'entry',
  ; or value 2 if control comes from 'if.then'.
  %x.ssa = phi i32 [ 1, %entry ], [ 2, %if.then ]
  ret i32 %x.ssa
}