;;; Core Test Suite - Essential #5
;;; Comprehensive tests for production readiness
;;; This test suite verifies all core functionality

(define test-count 0)
(define pass-count 0)
(define fail-count 0)

(define (test name expected actual)
  (set! test-count (+ test-count 1))
  (if (equal? expected actual)
      (begin
        (set! pass-count (+ pass-count 1))
        (display "[PASS] ")
        (display name)
        (newline))
      (begin
        (set! fail-count (+ fail-count 1))
        (display "[FAIL] ")
        (display name)
        (display " - Expected: ")
        (display expected)
        (display ", Got: ")
        (display actual)
        (newline))))

(define (test-section name)
  (newline)
  (display "=== ")
  (display name)
  (display " ===")
  (newline))

;; ============================================================
;; Section 1: Basic Arithmetic
;; ============================================================
(test-section "Basic Arithmetic")

(test "addition" 5 (+ 2 3))
(test "subtraction" 7 (- 10 3))
(test "multiplication" 24 (* 4 6))
(test "division" 5 (/ 15 3))
(test "modulo" 2 (modulo 17 5))
(test "negative numbers" -5 (- 3 8))
(test "float addition" 5.5 (+ 2.5 3.0))
(test "mixed arithmetic" 14 (+ (* 3 4) 2))
(test "nested arithmetic" 50 (* (+ 2 3) (- 15 5)))
(test "zero" 0 (+ 0 0))

;; ============================================================
;; Section 2: Comparisons
;; ============================================================
(test-section "Comparisons")

