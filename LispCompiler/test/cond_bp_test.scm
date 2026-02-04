; Conditional Breakpoint Test
(define (loop-count n)
  (if (> n 0)
      (begin
        (display n)
        (display " ")
        (loop-count (- n 1)))
      (newline)))

(loop-count 10)
(display "Done!")
(newline)
