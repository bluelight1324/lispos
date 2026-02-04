; hello.scm - Hello World
;
; The simplest Lisp program

(display "Hello, World!")
(newline)

; Simple arithmetic
(display "2 + 3 = ")
(display (+ 2 3))
(newline)

; Define a greeting function
(define (greet name)
  (display "Hello, ")
  (display name)
  (display "!")
  (newline))

(greet "Lisp")
(greet "World")
