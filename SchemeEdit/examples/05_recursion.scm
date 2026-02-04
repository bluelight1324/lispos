; ============================================
; Example 5: Recursion
; ============================================
; Recursion is when a function calls itself.
; Every recursive function needs:
; 1. A base case (when to stop)
; 2. A recursive case (calling itself with simpler input)
;
; Press F5 to run!
; ============================================

; Factorial: n! = n * (n-1) * ... * 1
(define (factorial n)
  (if (<= n 1)
      1                           ; Base case
      (* n (factorial (- n 1))))) ; Recursive case

(display "factorial(5) = ")
(display (factorial 5))
(newline)

(display "factorial(10) = ")
(display (factorial 10))
(newline)

; Fibonacci sequence: 0, 1, 1, 2, 3, 5, 8, 13, ...
(define (fib n)
  (if (< n 2)
      n                                    ; Base case
      (+ (fib (- n 1)) (fib (- n 2)))))   ; Recursive case

(display "First 10 Fibonacci numbers: ")
(newline)
(display "fib(0) = ") (display (fib 0)) (newline)
(display "fib(1) = ") (display (fib 1)) (newline)
(display "fib(2) = ") (display (fib 2)) (newline)
(display "fib(3) = ") (display (fib 3)) (newline)
(display "fib(4) = ") (display (fib 4)) (newline)
(display "fib(5) = ") (display (fib 5)) (newline)
(display "fib(10) = ") (display (fib 10)) (newline)

; Countdown using recursion
(define (countdown n)
  (if (<= n 0)
      (begin
        (display "Blastoff!")
        (newline))
      (begin
        (display n)
        (display "...")
        (countdown (- n 1)))))

(newline)
(countdown 5)