(test "equal numbers" #t (= 5 5))
(test "not equal" #f (= 5 6))
(test "less than true" #t (< 3 5))
(test "less than false" #f (< 5 3))
(test "greater than true" #t (> 10 5))
(test "greater than false" #f (> 5 10))
(test "less or equal true" #t (<= 5 5))
(test "greater or equal true" #t (>= 5 5))

;; ============================================================
;; Section 3: Boolean Operations
;; ============================================================
(test-section "Boolean Operations")

(test "and true" #t (and #t #t))
(test "and false" #f (and #t #f))
(test "or true" #t (or #f #t))
(test "or false" #f (or #f #f))
(test "not true" #f (not #t))
(test "not false" #t (not #f))

;; ============================================================
;; Section 4: List Operations
;; ============================================================
(test-section "List Operations")

(test "car" 1 (car '(1 2 3)))
(test "cdr" '(2 3) (cdr '(1 2 3)))
(test "cons" '(1 2 3) (cons 1 '(2 3)))
(test "list" '(1 2 3) (list 1 2 3))
(test "length" 5 (length '(a b c d e)))
(test "null? true" #t (null? '()))
(test "null? false" #f (null? '(1)))
(test "pair? true" #t (pair? '(1 . 2)))
(test "pair? false" #f (pair? 5))
(test "append" '(1 2 3 4) (append '(1 2) '(3 4)))
(test "reverse" '(3 2 1) (reverse '(1 2 3)))
(test "list-ref" 'c (list-ref '(a b c d) 2))

;; ============================================================
;; Section 5: Higher-Order Functions
;; ============================================================
(test-section "Higher-Order Functions")

(test "map square" '(1 4 9) (map (lambda (x) (* x x)) '(1 2 3)))
(test "map add1" '(2 3 4) (map (lambda (x) (+ x 1)) '(1 2 3)))
(test "filter even" '(2 4 6) (filter even? '(1 2 3 4 5 6)))
(test "filter positive" '(1 2 3) (filter positive? '(-1 0 1 2 3)))

;; ============================================================
;; Section 6: Lambda and Closures
;; ============================================================
(test-section "Lambda and Closures")

(define (make-adder n)
  (lambda (x) (+ x n)))

(define add5 (make-adder 5))
(define add10 (make-adder 10))

(test "closure add5" 12 (add5 7))
(test "closure add10" 17 (add10 7))

(define (make-counter)
  (let ((count 0))
    (lambda ()
      (set! count (+ count 1))
      count)))

(define counter1 (make-counter))
(define counter2 (make-counter))

(test "counter1 first" 1 (counter1))
(test "counter1 second" 2 (counter1))
(test "counter2 first" 1 (counter2))
(test "counter1 third" 3 (counter1))

;; ============================================================
;; Section 7: Let and Let*
;; ============================================================
(test-section "Let and Let*")

(test "let basic" 10 (let ((x 5)) (* x 2)))
(test "let multiple" 11 (let ((x 3) (y 8)) (+ x y)))
(test "let* sequential" 15 (let* ((x 5) (y (* x 2))) (+ x y)))
(test "let nested" 25
  (let ((x 5))
    (let ((y 10))
      (let ((z 10))
        (+ x y z)))))

;; ============================================================
;; Section 8: Recursion
;; ============================================================
(test-section "Recursion")

(define (factorial n)
  (if (= n 0)
      1
      (* n (factorial (- n 1)))))

(define (fibonacci n)
  (if (< n 2)
      n
      (+ (fibonacci (- n 1))
         (fibonacci (- n 2)))))

(test "factorial 0" 1 (factorial 0))
(test "factorial 5" 120 (factorial 5))
(test "factorial 10" 3628800 (factorial 10))
(test "fibonacci 0" 0 (fibonacci 0))
(test "fibonacci 1" 1 (fibonacci 1))
(test "fibonacci 10" 55 (fibonacci 10))

;; ============================================================
;; Section 9: Conditionals
;; ============================================================
(test-section "Conditionals")

(define (grade score)
  (cond
    ((>= score 90) 'A)
    ((>= score 80) 'B)
    ((>= score 70) 'C)
    ((>= score 60) 'D)
    (else 'F)))

(test "grade A" 'A (grade 95))
(test "grade B" 'B (grade 85))
(test "grade C" 'C (grade 75))
(test "grade D" 'D (grade 65))
(test "grade F" 'F (grade 50))

(test "if true" 'yes (if #t 'yes 'no))
(test "if false" 'no (if #f 'yes 'no))

;; ============================================================
;; Section 10: Strings
;; ============================================================
(test-section "Strings")

(test "string-length" 5 (string-length "hello"))
(test "string-append" "helloworld" (string-append "hello" "world"))
(test "string=?" #t (string=? "abc" "abc"))
(test "string=? false" #f (string=? "abc" "def"))
(test "string<?" #t (string<? "abc" "abd"))
(test "number->string" "42" (number->string 42))
(test "string->number" 42 (string->number "42"))

;; ============================================================
;; Section 11: Type Predicates
;; ============================================================
(test-section "Type Predicates")

(test "number? true" #t (number? 42))
(test "number? false" #f (number? "42"))
(test "string? true" #t (string? "hello"))
(test "string? false" #f (string? 42))
(test "symbol? true" #t (symbol? 'foo))
(test "symbol? false" #f (symbol? "foo"))
(test "list? true" #t (list? '(1 2 3)))
(test "list? false" #f (list? 42))
(test "procedure? true" #t (procedure? (lambda (x) x)))
(test "procedure? false" #f (procedure? 42))
(test "boolean? true" #t (boolean? #t))
(test "boolean? false" #f (boolean? 1))

;; ============================================================
;; Section 12: Vectors
;; ============================================================
(test-section "Vectors")

(define v (make-vector 5 0))
(vector-set! v 0 10)
(vector-set! v 2 20)

(test "vector-length" 5 (vector-length v))
(test "vector-ref 0" 10 (vector-ref v 0))
(test "vector-ref 2" 20 (vector-ref v 2))
(test "vector-ref default" 0 (vector-ref v 1))

;; ============================================================
;; Section 13: Math Functions
;; ============================================================
(test-section "Math Functions")

(test "abs positive" 5 (abs 5))
(test "abs negative" 5 (abs -5))
(test "min" 1 (min 3 1 4 1 5))
(test "max" 5 (max 3 1 4 1 5))
(test "floor" 3.0 (floor 3.7))
(test "ceiling" 4.0 (ceiling 3.2))
(test "round" 4.0 (round 3.5))
(test "zero? true" #t (zero? 0))
(test "zero? false" #f (zero? 1))
(test "positive? true" #t (positive? 5))
(test "negative? true" #t (negative? -5))
(test "even? true" #t (even? 4))
(test "odd? true" #t (odd? 5))

;; ============================================================
;; Section 14: Equality
;; ============================================================
(test-section "Equality")

(test "eq? same symbol" #t (eq? 'a 'a))
(test "eq? different symbols" #f (eq? 'a 'b))
(test "eqv? numbers" #t (eqv? 42 42))
(test "equal? lists" #t (equal? '(1 2 3) '(1 2 3)))
(test "equal? nested" #t (equal? '(1 (2 3) 4) '(1 (2 3) 4)))

;; ============================================================
;; Section 15: Define and Set!
;; ============================================================
(test-section "Define and Set!")

(define x 10)
(test "define" 10 x)
(set! x 20)
(test "set!" 20 x)

(define (double n) (* n 2))
(test "define function" 14 (double 7))

;; ============================================================
;; Summary
;; ============================================================
(newline)
(display "════════════════════════════════════════")
(newline)
(display "  Test Results")
(newline)
(display "════════════════════════════════════════")
(newline)
(display "  Total:  ")
(display test-count)
(newline)
(display "  Passed: ")
(display pass-count)
(newline)
(display "  Failed: ")
(display fail-count)
(newline)
(display "════════════════════════════════════════")
(newline)
(if (= fail-count 0)
    (display "  ✓ ALL TESTS PASSED!")
    (display "  ✗ SOME TESTS FAILED"))
(newline)
