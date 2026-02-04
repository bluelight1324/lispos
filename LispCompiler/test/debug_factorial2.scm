; Factorial debugging test - multi-line
(define (factorial n)
  (if (<= n 1)
      1
      (* n (factorial (- n 1)))))

(display "Result: ")
(display (factorial 5))
(newline)
