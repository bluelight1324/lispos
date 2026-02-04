; VS Code Integration Test File
; This file tests that VS Code can run Scheme files correctly

(display "=== VS Code Integration Test ===")
(newline)

; Test 1: Basic arithmetic
(display "Test 1 - Arithmetic: ")
(display (+ 10 20 30))
(newline)

; Test 2: Define and use function
(define (square x) (* x x))
(display "Test 2 - Square of 7: ")
(display (square 7))
(newline)

; Test 3: Recursion
(define (factorial n)
  (if (<= n 1)
      1
      (* n (factorial (- n 1)))))

(display "Test 3 - Factorial of 5: ")
(display (factorial 5))
(newline)

; Test 4: List operations
(define my-list '(1 2 3 4 5))
(display "Test 4 - List: ")
(display my-list)
(newline)

(display "Test 4 - Length: ")
(display (length my-list))
(newline)

; Test 5: Higher-order functions
(display "Test 5 - Map square: ")
(display (map square my-list))
(newline)

(display "=== All Tests Passed ===")
(newline)
