; comprehensive.scm - Comprehensive Test Suite
;
; Tests all major language features for production readiness

; ===========================================================================
; Test 1: Basic Arithmetic
; ===========================================================================
(display "=== Test 1: Basic Arithmetic ===\n")

(display "Addition: (+ 1 2 3 4 5) = ")
(display (+ 1 2 3 4 5))
(newline)

(display "Subtraction: (- 10 3) = ")
(display (- 10 3))
(newline)

(display "Unary minus: (- 42) = ")
(display (- 42))
(newline)

(display "Multiplication: (* 2 3 4) = ")
(display (* 2 3 4))
(newline)

(display "Division: (/ 100 4) = ")
(display (/ 100 4))
(newline)

(display "Modulo: (mod 17 5) = ")
(display (mod 17 5))
(newline)

(display "Absolute: (abs -42) = ")
(display (abs -42))
(newline)

; ===========================================================================
; Test 2: Comparisons
; ===========================================================================
(display "\n=== Test 2: Comparisons ===\n")

(display "(= 5 5) = ")
(display (= 5 5))
(newline)

(display "(< 3 5) = ")
(display (< 3 5))
(newline)

(display "(> 10 5) = ")
(display (> 10 5))
(newline)

(display "(<= 5 5) = ")
(display (<= 5 5))
(newline)

(display "(>= 10 5) = ")
(display (>= 10 5))
(newline)

; ===========================================================================
; Test 3: Boolean Logic
; ===========================================================================
(display "\n=== Test 3: Boolean Logic ===\n")

