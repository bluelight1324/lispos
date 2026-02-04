# UX Essentials for Real-World Use

This document identifies the **absolute minimum UX features** required for real-world use of SchemeEdit and the LispCompiler. These are the non-negotiable features without which users cannot effectively develop Scheme programs.

## Executive Summary

| Essential | Why It's Critical | Without It |
|-----------|-------------------|------------|
| **Bracket Matching** | Lisp is parentheses-heavy | Code is unreadable |
| **Find/Replace** | Basic editing capability | Users leave for another editor |
| **Error Location** | Know where bugs are | Debugging is impossible |
| **Keyboard Shortcuts** | Basic efficiency | Unusably slow workflow |
| **One-Click Run** | Test code quickly | Frustrating iteration cycle |

---

## The 5 Absolute Essential UX Features

### 1. Bracket Matching and Highlighting

**Why Essential**: Lisp/Scheme uses parentheses for everything. Without bracket matching, even simple code becomes impossible to read and write correctly.

**Minimum Implementation**:
```
When cursor is on ( or ):
- Highlight the matching bracket
- If no match, show error indicator (red)
```

**User Impact Without It**:
- Constant syntax errors from mismatched parens
- Hours wasted counting brackets manually
- Users will abandon the editor immediately

**Implementation Effort**: LOW (1-2 days)

**Acceptance Criteria**:
- [ ] Cursor on `(` highlights matching `)`
- [ ] Cursor on `)` highlights matching `(`
- [ ] Unmatched brackets shown in red
- [ ] Works for nested brackets at any depth

---

### 2. Find and Replace

**Why Essential**: Every text editor has this. Users expect it. You cannot work with code without the ability to search.

**Minimum Implementation**:
```
Ctrl+F: Open find bar
- Text input for search term
- "Next" and "Previous" buttons
- Highlight all matches
- Match count display

Ctrl+H: Find and Replace
- Search term input
- Replace term input
- "Replace" and "Replace All" buttons
```

**User Impact Without It**:
- Cannot locate code in large files
- Cannot rename variables
- Cannot fix repeated mistakes
- Users will use external editor instead

**Implementation Effort**: MEDIUM (3-5 days)

**Acceptance Criteria**:
- [ ] Ctrl+F opens find dialog
- [ ] All matches highlighted in editor
- [ ] F3 goes to next match
- [ ] Shift+F3 goes to previous match
- [ ] Ctrl+H opens find/replace dialog
- [ ] Replace and Replace All work correctly

---

### 3. Error Messages with Location

**Why Essential**: When code has an error, users must know WHERE the error is. Line number alone is insufficient - column position and visual highlighting are needed.

**Minimum Implementation**:
```
Error display:
1. Show error message
2. Show file:line:column
3. Highlight the error location in editor
4. Jump to error on click

Example:
ERROR: Unbound variable 'foo'
  at main.scm:15:8
  [Click to jump to error]

In editor:
  (display foo)
           ^^^
           Red underline
```

**User Impact Without It**:
- Hunting for errors in large files
- No understanding of what went wrong
- Extremely frustrating debugging experience

**Implementation Effort**: MEDIUM (3-5 days)

**Acceptance Criteria**:
- [ ] Error shows file, line, and column
- [ ] Error location highlighted in editor
- [ ] Click on error navigates to location
- [ ] Clear, readable error message text

---

### 4. Essential Keyboard Shortcuts

**Why Essential**: Mouse-only editing is slow and frustrating. Basic keyboard shortcuts are expected in any code editor.

**Minimum Implementation**:

| Action | Shortcut | Priority |
|--------|----------|----------|
| Save | Ctrl+S | CRITICAL |
| Undo | Ctrl+Z | CRITICAL |
| Redo | Ctrl+Y | CRITICAL |
| Cut | Ctrl+X | CRITICAL |
| Copy | Ctrl+C | CRITICAL |
| Paste | Ctrl+V | CRITICAL |
| Find | Ctrl+F | CRITICAL |
| Replace | Ctrl+H | HIGH |
| Select All | Ctrl+A | HIGH |
| Run | F5 | HIGH |
| Go to Line | Ctrl+G | MEDIUM |

**User Impact Without It**:
- Every action requires mouse
- Workflow is 5x slower
- Users familiar with other editors cannot work

**Implementation Effort**: LOW (1-2 days)

**Acceptance Criteria**:
- [ ] All CRITICAL shortcuts work
- [ ] All HIGH priority shortcuts work
- [ ] Shortcuts don't conflict with each other
- [ ] Shortcuts match common editor conventions

---

### 5. One-Click Run with Output

**Why Essential**: The edit-run-debug cycle must be fast. Users need to see results immediately.

