; Debug test program
; Tests basic debugging functionality

(define x 10)
(define y 20)

(define (add a b)
  (+ a b))

(define (multiply a b)
  (* a b))

(define result1 (add x y))
(display "Result of add: ")
(display result1)
(newline)

(define result2 (multiply x y))
(display "Result of multiply: ")
(display result2)
(newline)

(define (factorial n)
  (if (<= n 1)
      1
      (* n (factorial (- n 1)))))

(display "Factorial of 5: ")
(display (factorial 5))
(newline)
