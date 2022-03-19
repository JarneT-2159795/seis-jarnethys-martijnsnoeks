(module
  ;;(func (export "addInt") (param i32 i32) (result i32 i32 i32)
  ;;  local.get 0
  ;;  local.get 1
  ;;  local.get 0
  ;;  local.get 1
  ;;  i32.add
  ;;)
  ;;(func (export "add100") (param i32) (result i32)
  ;;  (local $var i32)
  ;;  (local.set $var (i32.const 100))
  ;;  local.get 0
  ;;  local.get $var
  ;;  i32.add
  ;;)
  ;;(func (export "if") (param i32) (result i32)
  ;;  local.get 0
  ;;  i32.const 0
  ;;  i32.gt_s
  ;;  if (result i32)
  ;;    i32.const 1
  ;;  else
  ;;    i32.const 0
  ;;  end
  ;;)
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
)
