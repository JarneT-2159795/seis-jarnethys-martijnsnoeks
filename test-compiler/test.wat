(module
  (func (export"compareSixty") (; test  inline comment with ; in the middle ;) (param i32 i32) (result i32)
    local.get 0
    local.get 1
    i32.add ;; test comment with at the end ;) and ;; ;)
    i32.const 10
    i32.const 20
    i32.add
    i32.const 30
    i32.add
    i32.const 60
    i32.eq
    if (result i32)  
        i32.const 1
    else
    	i32.const 0
    	i32.const 1
      call 0      ;; recursive shenanigans
    end
    
    i32.eq
  )
)

;; code should compile to: 

;; 000002c: 20  ; local.get
;; 000002d: 00  ; local index
;; 000002e: 20  ; local.get
;; 000002f: 01  ; local index
;; 0000030: 6a  ; i32.add
;; 0000031: 41  ; i32.const
;; 0000032: 0a  ; i32 literal
;; 0000033: 41  ; i32.const
;; 0000034: 14  ; i32 literal
;; 0000035: 6a  ; i32.add
;; 0000036: 41  ; i32.const
;; 0000037: 1e  ; i32 literal
;; 0000038: 6a  ; i32.add
;; 0000039: 41  ; i32.const
;; 000003a: 3c  ; i32 literal
;; 000003b: 46  ; i32.eq
;; 000003c: 04  ; if
;; 000003d: 7f  ; i32
;; 000003e: 41  ; i32.const
;; 000003f: 01  ; i32 literal
;; 0000040: 05  ; else
;; 0000041: 41  ; i32.const
;; 0000042: 00  ; i32 literal
;; 0000043: 41  ; i32.const
;; 0000044: 01  ; i32 literal
;; 0000045: 10  ; call
;; 0000046: 00  ; function index
;; 0000047: 0b  ; end
;; 0000048: 46  ; i32.eq
;; 0000049: 0b  ; end
