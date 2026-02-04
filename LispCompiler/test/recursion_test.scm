;;; Recursion Depth Protection Test
;;; Tests Essential #1: Crash-free operation

(display "Testing recursion depth protection...")
(newline)

;; This should trigger the recursion limit, not crash
(define (infinite-loop)
  (infinite-loop))

(display "Calling infinite recursion (should error gracefully)...")
(newline)
(infinite-loop)

;; This line should not be reached
(display "ERROR: This should not print - recursion limit failed!")
(newline)
