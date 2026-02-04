; Exception Breakpoint Test v2 - Undefined variable error
(define x 10)
(define y 20)

(display "x = ")
(display x)
(newline)

(display "y = ")
(display y)
(newline)

(display "This will cause an error - referencing undefined z...")
(newline)

(display "z = ")
(display z)  ; This should cause an error
(newline)

(display "This should not print")
(newline)
