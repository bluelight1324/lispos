# VS Code Integration for LispCompiler

This document describes how to use the LispCompiler Scheme development environment within Visual Studio Code.

## Overview

The LispCompiler project provides two levels of VS Code integration:

1. **Workspace Configuration** (`.vscode/` folder) - Ready-to-use tasks and launch configurations
2. **Custom Extension** (`vscode-scheme/` folder) - Full-featured extension with syntax highlighting and debugging

## Quick Start

### Using Workspace Configuration

1. Open the `LispCompiler` folder in VS Code
2. Install recommended extensions when prompted
3. Build the compiler: `Ctrl+Shift+B` (Build Release)
4. Open any `.scm` file
5. Run: Use `F5` or select a launch configuration

### Available Tasks

Access via `Ctrl+Shift+P` → "Tasks: Run Task"

| Task | Description |
|------|-------------|
| **CMake Configure** | Initialize CMake build system |
| **Build Release** | Build compiler in Release mode (default) |
| **Build Debug** | Build compiler with debug symbols |
| **Run Tests** | Run the test suite |
| **Run Scheme File** | Interpret current `.scm` file |
| **Compile Scheme to ASM** | Generate assembly output |
| **Full Build** | Compile → Assemble → Link chain |
| **Debug Scheme File** | Run with interactive debugger |

### Launch Configurations

Access via `F5` or Run panel:

| Configuration | Description |
|---------------|-------------|
| **Run Scheme File** | Run current file with Release build |
| **Run Scheme File (Debug Build)** | Run with debug-enabled compiler |
| **Debug Scheme File** | Interactive text-based debugging |
| **Compile to Assembly** | Generate `.asm` file |
| **Build and Run (Full)** | Complete build pipeline |
| **Debug LispCompiler (C++)** | Debug the compiler itself |

## Installing the Extension

### Option 1: Development Mode

1. Navigate to extension folder:
   ```bash
   cd LispCompiler/vscode-scheme
   ```

2. Install dependencies:
   ```bash
   npm install
   ```

3. Compile TypeScript:
   ```bash
   npm run compile
   ```

4. Press `F5` in VS Code to launch Extension Development Host

### Option 2: Package and Install

1. Install vsce:
   ```bash
   npm install -g @vscode/vsce
   ```

2. Package extension:
   ```bash
   cd LispCompiler/vscode-scheme
   vsce package
   ```

3. Install the generated `.vsix` file:
   - `Ctrl+Shift+P` → "Extensions: Install from VSIX..."
   - Select `scheme-lisp-compiler-1.0.0.vsix`

## Extension Features

### Syntax Highlighting

The extension provides comprehensive Scheme syntax highlighting:

- **Keywords**: `define`, `lambda`, `if`, `cond`, `let`, `let*`, `letrec`, etc.
- **Built-in functions**: `car`, `cdr`, `cons`, `list`, `map`, `display`, etc.
- **Operators**: `+`, `-`, `*`, `/`, `<`, `>`, `=`, etc.
- **Numbers**: integers, floats, rationals, complex, hex/binary/octal
- **Strings**: with escape sequence support
- **Characters**: `#\a`, `#\space`, `#\newline`, etc.
- **Booleans**: `#t`, `#f`, `#true`, `#false`
- **Comments**: `;` line comments, `#| |#` block comments

### Commands

Access via `Ctrl+Shift+P`:

| Command | Keybinding | Description |
|---------|------------|-------------|
| `Scheme: Run Scheme File` | `F5` | Execute current file |
| `Scheme: Debug Scheme File` | `Ctrl+F5` | Start debugger |
| `Scheme: Compile Scheme to Assembly` | `Ctrl+Shift+B` | Generate assembly |
| `Scheme: Build and Run Scheme` | - | Full compile pipeline |

### Debugging

The extension includes a Debug Adapter Protocol (DAP) implementation:

#### Supported Debug Features

- **Breakpoints**: Click in gutter to set/remove
- **Step Into**: `F11` - Enter function calls
- **Step Over**: `F10` - Execute without entering
- **Step Out**: `Shift+F11` - Return from function
- **Continue**: `F5` - Run to next breakpoint
- **Call Stack**: View function call hierarchy
- **Variables**: Inspect local and global bindings
- **Watch**: Evaluate expressions in debug context

#### Debug Configuration

Add to `.vscode/launch.json`:

```json
{
  "type": "scheme",
  "request": "launch",
  "name": "Debug My Program",
  "program": "${file}",
  "stopOnEntry": false
}
```

## Configuration Settings

Configure in VS Code settings (`Ctrl+,`):

| Setting | Default | Description |
|---------|---------|-------------|
| `scheme.compilerPath` | (auto-detected) | Path to `lisp.exe` |
| `scheme.ml64Path` | `ml64.exe` | Path to MASM assembler |
| `scheme.linkPath` | `link.exe` | Path to linker |
| `scheme.outputDirectory` | `${workspaceFolder}/build` | Output directory |

