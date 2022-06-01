(module
  (func (export "dualAnswerFastReturn") (param i32) (result i32)
        local.get 0
        i32.const 1
        i32.eq         
        if (result i32)
            local.get 0
            i32.const 5
            i32.add
        else
            local.get 0
            i32.const 5
            i32.mul
        end
    )
)