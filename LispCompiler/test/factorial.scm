; factorial.scm - Factorial function test
;
; Tests basic functionality:
; - Function definition
; - Recursion
; - Arithmetic operations
; - Conditional expressions

; Recursive factorial
(define (factorial n)
  (if (= n 0)
      1
      (* n (factorial (- n 1)))))

; Test cases
(display "5! = ")
(display (factorial 5))
(newline)

(display "10! = ")
(display (factorial 10))
(newline)

; Tail-recursive version using named let
(define (factorial-tr n)
  (let loop ((n n) (acc 1))
    (if (= n 0)
        acc
        (loop (- n 1) (* n acc)))))

(display "12! (tail-recursive) = ")
(display (factorial-tr 12))
(newline)