### Example settings.json

```json
{
  "scheme.compilerPath": "C:/dev/LispCompiler/build/Release/lisp.exe",
  "scheme.ml64Path": "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.36.32532/bin/Hostx64/x64/ml64.exe",
  "scheme.linkPath": "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.36.32532/bin/Hostx64/x64/link.exe"
}
```

## File Type Associations

The extension associates these file types with Scheme:

| Extension | Description |
|-----------|-------------|
| `.scm` | Standard Scheme files |
| `.ss` | Scheme source |
| `.rkt` | Racket-compatible files |
| `.lisp` | Generic Lisp files |
| `.lsp` | Lisp source |

## Keyboard Shortcuts

### Scheme Files

| Shortcut | Action |
|----------|--------|
| `F5` | Run current file |
| `Ctrl+F5` | Debug current file |
| `Ctrl+Shift+B` | Compile to assembly |

### During Debugging

| Shortcut | Action |
|----------|--------|
| `F5` | Continue |
| `F10` | Step Over |
| `F11` | Step Into |
| `Shift+F11` | Step Out |
| `Ctrl+Shift+F5` | Restart |
| `Shift+F5` | Stop |

## Workflow Examples

### Basic Development Workflow

1. Create or open a `.scm` file
2. Write Scheme code with syntax highlighting
3. Press `F5` to run and see output
4. Fix any errors shown in the terminal
5. Repeat until working

### Debugging Workflow

1. Open your Scheme file
2. Click in the gutter to set breakpoints (red dots appear)
3. Press `Ctrl+F5` to start debugging
4. Use debug controls to step through code
5. Inspect variables in the Variables panel
6. View call stack in the Call Stack panel

### Compiled Executable Workflow

1. Open your `.scm` file
2. Run task "Full Build (Compile + Assemble + Link)"
3. Find the `.exe` in the same directory
4. Run the executable directly

## Troubleshooting

### "Scheme compiler not found"

1. Ensure LispCompiler is built: `cmake --build build --config Release`
2. Set `scheme.compilerPath` in settings to full path of `lisp.exe`

### "ml64.exe not found"

1. Open Visual Studio Developer Command Prompt
2. Run VS Code from there: `code .`
3. Or set `scheme.ml64Path` to full path

### Syntax highlighting not working

1. Check file extension is `.scm`, `.ss`, or `.rkt`
2. Try `Ctrl+Shift+P` → "Change Language Mode" → "Scheme"
3. Reload window: `Ctrl+Shift+P` → "Reload Window"

### Debugger not starting

1. Ensure compiler built with debug support
2. Check Output panel (View → Output → Scheme)
3. Try running with `--debug` flag manually first

## Extension Development

### Building from Source

```bash
cd LispCompiler/vscode-scheme
npm install
npm run compile
```

### Running Tests

```bash
npm test
```

### Packaging

```bash
vsce package
```

### Publishing

```bash
vsce publish
```

## Architecture

```
vscode-scheme/
├── package.json          # Extension manifest
├── language-configuration.json  # Bracket matching, comments
├── tsconfig.json         # TypeScript configuration
├── src/
│   ├── extension.ts      # Main extension entry point
│   └── debugAdapter.ts   # Debug Adapter Protocol implementation
└── syntaxes/
    └── scheme.tmLanguage.json  # TextMate grammar for highlighting
```

### Debug Adapter Communication

The debugger communicates with LispCompiler using JSON protocol:

```
VS Code ←→ Debug Adapter ←→ lisp.exe --debug-json
           (TypeScript)        (stdin/stdout JSON)
```

Commands sent to debugger:
```json
{"command": "continue"}
{"command": "step"}
{"command": "break", "file": "test.scm", "line": 10}
{"command": "print", "expression": "x"}
```

Events from debugger:
```json
{"event": "paused", "line": 10, "file": "test.scm", "reason": "breakpoint"}
{"event": "output", "text": "Hello, World!"}
{"event": "terminated"}
```

## Known Limitations

1. **Line-level breakpoints**: Breakpoints work at expression level, not exact line numbers
2. **Conditional breakpoints**: Not yet fully implemented
3. **Watch expressions**: Evaluation is asynchronous, may show stale values
4. **Hot reload**: Requires restart to pick up code changes during debug

## Future Enhancements

- [ ] IntelliSense for Scheme built-ins
- [ ] Hover documentation
- [ ] Go to Definition
- [ ] Find All References
- [ ] Code formatting
- [ ] Linting with scheme-lint
- [ ] REPL integration
- [ ] Full DAP compliance
- [ ] Conditional breakpoint evaluation
