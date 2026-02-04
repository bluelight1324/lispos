; Debugger Feature Test
; Test conditional breakpoints, watch expressions, and exception handling

(define counter 0)

(define (increment)
  (set! counter (+ counter 1))
  counter)

(define (factorial n)
  (if (<= n 1)
      1
      (* n (factorial (- n 1)))))

(define (loop-test n)
  (if (> n 0)
      (begin
        (display "n = ")
        (display n)
        (newline)
        (increment)
        (loop-test (- n 1)))))

; Main test
(display "Starting debugger test...")
(newline)

(loop-test 5)

(display "Counter: ")
(display counter)
(newline)

(display "Factorial 5: ")
(display (factorial 5))
(newline)

(display "Test complete!")
(newline)
