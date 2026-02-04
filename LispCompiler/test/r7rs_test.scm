; R7RS Feature Test Suite
; Tests when/unless, case-lambda, do, case, higher-order functions, etc.

(define (test name expected actual)
  (display name)
  (display ": ")
  (if (equal? expected actual)
      (display "PASS")
      (begin
        (display "FAIL - expected ")
        (display expected)
        (display ", got ")
        (display actual)))
  (newline))

(display "=== R7RS Special Forms ===")
(newline)

; when
(test "when true" 42 (when #t 1 2 42))
(test "when false" () (when #f 'never))

; unless
(test "unless false" 'yes (unless #f 'yes))
(test "unless true" () (unless #t 'never))

; case-lambda
(define add-numbers
  (case-lambda
    (() 0)
    ((x) x)
    ((x y) (+ x y))
    ((x y z) (+ x y z))))

(test "case-lambda 0 args" 0 (add-numbers))
(test "case-lambda 1 arg" 5 (add-numbers 5))
(test "case-lambda 2 args" 7 (add-numbers 3 4))
(test "case-lambda 3 args" 15 (add-numbers 5 6 4))

; do - iteration
(test "do loop sum" 55
  (do ((i 1 (+ i 1))
       (sum 0 (+ sum i)))
      ((> i 10) sum)))

(test "do loop countdown" '(1 2 3)
  (let ((result '()))
    (do ((i 3 (- i 1)))
        ((= i 0) result)
      (set! result (cons i result)))))

; case
(test "case match first" 'one
  (case 1
    ((1) 'one)
    ((2) 'two)
    (else 'other)))

(test "case match list" 'vowel
  (case 'a
    ((a e i o u) 'vowel)
    (else 'consonant)))

(test "case else" 'unknown
  (case 99
    ((1 2 3) 'small)
    (else 'unknown)))

(newline)
(display "=== R7RS Multiple Values ===")
(newline)

(test "values single" 42 (values 42))
(test "call-with-values" 7
  (call-with-values
    (lambda () (values 3 4))
    (lambda (a b) (+ a b))))

(newline)
(display "=== R7RS List Operations ===")
(newline)

(test "make-list length" '(() () () () ())
  (make-list 5))
(test "make-list with fill" '(x x x)
  (make-list 3 'x))

(define lst1 '(1 2 3))
(define lst2 (list-copy lst1))
(test "list-copy creates copy" '(1 2 3) lst2)
(test "list-copy is different object" #f (eq? lst1 lst2))

(define mutable-list (list 'a 'b 'c))
(list-set! mutable-list 1 'x)
(test "list-set!" '(a x c) mutable-list)

(newline)
(display "=== R7RS Vector Operations ===")
(newline)

(define v (vector 1 2 3 4 5))
(define vc (vector-copy v))
(test "vector-copy length" 5 (vector-length vc))
(test "vector-copy content" 1 (vector-ref vc 0))

(define vc2 (vector-copy v 2))
(test "vector-copy with start" 3 (vector-length vc2))
(test "vector-copy start content" 3 (vector-ref vc2 0))

(define vc3 (vector-copy v 1 4))
(test "vector-copy with start/end" 3 (vector-length vc3))
(test "vector-copy range content" 2 (vector-ref vc3 0))

(define v2 (vector 0 0 0))
(vector-fill! v2 7)
(test "vector-fill! first" 7 (vector-ref v2 0))
(test "vector-fill! last" 7 (vector-ref v2 2))

(define va (vector-append (vector 1 2) (vector 3 4)))
(test "vector-append length" 4 (vector-length va))
(test "vector-append first" 1 (vector-ref va 0))
(test "vector-append last" 4 (vector-ref va 3))

(newline)
(display "=== R7RS String Operations ===")
(newline)

(test "string-copy" "hello" (string-copy "hello"))
(test "substring" "ell" (substring "hello" 1 4))
(test "string=? true" #t (string=? "abc" "abc"))
(test "string=? false" #f (string=? "abc" "xyz"))
(test "string<? true" #t (string<? "abc" "abd"))
(test "string<? false" #f (string<? "xyz" "abc"))

(newline)
(display "=== R7RS Numeric Operations ===")
(newline)

(test "square" 25 (square 5))
(test "square negative" 16 (square -4))
(test "exact" 3 (exact 3.7))
(test "inexact" 5 (inexact 5))
(test "finite? normal" #t (finite? 42))
(test "gcd" 6 (gcd 12 18))
(test "gcd multiple" 2 (gcd 12 18 8))
(test "lcm" 36 (lcm 12 18))
(test "lcm multiple" 12 (lcm 3 4 6))

(newline)
(display "=== R7RS Equivalence ===")
(newline)

(test "boolean=? all true" #t (boolean=? #t #t #t))
(test "boolean=? all false" #t (boolean=? #f #f))
(test "boolean=? mixed" #f (boolean=? #t #f))
(test "symbol=? same" #t (symbol=? 'foo 'foo 'foo))
(test "symbol=? different" #f (symbol=? 'foo 'bar))

(newline)
(display "=== R7RS Higher-Order Functions ===")
(newline)

(test "map single list" '(2 4 6)
  (map (lambda (x) (* x 2)) '(1 2 3)))

(test "map multiple lists" '(5 7 9)
  (map + '(1 2 3) '(4 5 6)))

(define sum 0)
(for-each (lambda (x) (set! sum (+ sum x))) '(1 2 3 4 5))
(test "for-each" 15 sum)

(test "filter" '(2 4 6)
  (filter even? '(1 2 3 4 5 6)))

(test "fold" 15
  (fold + 0 '(1 2 3 4 5)))

(test "fold-right" '(1 2 3 4 5)
  (fold-right cons '() '(1 2 3 4 5)))

(newline)
(display "=== All R7RS Tests Complete ===")
(newline)
