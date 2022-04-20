(module
  (func (export "addInt") (param i32 i32) (result i32)
    local.get 0
    local.get 1
    i32.add
  )

  (func (export "add100") (param i32) (result i32)
    (local $var i32)
    (local.set $var (i32.const 100))
    local.get 0
    local.get $var
    i32.add
  )

  (func $swap (param i32 i32) (result i32 i32)
    local.get 1
    local.get 0
  )

  (func (export "reverseSub") (param i32 i32) (result i32)
    local.get 0
    local.get 1
    call $swap
    i32.sub
  )
  
  ;; https://samrat.me/posts/2020-03-29-webassembly-control-instr-examples/
  ;; if (x == 0)
  ;;   return 42;
  ;; else if (x == 1)
  ;;   return 99;
  ;; else
  ;;   return 7;
  (func (export "blockTest") (param i32) (result i32)
       (local i32)
       (block
           (block
               (block
                   ;; x == 0
                   local.get 0
                   i32.eqz
                   br_if 0
  
                   ;; x == 1
                   local.get 0
                   i32.const 1
                   i32.eq
                   br_if 1
  
                   ;; the `else` case
                   i32.const 7
                   local.set 1
                   br 2)
             i32.const 42
             local.set 1
             br 1)
         i32.const 99
         local.set 1)
       local.get 1
  )

  ;;(func $fac (export "fac") (param f64) (result f64)
  ;;  local.get 0
  ;;  f64.const 1
  ;;  f64.lt
  ;;  if (result f64)
  ;;    f64.const 1
  ;;  else
  ;;    local.get 0
  ;;    local.get 0
  ;;    f64.const 1
  ;;    f64.sub
  ;;    call $fac
  ;;    f64.mul
  ;;  end
  ;;)

  ;; if (x < 0) {
  ;;   return -1
  ;; } else {
  ;;   return 1
  ;; }
  (func (export "ifTest") (param i32) (result i32)
    local.get 0
    i32.const 0
    i32.lt_s
    if (result i32)
      i32.const -1
    else
      i32.const 1
    end
  )
)
