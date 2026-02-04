; ============================================
; Example 3: Defining Functions
; ============================================
; Functions are defined with (define (name args) body)
; Call functions with (name arguments)
;
; Press F5 to run!
; ============================================

; A simple function that squares a number
(define (square x)
  (* x x))

(display "square(5) = ")
(display (square 5))
(newline)

; A function with multiple arguments
(define (add a b)
  (+ a b))

(display "add(3, 7) = ")
(display (add 3 7))
(newline)

; A function that calls other functions
(define (sum-of-squares a b)
  (+ (square a) (square b)))

(display "sum-of-squares(3, 4) = ")
(display (sum-of-squares 3 4))
(newline)

; Function with conditional logic
(define (abs x)
  (if (< x 0)
      (- 0 x)
      x))

(display "abs(-5) = ")
(display (abs -5))
(newline)

(display "abs(10) = ")
(display (abs 10))
(newline)
