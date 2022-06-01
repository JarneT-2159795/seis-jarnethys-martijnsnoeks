(module
  (func (export "whileOne") (result i32) (local $i i32)
        i32.const 5
        local.set $i  ;; i = first input parameter

        (loop
            ;; add one to $i
            local.get $i
            i32.const 1
            i32.add
            local.set $i

            ;; if $i is less than 10, jump back to loop
            local.get $i
            i32.const 10
            i32.lt_s        ;; _s = signed variant of lt
            br_if 0
        )
      
        local.get $i ;; return final value of $i, should be 10
    )
)