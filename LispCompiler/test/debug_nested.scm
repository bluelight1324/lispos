; Nested function calls for debugging

(define (inner x)
  (display "Inner called with: ")
  (display x)
  (newline)
  (* x 2))

(define (middle x)
  (display "Middle called with: ")
  (display x)
  (newline)
  (inner (+ x 1)))

(define (outer x)
  (display "Outer called with: ")
  (display x)
  (newline)
  (middle (* x 2)))

(display "Final result: ")
(display (outer 5))
(newline)