(display "(not #f) = ")
(display (not #f))
(newline)

(display "(not #t) = ")
(display (not #t))
(newline)

(display "(and #t #t #t) = ")
(display (and #t #t #t))
(newline)

(display "(and #t #f #t) = ")
(display (and #t #f #t))
(newline)

(display "(or #f #f #t) = ")
(display (or #f #f #t))
(newline)

(display "(or #f #f #f) = ")
(display (or #f #f #f))
(newline)

; ===========================================================================
; Test 4: List Operations
; ===========================================================================
(display "\n=== Test 4: List Operations ===\n")

(define test-list '(a b c d e))
(display "test-list = ")
(display test-list)
(newline)

(display "(car test-list) = ")
(display (car test-list))
(newline)

(display "(cdr test-list) = ")
(display (cdr test-list))
(newline)

(display "(cons 'x test-list) = ")
(display (cons 'x test-list))
(newline)

(display "(length test-list) = ")
(display (length test-list))
(newline)

(display "(reverse test-list) = ")
(display (reverse test-list))
(newline)

(display "(append '(1 2) '(3 4)) = ")
(display (append '(1 2) '(3 4)))
(newline)

(display "(null? '()) = ")
(display (null? '()))
(newline)

(display "(null? test-list) = ")
(display (null? test-list))
(newline)

(display "(pair? test-list) = ")
(display (pair? test-list))
(newline)

; ===========================================================================
; Test 5: Conditionals
; ===========================================================================
(display "\n=== Test 5: Conditionals ===\n")

(display "(if #t 'yes 'no) = ")
(display (if #t 'yes 'no))
(newline)

(display "(if #f 'yes 'no) = ")
(display (if #f 'yes 'no))
(newline)

(define (grade score)
  (cond ((>= score 90) 'A)
        ((>= score 80) 'B)
        ((>= score 70) 'C)
        ((>= score 60) 'D)
        (else 'F)))

(display "(grade 95) = ")
(display (grade 95))
(newline)

(display "(grade 75) = ")
(display (grade 75))
(newline)

(display "(grade 55) = ")
(display (grade 55))
(newline)

; ===========================================================================
; Test 6: Functions and Closures
; ===========================================================================
(display "\n=== Test 6: Functions and Closures ===\n")

(define (square x) (* x x))
(display "(square 7) = ")
(display (square 7))
(newline)

(define (cube x) (* x x x))
(display "(cube 3) = ")
(display (cube 3))
(newline)

; Closure test
(define (make-adder n)
  (lambda (x) (+ x n)))

(define add-10 (make-adder 10))
(define add-5 (make-adder 5))

(display "(add-10 25) = ")
(display (add-10 25))
(newline)

(display "(add-5 25) = ")
(display (add-5 25))
(newline)

; Counter closure
(define (make-counter)
  (let ((count 0))
    (lambda ()
      (set! count (+ count 1))
      count)))

(define counter (make-counter))
(display "counter() = ")
(display (counter))
(newline)

(display "counter() = ")
(display (counter))
(newline)

(display "counter() = ")
(display (counter))
(newline)

; ===========================================================================
; Test 7: Recursion
; ===========================================================================
(display "\n=== Test 7: Recursion ===\n")

(define (factorial n)
  (if (= n 0)
      1
      (* n (factorial (- n 1)))))

(display "(factorial 10) = ")
(display (factorial 10))
(newline)

(define (fibonacci n)
  (if (< n 2)
      n
      (+ (fibonacci (- n 1))
         (fibonacci (- n 2)))))

(display "(fibonacci 10) = ")
(display (fibonacci 10))
(newline)

; Tail-recursive factorial
(define (factorial-tr n)
  (let loop ((n n) (acc 1))
    (if (= n 0)
        acc
        (loop (- n 1) (* n acc)))))

(display "(factorial-tr 15) = ")
(display (factorial-tr 15))
(newline)

; ===========================================================================
; Test 8: Higher-Order Functions
; ===========================================================================
(display "\n=== Test 8: Higher-Order Functions ===\n")

(define (map f lst)
  (if (null? lst)
      '()
      (cons (f (car lst))
            (map f (cdr lst)))))

(display "(map square '(1 2 3 4 5)) = ")
(display (map square '(1 2 3 4 5)))
(newline)

(define (filter pred lst)
  (cond ((null? lst) '())
        ((pred (car lst))
         (cons (car lst) (filter pred (cdr lst))))
        (else (filter pred (cdr lst)))))

(define (even? n) (= (mod n 2) 0))
(define (odd? n) (not (even? n)))

(display "(filter even? '(1 2 3 4 5 6 7 8)) = ")
(display (filter even? '(1 2 3 4 5 6 7 8)))
(newline)

(define (reduce f init lst)
  (if (null? lst)
      init
      (reduce f (f init (car lst)) (cdr lst))))

(display "(reduce + 0 '(1 2 3 4 5)) = ")
(display (reduce + 0 '(1 2 3 4 5)))
(newline)

; ===========================================================================
; Test 9: Let Bindings
; ===========================================================================
(display "\n=== Test 9: Let Bindings ===\n")

(display "(let ((x 10) (y 20)) (+ x y)) = ")
(display (let ((x 10) (y 20)) (+ x y)))
(newline)

(display "(let* ((x 5) (y (* x 2))) (+ x y)) = ")
(display (let* ((x 5) (y (* x 2))) (+ x y)))
(newline)

; letrec for mutual recursion
(display "(letrec even?/odd? test) = ")
(display (letrec ((is-even? (lambda (n)
                              (if (= n 0) #t (is-odd? (- n 1)))))
                  (is-odd? (lambda (n)
                             (if (= n 0) #f (is-even? (- n 1))))))
           (is-even? 10)))
(newline)

; ===========================================================================
; Test 10: Strings
; ===========================================================================
(display "\n=== Test 10: Strings ===\n")

(display "(string-length \"hello\") = ")
(display (string-length "hello"))
(newline)

(display "(string-append \"hello\" \" \" \"world\") = ")
(display (string-append "hello" " " "world"))
(newline)

(display "(string-ref \"hello\" 1) = ")
(display (string-ref "hello" 1))
(newline)

(display "(number->string 42) = ")
(display (number->string 42))
(newline)

(display "(string->number \"123\") = ")
(display (string->number "123"))
(newline)

; ===========================================================================
; Test 11: Type Predicates
; ===========================================================================
(display "\n=== Test 11: Type Predicates ===\n")

(display "(number? 42) = ")
(display (number? 42))
(newline)

(display "(number? 'foo) = ")
(display (number? 'foo))
(newline)

(display "(symbol? 'foo) = ")
(display (symbol? 'foo))
(newline)

(display "(string? \"hello\") = ")
(display (string? "hello"))
(newline)

(display "(procedure? square) = ")
(display (procedure? square))
(newline)

(display "(boolean? #t) = ")
(display (boolean? #t))
(newline)

; ===========================================================================
; Test 12: Quote and Quasiquote
; ===========================================================================
(display "\n=== Test 12: Quote and Quasiquote ===\n")

(display "'(1 2 3) = ")
(display '(1 2 3))
(newline)

(display "'(a b c) = ")
(display '(a b c))
(newline)

(define x 10)
(display "`(a ,x b) = ")
(display `(a ,x b))
(newline)

(define lst '(1 2 3))
(display "`(a ,@lst b) = ")
(display `(a ,@lst b))
(newline)

; ===========================================================================
; Summary
; ===========================================================================
(display "\n=== All Tests Complete ===\n")
