; list_ops.scm - List operation tests
;
; Tests:
; - car, cdr, cons
; - List construction and manipulation
; - Higher-order functions

; Basic list operations
(display "Testing basic list operations:\n")

(define lst '(1 2 3 4 5))
(display "lst = ")
(display lst)
(newline)

(display "car: ")
(display (car lst))
(newline)

(display "cdr: ")
(display (cdr lst))
(newline)

(display "cons 0 lst: ")
(display (cons 0 lst))
(newline)

; Map function
(define (map f lst)
  (if (null? lst)
      '()
      (cons (f (car lst))
            (map f (cdr lst)))))

(display "\nTesting map:\n")
(display "map (lambda (x) (* x x)) '(1 2 3 4 5):\n")
(display (map (lambda (x) (* x x)) '(1 2 3 4 5)))
(newline)

; Filter function
(define (filter pred lst)
  (cond ((null? lst) '())
        ((pred (car lst))
         (cons (car lst) (filter pred (cdr lst))))
        (else (filter pred (cdr lst)))))

(display "\nTesting filter:\n")
(display "filter even numbers from (1 2 3 4 5 6 7 8):\n")
(display (filter (lambda (x) (= (mod x 2) 0)) '(1 2 3 4 5 6 7 8)))
(newline)

; Reduce/fold function
(define (reduce f init lst)
  (if (null? lst)
      init
      (reduce f (f init (car lst)) (cdr lst))))

(display "\nTesting reduce:\n")
(display "sum of (1 2 3 4 5): ")
(display (reduce + 0 '(1 2 3 4 5)))
(newline)

(display "product of (1 2 3 4 5): ")
(display (reduce * 1 '(1 2 3 4 5)))
(newline)
