# Scheme (LispCompiler) Extension for VS Code

Scheme/Lisp language support with LispCompiler integration for Visual Studio Code.

## Features

- **Syntax Highlighting** - Full support for R7RS Scheme syntax
- **Run Scheme Files** - Execute `.scm` files directly (F5)
- **Compile to Assembly** - Generate x86-64 MASM output
- **Debugger** - Step through code with breakpoints and variable inspection

## Requirements

- [LispCompiler](../README.md) built and available
- Visual Studio (for MASM assembler if compiling to native)

## Quick Start

1. Install the extension
2. Open a `.scm` file
3. Press `F5` to run

## Commands

| Command | Keybinding | Description |
|---------|------------|-------------|
| Run Scheme File | `F5` | Execute current file |
| Debug Scheme File | `Ctrl+F5` | Start debugger |
| Compile to Assembly | `Ctrl+Shift+B` | Generate .asm file |

## Configuration

```json
{
  "scheme.compilerPath": "path/to/lisp.exe",
  "scheme.ml64Path": "ml64.exe",
  "scheme.linkPath": "link.exe"
}
```

## Debugging

1. Set breakpoints by clicking in the gutter
2. Press `Ctrl+F5` to start debugging
3. Use F10/F11 to step through code
4. View variables and call stack in the debug panels

## Supported File Extensions

- `.scm` - Scheme source
- `.ss` - Scheme source
- `.rkt` - Racket-compatible
- `.lisp` / `.lsp` - Lisp source

## License

MIT
