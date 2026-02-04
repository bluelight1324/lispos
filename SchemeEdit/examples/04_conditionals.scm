; ============================================
; Example 4: Conditionals (if, cond, and, or)
; ============================================
; Scheme provides several ways to make decisions:
; - (if condition then-expr else-expr)
; - (cond (test1 expr1) (test2 expr2) (else default))
; - (and ...) / (or ...)
;
; Press F5 to run!
; ============================================

; Basic if-then-else
(define (max a b)
  (if (> a b) a b))

(display "max(10, 7) = ")
(display (max 10 7))
(newline)

; Using cond for multiple conditions
(define (grade score)
  (cond ((>= score 90) "A")
        ((>= score 80) "B")
        ((>= score 70) "C")
        ((>= score 60) "D")
        (else "F")))

(display "Score 95 gets grade: ")
(display (grade 95))
(newline)

(display "Score 75 gets grade: ")
(display (grade 75))
(newline)

(display "Score 45 gets grade: ")
(display (grade 45))
(newline)

; Using and/or
(define (in-range? x low high)
  (and (>= x low) (<= x high)))

(display "Is 5 in range [1, 10]? ")
(display (in-range? 5 1 10))
(newline)

(display "Is 15 in range [1, 10]? ")
(display (in-range? 15 1 10))
(newline)
