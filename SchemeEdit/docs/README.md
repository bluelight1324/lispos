# SchemeEdit - A Qt-based Text Editor with Scheme Integration

SchemeEdit is a modern text editor built with Qt 6 that provides integrated support for editing and running Scheme/Lisp code using the LispOS Scheme compiler.

## Features

- **Modern Text Editing**
  - Line numbers
  - Current line highlighting
  - Dark theme (VS Code-inspired)
  - Standard editing operations (cut, copy, paste, undo, redo)

- **Scheme Syntax Highlighting**
  - Keywords (define, lambda, if, cond, let, etc.)
  - Built-in functions (car, cdr, cons, map, filter, etc.)
  - Strings, numbers, and booleans
  - Comments
  - Quote and quasiquote operators

- **Scheme Integration**
  - Run entire file with F5
  - Run selected code with Ctrl+Enter
  - Integrated output pane
  - Uses the LispOS R7RS Scheme interpreter

## Requirements

- Qt 6.x (tested with Qt 6.10.2)
- MinGW or MSVC compiler
- LispOS Scheme compiler (lisp.exe)

## Building

### Using Qt Tools (Recommended)

```bash
cd SchemeEdit

# Configure with CMake
C:/Qt/Tools/CMake_64/bin/cmake.exe -B build -S . \
  -G "MinGW Makefiles" \
  -DCMAKE_PREFIX_PATH="C:/Qt/6.10.2/mingw_64" \
  -DCMAKE_MAKE_PROGRAM="C:/Qt/Tools/mingw1310_64/bin/mingw32-make.exe" \
  -DCMAKE_CXX_COMPILER="C:/Qt/Tools/mingw1310_64/bin/g++.exe" \
  -DCMAKE_BUILD_TYPE=Release

# Build
C:/Qt/Tools/mingw1310_64/bin/mingw32-make.exe -C build -j4

# Deploy Qt dependencies
C:/Qt/6.10.2/mingw_64/bin/windeployqt6.exe build/bin/SchemeEdit.exe
```

### Using Visual Studio

```bash
cd SchemeEdit
cmake -B build -S . -DCMAKE_PREFIX_PATH="C:/Qt/6.10.2/msvc2022_64"
cmake --build build --config Release
```

## Usage

### Running the Editor

```bash
SchemeEdit.exe
```

### Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| Ctrl+N | New file |
| Ctrl+O | Open file |
| Ctrl+S | Save file |
| Ctrl+Shift+S | Save file as |
| Ctrl+Z | Undo |
| Ctrl+Y | Redo |
| Ctrl+X | Cut |
| Ctrl+C | Copy |
| Ctrl+V | Paste |
| Ctrl+A | Select all |
| **F5** | **Run Scheme code** |
| **Ctrl+Enter** | **Run selected code** |
| Ctrl+L | Clear output |

### Running Scheme Code

1. Type or open a Scheme file (.scm)
2. Press **F5** to run the entire file
3. Or select code and press **Ctrl+Enter** to run just the selection

The output appears in the Output pane at the bottom of the window.

### Example Scheme Code

```scheme
; Hello World
(display "Hello from SchemeEdit!")
(newline)

; Define a function
(define (factorial n)
  (if (= n 0)
      1
      (* n (factorial (- n 1)))))

; Test it
(display "5! = ")
(display (factorial 5))
(newline)
```

## Project Structure

```
SchemeEdit/
├── CMakeLists.txt          # Build configuration
├── src/
│   ├── main.cpp            # Application entry point
│   ├── mainwindow.h/cpp    # Main window and menus
│   ├── editor.h/cpp        # Text editor with line numbers
│   ├── schemerunner.h/cpp  # Scheme interpreter integration
│   └── schemehighlighter.h/cpp  # Syntax highlighting
├── docs/
│   └── README.md           # This file
└── build/
    └── bin/
        └── SchemeEdit.exe  # Built executable
```

## Configuration

### Scheme Interpreter Path

SchemeEdit automatically searches for the Scheme interpreter in these locations:

1. Same directory as SchemeEdit.exe
2. `../lisp.exe` (relative to executable)
3. `../../LispCompiler/build/Release/lisp.exe`
4. `I:/lispos/LispCompiler/build/Release/lisp.exe`
5. System PATH

## Architecture

### Components

1. **MainWindow** - Main application window with menus, toolbars, and dock widgets
2. **Editor** - QPlainTextEdit subclass with line numbers and current line highlighting
3. **SchemeHighlighter** - QSyntaxHighlighter for Scheme code
4. **SchemeRunner** - QProcess-based runner that executes Scheme code via lisp.exe

### Execution Flow

1. User writes/loads Scheme code in the editor
2. User presses F5 or Ctrl+Enter
3. SchemeRunner writes code to a temporary .scm file
4. SchemeRunner launches lisp.exe with the temp file
5. Output is captured and displayed in the output pane
6. Temporary file is cleaned up after execution

## License

Part of the LispOS project.

## See Also

- [LispCompiler Documentation](../../LispCompiler/docs/README.md)
- [R7RS Implementation](../../LispCompiler/docs/R7RS_IMPLEMENTATION.md)
- [Test Results](../../LispCompiler/docs/TEST_RESULTS.md)
