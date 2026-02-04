; ============================================
; Example 8: Complete Program - Prime Numbers
; ============================================
; A complete program that finds prime numbers
; using the Sieve of Eratosthenes algorithm.
;
; Press F5 to run!
; ============================================

(display "=== Prime Number Generator ===")
(newline)
(newline)

; Generate a range of numbers
(define (range start end)
  (if (> start end)
      '()
      (cons start (range (+ start 1) end))))

; Check if n is divisible by d
(define (divisible? n d)
  (= (remainder n d) 0))

; Check if a number is prime
(define (prime? n)
  (define (check divisor)
    (cond ((> (* divisor divisor) n) #t)
          ((divisible? n divisor) #f)
          (else (check (+ divisor 1)))))
  (and (> n 1) (check 2)))

; Filter to get primes from a list
(define (filter pred lst)
  (cond ((null? lst) '())
        ((pred (car lst))
         (cons (car lst) (filter pred (cdr lst))))
        (else (filter pred (cdr lst)))))

; Get all primes up to n
(define (primes-up-to n)
  (filter prime? (range 2 n)))

; Count elements in a list
(define (length lst)
  (if (null? lst)
      0
      (+ 1 (length (cdr lst)))))

; Display primes up to various limits
(display "Primes up to 20: ")
(display (primes-up-to 20))
(newline)

(display "Primes up to 50: ")
(display (primes-up-to 50))
(newline)

(display "Primes up to 100: ")
(display (primes-up-to 100))
(newline)

(define primes-100 (primes-up-to 100))
(display "Count of primes up to 100: ")
(display (length primes-100))
(newline)

(newline)
(display "=== Program Complete ===")
(newline)
