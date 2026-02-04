; ============================================
; Example 2: Variables and Definitions
; ============================================
; Use (define name value) to create variables
; Variables can hold numbers, strings, or any value
;
; Press F5 to run!
; ============================================

; Define some variables
(define pi 3.14159)
(define greeting "Hello")
(define my-number 42)

; Display them
(display "pi = ")
(display pi)
(newline)

(display "greeting = ")
(display greeting)
(newline)

(display "my-number = ")
(display my-number)
(newline)

; You can use variables in expressions
(define radius 5)
(define area (* pi (* radius radius)))

(display "Circle with radius ")
(display radius)
(display " has area ")
(display area)
(newline)
