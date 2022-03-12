(module
  (func (export "addInt") (param i32) (result i32)
    (local $var1 i32)
    (local $var2 f32)
    (local $var3 f32)
    (local.set $var3 (f32.const -123.456))
    (local.set $var1 (i32.const 123456))
    (local.set $var2 (f32.const 123.456))
    local.get 0
    local.get $var1
    i32.add
  )
  (func (export "addFloat") (param f32 f32) (result f32)
    local.get 0
    local.get 1
    f32.add
  )
)
