; ============================================
; Example 6: Lists
; ============================================
; Lists are fundamental in Scheme:
; - Create with '(a b c) or (list a b c)
; - Access with (car lst) for first, (cdr lst) for rest
; - Check empty with (null? lst)
;
; Press F5 to run!
; ============================================

; Creating lists
(define numbers '(1 2 3 4 5))
(define names '("Alice" "Bob" "Charlie"))
(define mixed '(1 "hello" #t))

(display "numbers: ")
(display numbers)
(newline)

; Accessing elements
(display "First element (car): ")
(display (car numbers))
(newline)

(display "Rest of list (cdr): ")
(display (cdr numbers))
(newline)

(display "Second element (cadr): ")
(display (cadr numbers))
(newline)

; Building lists with cons
(define new-list (cons 0 numbers))
(display "After adding 0 to front: ")
(display new-list)
(newline)

; List length function
(define (length lst)
  (if (null? lst)
      0
      (+ 1 (length (cdr lst)))))

(display "Length of numbers: ")
(display (length numbers))
(newline)

; Sum all numbers in a list
(define (sum lst)
  (if (null? lst)
      0
      (+ (car lst) (sum (cdr lst)))))

(display "Sum of numbers: ")
(display (sum numbers))
(newline)

; Reverse a list
(define (reverse-helper lst acc)
  (if (null? lst)
      acc
      (reverse-helper (cdr lst) (cons (car lst) acc))))

(define (my-reverse lst)
  (reverse-helper lst '()))

(display "Reversed: ")
(display (my-reverse numbers))
(newline)
