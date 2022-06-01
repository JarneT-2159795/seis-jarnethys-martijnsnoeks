(module
  (import "env" "memory" (memory 1))
  (func (export "memory") (result i32)
    i32.const 0
    i32.const 4
    i32.store offset=5
    i32.const 5
    i32.load ;; returns 4
)
)
