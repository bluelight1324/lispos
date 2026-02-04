; Exception Breakpoint Test
(define (safe-divide a b)
  (display "Dividing ")
  (display a)
  (display " by ")
  (display b)
  (newline)
  (/ a b))

(display "Test 1: 10/2 = ")
(display (safe-divide 10 2))
(newline)

(display "Test 2: 20/4 = ")
(display (safe-divide 20 4))
(newline)

(display "Test 3: This will cause an error...")
(newline)
(display (safe-divide 10 0))
(newline)
