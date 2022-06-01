(module
  (func (export "weirdBlocks") (param i32) (result i32) (local i32)
       (block
           (block
               (block
                   ;; x == 0
                   local.get 0
                   i32.eqz
                   br_if 0  ;; relative to current block, so this one

                   ;; x == 1
                   local.get 0
                   i32.const 1
                   i32.eq
                   br_if 1  ;; relative to the current block, so the one before

                   ;; the `else` case
                   i32.const 7
                   local.set 1
                   br 2)    ;; relative to the current block, so the top-level one

             i32.const 42   ;; br 0 went here
             local.set 1
             br 1)
         i32.const 99       ;; br 1  went here
         local.set 1)
       local.get 1          ;; br 2  went here
    )
)               
