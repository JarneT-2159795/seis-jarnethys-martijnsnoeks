(module
  ;; dualAnswer( int A ) {
    ;;     if ( A == 0 ) {
    ;;         A = A + 5;
    ;;     }
    ;;     else {
    ;;         A = A * 5;
    ;;     }
    ;;     return A + 10;
    ;; }
    (func (export "dualAnswer") (param i32) (result i32)
        local.get 0
        i32.const 1
        i32.eq 
        if                  ;; expects 0 or 1
            local.get 0
            i32.const 5
            i32.add
      		local.set 0
        else
            local.get 0
            i32.const 5
            i32.mul
      		local.set 0
        end
		
      	local.get 0
        i32.const 10
        i32.add
    )
)               
        