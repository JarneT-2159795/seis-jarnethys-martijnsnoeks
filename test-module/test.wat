(module
  (func (export "add") (param i32) (result i32)
    (local $var i32)
    (local.set $var (i32.const 10))
    local.get 0
    local.get $var
    i32.add
    local.get 0
    i32.add
  )
)
