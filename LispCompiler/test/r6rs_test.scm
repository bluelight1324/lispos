; R6RS Feature Test Suite
; Tests vectors, bytevectors, hashtables, and numeric operations

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

(display "=== R6RS Vector Tests ===")
(newline)

; Basic vector creation
(define v1 (make-vector 5 0))
(test "make-vector creates vector" #t (vector? v1))
(test "vector-length" 5 (vector-length v1))
(test "vector-ref default fill" 0 (vector-ref v1 0))

; Vector with custom fill
(define v2 (make-vector 3 42))
(test "vector-ref with fill" 42 (vector-ref v2 1))

; vector constructor
(define v3 (vector 1 2 3 4 5))
(test "vector constructor length" 5 (vector-length v3))
(test "vector-ref first" 1 (vector-ref v3 0))
(test "vector-ref last" 5 (vector-ref v3 4))

; vector-set!
(vector-set! v1 2 99)
(test "vector-set!" 99 (vector-ref v1 2))

; vector->list and list->vector
(define lst (vector->list v3))
(test "vector->list" '(1 2 3 4 5) lst)
(define v4 (list->vector '(a b c)))
(test "list->vector length" 3 (vector-length v4))

(newline)
(display "=== R6RS Bytevector Tests ===")
(newline)

; Basic bytevector
(define bv1 (make-bytevector 4 0))
(test "make-bytevector" #t (bytevector? bv1))
(test "bytevector-length" 4 (bytevector-length bv1))
(test "bytevector-u8-ref default" 0 (bytevector-u8-ref bv1 0))

; With fill
(define bv2 (make-bytevector 3 255))
(test "bytevector with fill" 255 (bytevector-u8-ref bv2 0))

; bytevector-u8-set!
(bytevector-u8-set! bv1 1 128)
(test "bytevector-u8-set!" 128 (bytevector-u8-ref bv1 1))

(newline)
(display "=== R6RS Hashtable Tests ===")
(newline)

; eq hashtable
(define ht1 (make-eq-hashtable))
(test "make-eq-hashtable" #t (hashtable? ht1))
(test "empty hashtable size" 0 (hashtable-size ht1))

; hashtable-set! and hashtable-ref
(hashtable-set! ht1 'foo 42)
(test "hashtable-set!/ref" 42 (hashtable-ref ht1 'foo 0))
(test "hashtable-size after set" 1 (hashtable-size ht1))

; hashtable-contains?
(test "hashtable-contains? existing" #t (hashtable-contains? ht1 'foo))
(test "hashtable-contains? missing" #f (hashtable-contains? ht1 'bar))

; Default value for missing
(test "hashtable-ref default" 'default (hashtable-ref ht1 'missing 'default))

; Multiple entries
(hashtable-set! ht1 'bar 100)
(hashtable-set! ht1 'baz 200)
(test "hashtable-size multiple" 3 (hashtable-size ht1))

; hashtable-delete!
(hashtable-delete! ht1 'bar)
(test "hashtable-delete!" #f (hashtable-contains? ht1 'bar))
(test "hashtable-size after delete" 2 (hashtable-size ht1))

(newline)
(display "=== R6RS Numeric Tests ===")
(newline)

; Floor, ceiling, truncate, round
(test "floor 3.7" 3 (floor 3.7))
(test "floor -3.7" -4 (floor -3.7))
(test "ceiling 3.2" 4 (ceiling 3.2))
(test "ceiling -3.2" -3 (ceiling -3.2))
(test "truncate 3.7" 3 (truncate 3.7))
(test "truncate -3.7" -3 (truncate -3.7))
(test "round 3.5" 4 (round 3.5))
(test "round 2.5" 2 (round 2.5))

; Mathematical functions
(test "sqrt 4" 2 (sqrt 4))
(test "sqrt 9" 3 (sqrt 9))
(test "expt 2 10" 1024 (expt 2 10))

; Division operations
(test "quotient 17 5" 3 (quotient 17 5))
(test "remainder 17 5" 2 (remainder 17 5))
(test "modulo 17 5" 2 (modulo 17 5))
(test "modulo -17 5" 3 (modulo -17 5))

; Numeric predicates
(test "integer? 5" #t (integer? 5))
(test "integer? 5.0" #t (integer? 5.0))
(test "integer? 5.5" #f (integer? 5.5))
(test "real? 3.14" #t (real? 3.14))
(test "zero? 0" #t (zero? 0))
(test "zero? 1" #f (zero? 1))
(test "positive? 5" #t (positive? 5))
(test "positive? -5" #f (positive? -5))
(test "negative? -5" #t (negative? -5))
(test "odd? 7" #t (odd? 7))
(test "even? 8" #t (even? 8))

; min/max
(test "min 3 1 4" 1 (min 3 1 4))
(test "max 3 1 4" 4 (max 3 1 4))

(newline)
(display "=== R6RS List Tests ===")
(newline)

(test "list? proper" #t (list? '(1 2 3)))
(test "list? empty" #t (list? '()))
(test "list? improper" #f (list? (cons 1 2)))
(test "list-ref" 'c (list-ref '(a b c d) 2))
(test "list-tail" '(c d) (list-tail '(a b c d) 2))

; Association lists
(define alist '((a . 1) (b . 2) (c . 3)))
(test "assq found" '(b . 2) (assq 'b alist))
(test "assq not found" #f (assq 'd alist))

; Membership
(test "memq found" '(b c d) (memq 'b '(a b c d)))
(test "memq not found" #f (memq 'e '(a b c d)))
(test "member" '(2 3) (member 2 '(1 2 3)))

(newline)
(display "=== R6RS Character Tests ===")
(newline)

(test "char? #\\a" #t (char? #\a))
(test "char? 1" #f (char? 1))
(test "char=?" #t (char=? #\a #\a))
(test "char<?" #t (char<? #\a #\b))
(test "char->integer" 65 (char->integer #\A))
(test "integer->char" #\A (integer->char 65))

(newline)
(display "=== All R6RS Tests Complete ===")
(newline)