**Minimum Implementation**:
```
┌─────────────────────────────────────┐
│ [▶ Run] [🐛 Debug] [⬛ Stop]        │
├─────────────────────────────────────┤
│ Editor                              │
│ (display "Hello")                   │
│ (display (+ 1 2))                   │
├─────────────────────────────────────┤
│ Output                              │
│ Hello                               │
│ 3                                   │
│                                     │
│ [Program exited successfully]       │
└─────────────────────────────────────┘
```

**Requirements**:
- Single button/shortcut to run (F5)
- Output displayed in dedicated panel
- Clear indication of success/failure
- Stop button for long-running programs

**User Impact Without It**:
- Must switch to terminal to run
- Context switching breaks flow
- No integrated error feedback

**Implementation Effort**: LOW-MEDIUM (2-3 days)

**Acceptance Criteria**:
- [ ] F5 runs current file
- [ ] Output visible in editor
- [ ] Errors displayed with location
- [ ] Can stop running program

---

## What Can Wait (Not Essential)

These features from ADDITIONAL_UX_FEATURES.md are nice-to-have but NOT required for real-world use:

| Feature | Why It Can Wait |
|---------|-----------------|
| Auto-completion | Users can type; it's slower but works |
| Code folding | Scroll works for navigation |
| Multiple tabs | Close and open files as needed |
| REPL integration | Run full file instead |
| Themes | Default colors are usable |
| Rainbow brackets | Single-color matching is sufficient |
| Git integration | Use external git client |
| Refactoring | Manual rename works |
| Documentation hover | Reference external docs |
| Project management | Work with single files |
| Code snippets | Type manually |
| Profiler | Optimize later |

---

## Implementation Priority

### Week 1: Must Ship
| Day | Feature | Effort |
|-----|---------|--------|
| 1-2 | Bracket matching | Low |
| 2-3 | Essential keyboard shortcuts | Low |
| 3-5 | Error location highlighting | Medium |

### Week 2: Must Ship
| Day | Feature | Effort |
|-----|---------|--------|
| 1-3 | Find functionality | Medium |
| 3-5 | Find and Replace | Medium |

### Already Done (Verify Works)
- [ ] One-click Run (F5)
- [ ] Output panel
- [ ] Stop button
- [ ] Basic save/load

---

## Minimum Viable Editor Checklist

Before claiming "real-world ready":

### Critical (Must Have)
- [ ] Bracket matching works
- [ ] Ctrl+F find works
- [ ] Ctrl+H replace works
- [ ] F5 runs code
- [ ] Errors show line numbers
- [ ] Errors highlight in editor
- [ ] Ctrl+S saves
- [ ] Ctrl+Z/Y undo/redo

### Important (Should Have)
- [ ] F3/Shift+F3 find next/prev
- [ ] Ctrl+G go to line
- [ ] Stop button works
- [ ] Clear output before run

### Optional (Nice to Have)
- [ ] Match case option in find
- [ ] Whole word option in find
- [ ] Jump to matching bracket shortcut

---

## User Workflow Validation

A user should be able to perform this workflow smoothly:

```
1. Open file (Ctrl+O or File menu)
2. Edit code (type, with bracket matching visible)
3. Find something (Ctrl+F, type, F3 to navigate)
4. Replace something (Ctrl+H, replace, done)
5. Save (Ctrl+S)
6. Run (F5)
7. See output (in output panel)
8. See error location (highlighted in editor)
9. Fix error (navigate to it, edit)
10. Run again (F5)
11. Repeat until done
```

**Test this workflow before release.**

---

## Comparison: Essential vs Full Feature Set

| Category | Essential (Ship Now) | Full (Ship Later) |
|----------|---------------------|-------------------|
| Editing | Bracket match | + Rainbow, folding, snippets |
| Search | Find/Replace | + Regex, find in files |
| Navigation | Go to line | + Go to definition, bookmarks |
| Running | One-click run | + REPL, test runner |
| Errors | Location + highlight | + Suggestions, auto-fix |
| Appearance | Fixed theme | + Multiple themes, fonts |
| Files | Single file | + Tabs, projects |

---

## Conclusion

**The 5 absolute essentials for real-world UX:**

1. **Bracket matching** - Can't write Lisp without it
2. **Find/Replace** - Can't edit code without it
3. **Error location** - Can't debug without it
4. **Keyboard shortcuts** - Can't work efficiently without them
5. **One-click run** - Can't iterate quickly without it

**Total implementation effort**: ~2 weeks

**Without these features**: Users will immediately switch to VS Code, Emacs, or another editor. The editor will be unusable for any real work.

**With these features**: Users can effectively write, edit, run, and debug Scheme programs. Additional features improve the experience but are not blockers.

---

## Success Metrics

| Metric | Target |
|--------|--------|
| Time to find code | < 5 seconds with Ctrl+F |
| Time to run code | < 1 second after F5 |
| Bracket errors | Visible immediately |
| Error navigation | 1 click to location |
| Basic edit workflow | No mouse required |

When these metrics are met, the editor is ready for real-world use.
