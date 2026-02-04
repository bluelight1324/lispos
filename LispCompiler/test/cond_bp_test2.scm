; Conditional Breakpoint Test v2
(define (factorial n)
  (display "Computing factorial of ")
  (display n)
  (newline)
  (if (<= n 1)
      1
      (* n (factorial (- n 1)))))

(display "Result: ")
(display (factorial 7))
(newline)
