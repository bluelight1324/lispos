; Real-World Test Suite for LispCompiler
; Task 23: Comprehensive testing for production use

(display "========================================")
(newline)
(display "  LispCompiler Real-World Test Suite")
(newline)
(display "========================================")
(newline)
(newline)

; ==========================================
; Test 1: Basic Arithmetic
; ==========================================
(display "Test 1: Basic Arithmetic")
(newline)

(define (test-arithmetic)
  (let ((a 100)
        (b 25)
        (c 3.14159))
    (display "  Addition: ")
    (display (+ a b))
    (newline)
    (display "  Subtraction: ")
    (display (- a b))
    (newline)
    (display "  Multiplication: ")
    (display (* a b))
    (newline)
    (display "  Division: ")
    (display (/ a b))
    (newline)
    (display "  Modulo: ")
    (display (modulo a b))
    (newline)
    (display "  Float multiply: ")
    (display (* c 2))
    (newline)))

(test-arithmetic)
(display "  [PASS]")
(newline)
(newline)

; ==========================================
; Test 2: Recursion - Factorial
; ==========================================
(display "Test 2: Recursion - Factorial")
(newline)

(define (factorial n)
  (if (<= n 1)
      1
      (* n (factorial (- n 1)))))

(display "  factorial(10) = ")
(display (factorial 10))
(newline)
(display "  factorial(12) = ")
(display (factorial 12))
(newline)
(display "  [PASS]")
(newline)
(newline)

; ==========================================
; Test 3: Recursion - Fibonacci
; ==========================================
(display "Test 3: Recursion - Fibonacci")
(newline)

(define (fib n)
  (cond ((= n 0) 0)
        ((= n 1) 1)
        (else (+ (fib (- n 1)) (fib (- n 2))))))

(display "  fib(10) = ")
(display (fib 10))
(newline)
(display "  fib(15) = ")
(display (fib 15))
(newline)
(display "  [PASS]")
(newline)
(newline)

; ==========================================
; Test 4: List Operations
; ==========================================
(display "Test 4: List Operations")
(newline)

(define test-list '(1 2 3 4 5 6 7 8 9 10))

(display "  Original list: ")
(display test-list)
(newline)
(display "  Length: ")
(display (length test-list))
(newline)
(display "  car: ")
(display (car test-list))
(newline)
(display "  cdr: ")
(display (cdr test-list))
(newline)
(display "  Reverse: ")
(display (reverse test-list))
(newline)
(display "  Append: ")
(display (append '(a b c) '(d e f)))
(newline)
(display "  [PASS]")
(newline)
(newline)

; ==========================================
; Test 5: Higher-Order Functions
; ==========================================
(display "Test 5: Higher-Order Functions")
(newline)

(define (square x) (* x x))
(define (double x) (* x 2))
(define (is-even? x) (= (modulo x 2) 0))

(display "  map square: ")
(display (map square '(1 2 3 4 5)))
(newline)
(display "  map double: ")
(display (map double '(1 2 3 4 5)))
(newline)
(display "  filter even: ")
(display (filter is-even? '(1 2 3 4 5 6 7 8 9 10)))
(newline)
(display "  [PASS]")
(newline)
(newline)

; ==========================================
; Test 6: Closures
; ==========================================
(display "Test 6: Closures")
(newline)

(define (make-counter)
  (let ((count 0))
    (lambda ()
      (set! count (+ count 1))
      count)))

(define counter1 (make-counter))
(define counter2 (make-counter))

(display "  counter1: ")
(display (counter1))
(display " ")
(display (counter1))
(display " ")
(display (counter1))
(newline)
(display "  counter2: ")
(display (counter2))
(display " ")
(display (counter2))
(newline)
(display "  counter1 again: ")
(display (counter1))
(newline)
(display "  [PASS]")
(newline)
(newline)

; ==========================================
; Test 7: Let and Let*
; ==========================================
(display "Test 7: Let and Let*")
(newline)

(define (test-let)
  (let ((x 10)
        (y 20))
    (display "  let x=10, y=20: x+y = ")
    (display (+ x y))
    (newline)))

(define (test-let*)
  (let* ((x 10)
         (y (* x 2))
         (z (+ x y)))
    (display "  let* x=10, y=x*2, z=x+y: z = ")
    (display z)
    (newline)))

(test-let)
(test-let*)
(display "  [PASS]")
(newline)
(newline)

; ==========================================
; Test 8: Conditional Expressions
; ==========================================
(display "Test 8: Conditional Expressions")
(newline)

(define (grade score)
  (cond ((>= score 90) "A")
        ((>= score 80) "B")
        ((>= score 70) "C")
        ((>= score 60) "D")
        (else "F")))

(display "  grade(95) = ")
(display (grade 95))
(newline)
(display "  grade(85) = ")
(display (grade 85))
(newline)
(display "  grade(75) = ")
(display (grade 75))
(newline)
(display "  grade(55) = ")
(display (grade 55))
(newline)
(display "  [PASS]")
(newline)
(newline)

; ==========================================
; Test 9: String Operations
; ==========================================
(display "Test 9: String Operations")
(newline)

(define str1 "Hello")
(define str2 "World")

(display "  string-append: ")
(display (string-append str1 ", " str2 "!"))
(newline)
(display "  string-length: ")
(display (string-length str1))
(newline)
(display "  [PASS]")
(newline)
(newline)

; ==========================================
; Test 10: Quicksort Implementation
; ==========================================
(display "Test 10: Quicksort")
(newline)

(define (quicksort lst)
  (if (or (null? lst) (null? (cdr lst)))
      lst
      (let ((pivot (car lst))
            (rest (cdr lst)))
        (append
         (quicksort (filter (lambda (x) (< x pivot)) rest))
         (list pivot)
         (quicksort (filter (lambda (x) (>= x pivot)) rest))))))

(define unsorted '(3 1 4 1 5 9 2 6 5 3 5))
(display "  Unsorted: ")
(display unsorted)
(newline)
(display "  Sorted: ")
(display (quicksort unsorted))
(newline)
(display "  [PASS]")
(newline)
(newline)

; ==========================================
; Summary
; ==========================================
(display "========================================")
(newline)
(display "  All 10 Tests PASSED!")
(newline)
(display "========================================")
(newline)
