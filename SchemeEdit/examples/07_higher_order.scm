; ============================================
; Example 7: Higher-Order Functions
; ============================================
; Functions that take or return other functions
; - lambda creates anonymous functions
; - map applies a function to each list element
; - filter selects elements matching a predicate
;
; Press F5 to run!
; ============================================

; Lambda (anonymous function)
(display "Using lambda to square: ")
(display ((lambda (x) (* x x)) 5))
(newline)

; Define map function
(define (map f lst)
  (if (null? lst)
      '()
      (cons (f (car lst)) (map f (cdr lst)))))

; Square all numbers
(define numbers '(1 2 3 4 5))
(display "Original: ")
(display numbers)
(newline)

(display "Squared:  ")
(display (map (lambda (x) (* x x)) numbers))
(newline)

(display "Doubled:  ")
(display (map (lambda (x) (* 2 x)) numbers))
(newline)

; Define filter function
(define (filter pred lst)
  (cond ((null? lst) '())
        ((pred (car lst))
         (cons (car lst) (filter pred (cdr lst))))
        (else (filter pred (cdr lst)))))

; Filter even numbers
(define (even? n)
  (= (remainder n 2) 0))

(display "Even numbers: ")
(display (filter even? numbers))
(newline)

; Filter numbers greater than 3
(display "Numbers > 3: ")
(display (filter (lambda (x) (> x 3)) numbers))
(newline)

; Define reduce (fold) function
(define (reduce f init lst)
  (if (null? lst)
      init
      (reduce f (f init (car lst)) (cdr lst))))

; Sum using reduce
(display "Sum (using reduce): ")
(display (reduce + 0 numbers))
(newline)

; Product using reduce
(display "Product (using reduce): ")
(display (reduce * 1 numbers))
(newline)

; Function composition
(define (compose f g)
  (lambda (x) (f (g x))))

(define add1 (lambda (x) (+ x 1)))
(define double (lambda (x) (* x 2)))
(define add1-then-double (compose double add1))

(display "add1-then-double(5) = ")
(display (add1-then-double 5))
(newline)
