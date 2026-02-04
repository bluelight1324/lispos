# SchemeEdit Rider-Like Features

This document describes the JetBrains Rider-inspired features implemented in SchemeEdit.

## Visual Theme

### Dark Color Scheme
SchemeEdit uses a Rider-inspired dark theme:

| Element | Color | Description |
|---------|-------|-------------|
| Background | `#2b2b2b` | Editor background |
| Text | `#a9b7c6` | Default text color |
| Selection | `#214283` | Selection highlight |
| Current Line | `#323232` | Subtle current line highlight |
| Gutter | `#313335` | Line number area background |

### Font
- Primary: **JetBrains Mono** (11pt)
- Fallback 1: Consolas
- Fallback 2: Courier New
- Tab width: 2 spaces (Lisp convention)

## Bracket Features

### Rainbow Parentheses
Brackets are colorized based on nesting depth using a 6-color palette:

1. **Gold** `#ffd700` - Depth 0, 6, 12...
2. **Orchid** `#da70d6` - Depth 1, 7, 13...
3. **Sky Blue** `#87ceeb` - Depth 2, 8, 14...
4. **Pale Green** `#98fb98` - Depth 3, 9, 15...
5. **Light Salmon** `#ffa07a` - Depth 4, 10, 16...
6. **Powder Blue** `#b0e0e6` - Depth 5, 11, 17...

### Bracket Matching
When the cursor is near a bracket:
- Both the bracket and its match are highlighted
- Highlighted with bold text and subtle background (`#3c3c3c`)
- Uses the rainbow color for the bracket's nesting depth

### Auto-Close Brackets
Typing an opening bracket automatically inserts the closing bracket:
- `(` → `()` with cursor between
- `[` → `[]` with cursor between
- `"` → `""` with cursor between

### Skip-Over Closing Brackets
When typing a closing bracket that already exists at the cursor position, the cursor skips over it instead of inserting a duplicate.

## Code Editing

### Auto-Indent
Pressing Enter automatically indents the new line:
- Preserves the indentation level of the current line
- Adds 2 spaces for each unclosed parenthesis on the current line

Example:
```scheme
(define (factorial n)
  (if (<= n 1)    ; Press Enter here
      |           ; Cursor indents 6 spaces (3 open parens × 2)
```

### Cursor
- 2-pixel wide cursor for better visibility

## Line Number Area

### Gutter Styling
- Background: `#313335` (Rider gutter color)
- Separator line on right edge
- Right-aligned numbers with padding

### Current Line Indicator
- Current line number in **bold white**
- Yellow indicator bar on the left edge
- Other line numbers in gray

## Implementation Details

### Source Files
- [editor.h](../src/editor.h) - Editor class declaration
- [editor.cpp](../src/editor.cpp) - Editor implementation with all features
- [schemehighlighter.cpp](../src/schemehighlighter.cpp) - Syntax highlighting

### Key Methods
| Method | Purpose |
|--------|---------|
| `setupEditor()` | Initializes theme, font, and colors |
| `keyPressEvent()` | Handles auto-close and skip-over |
| `autoCloseBracket()` | Inserts matching bracket |
| `autoIndent()` | Calculates and applies indentation |
| `highlightMatchingBrackets()` | Highlights bracket pairs |
| `findMatchingBracket()` | Finds matching bracket position |
| `lineNumberAreaPaintEvent()` | Draws the line number gutter |

## Screenshots

The editor provides a modern, dark IDE experience similar to JetBrains Rider:
- Clean, minimal interface
- Syntax highlighting optimized for Scheme/Lisp
- Visual bracket matching aids code navigation
- Rainbow parentheses help track nesting levels

## Future Improvements (Phase 2+)

Planned features for future releases:
- Code folding for define blocks
- Minimap/document overview
- Error highlighting (red squiggles)
- Auto-completion for Scheme keywords
- Parameter hints for procedures
- Structure view/outline panel
