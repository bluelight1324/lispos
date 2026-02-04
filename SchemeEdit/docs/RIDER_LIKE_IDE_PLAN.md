# Making SchemeEdit Like JetBrains Rider

## Overview

This document outlines how to transform SchemeEdit from a simple text editor into a full-featured IDE similar to JetBrains Rider. The plan is organized by feature category with implementation priorities.

## Current State vs Target

| Feature | Current SchemeEdit | Rider-like Target |
|---------|-------------------|-------------------|
| Syntax Highlighting | ✅ Basic | ✅ Advanced with semantic |
| Code Execution | ✅ Run/Run Selection | ✅ Run configurations |
| Compilation | ✅ Basic | ✅ Build system integration |
| Error Display | ❌ Text only | ✅ Inline error markers |
| Code Completion | ❌ None | ✅ IntelliSense |
| Navigation | ❌ None | ✅ Go to definition |
| Debugging | ❌ None | ✅ Step debugger |
| Project System | ❌ Single file | ✅ Multi-file projects |
| Version Control | ❌ None | ✅ Git integration |
| Refactoring | ❌ None | ✅ Rename, extract |

---

## Phase 1: Enhanced Editor Features (Priority: High)

### 1.1 Improved Syntax Highlighting

**Current**: Basic keyword highlighting
**Target**: Semantic highlighting with context awareness

```cpp
// Implementation in schemehighlighter.cpp
class SchemeHighlighter : public QSyntaxHighlighter {
    // Add semantic token types
    enum TokenType {
        Keyword,
        BuiltinFunction,
        UserFunction,
        Parameter,
        LocalVariable,
        GlobalVariable,
        Macro,
        String,
        Number,
        Comment
    };

    // Parse and track definitions for semantic highlighting
    QHash<QString, TokenType> m_symbolTable;
};
```

### 1.2 Bracket Matching and Rainbow Parentheses

**Implementation**:
```cpp
// In editor.cpp
void Editor::highlightMatchingParentheses() {
    QList<QTextEdit::ExtraSelection> selections;

    // Find matching paren at cursor
    QTextCursor cursor = textCursor();
    char ch = document()->characterAt(cursor.position());

    if (ch == '(' || ch == ')') {
        int matchPos = findMatchingParen(cursor.position());
        if (matchPos >= 0) {
            // Highlight both parentheses
            addParenHighlight(selections, cursor.position());
            addParenHighlight(selections, matchPos);
        }
    }

    setExtraSelections(selections);
}

// Rainbow colors for nested parens
QColor parenColors[] = {
    QColor("#FFD700"),  // Gold
    QColor("#DA70D6"),  // Orchid
    QColor("#87CEEB"),  // Sky Blue
    QColor("#98FB98"),  // Pale Green
    QColor("#FFA07A"),  // Light Salmon
};
```

### 1.3 Code Folding

**Implementation**:
```cpp
// Add folding support for:
// - Top-level definitions: (define ...)
// - Lambda expressions: (lambda ...)
// - Let blocks: (let ...)
// - Comments blocks

class FoldingArea : public QWidget {
    void paintEvent(QPaintEvent *event) override {
        // Draw fold markers [+] [-]
    }

    void mousePressEvent(QMouseEvent *event) override {
        // Toggle fold on click
    }
};
```

### 1.4 Line Numbers with Clickable Breakpoints

**Implementation**:
```cpp
class LineNumberArea : public QWidget {
    void paintEvent(QPaintEvent *event) override {
        // Draw line numbers
        // Draw breakpoint indicators (red circles)
        // Draw current line indicator (arrow)
    }

    void mousePressEvent(QMouseEvent *event) override {
        int line = getLineFromY(event->y());
        toggleBreakpoint(line);
    }
};
```

---

## Phase 2: Code Intelligence (Priority: High)

### 2.1 Code Completion (IntelliSense)

**Architecture**:
```
┌─────────────────────────────────────────────────────────────┐
│                    Code Completion System                    │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐  │
│  │   Parser     │───→│  Symbol      │───→│  Completion  │  │
│  │   (AST)      │    │  Table       │    │  Provider    │  │
│  └──────────────┘    └──────────────┘    └──────────────┘  │
│                                                │             │
│                                                ↓             │
│                                          ┌──────────────┐   │
│                                          │  Popup UI    │   │
│                                          │  (QListView) │   │
│                                          └──────────────┘   │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

**Implementation**:
```cpp
class CompletionProvider : public QObject {
public:
    QList<CompletionItem> getCompletions(const QString &prefix, int position);

private:
    // Built-in Scheme functions
    QStringList m_builtins = {
        "define", "lambda", "if", "cond", "let", "let*", "letrec",
        "begin", "set!", "quote", "quasiquote", "unquote",
        "car", "cdr", "cons", "list", "append", "reverse",
        "map", "filter", "fold", "for-each",
        "+", "-", "*", "/", "=", "<", ">", "<=", ">=",
        "display", "newline", "read", "write",
        // ... more
    };

    // User-defined symbols from current file
    QHash<QString, SymbolInfo> m_userSymbols;

    void parseForSymbols(const QString &code);
};

class CompletionPopup : public QListView {
    // Show completion popup near cursor
    // Handle arrow keys, Enter to select
    // Handle Escape to close
};
```

### 2.2 Go to Definition

**Implementation**:
```cpp
class NavigationProvider : public QObject {
public:
    struct Definition {
        QString file;
        int line;
        int column;
        QString snippet;
    };

    Definition findDefinition(const QString &symbol);
    QList<Definition> findUsages(const QString &symbol);

private:
    // Parse all project files for definitions
    QHash<QString, Definition> m_definitions;
};

// Keyboard shortcut: Ctrl+Click or F12
void Editor::goToDefinition() {
    QString symbol = getSymbolUnderCursor();
    auto def = m_navigationProvider->findDefinition(symbol);
    if (def.isValid()) {
        openFileAtPosition(def.file, def.line, def.column);
    }
}
```

### 2.3 Inline Error Highlighting

**Implementation**:
```cpp
class ErrorHighlighter : public QObject {
public:
    struct Error {
        int line;
        int column;
        int length;
        QString message;
        enum Severity { Error, Warning, Info };
        Severity severity;
    };

    void setErrors(const QList<Error> &errors);

private:
    // Draw red squiggly underlines for errors
    // Yellow for warnings
    // Show tooltip on hover
};

// Integrate with compiler output parsing
void MainWindow::onCompileFinished() {
    QList<Error> errors = parseCompilerOutput(output);
    m_editor->setErrors(errors);
}
```

### 2.4 Hover Information (Quick Documentation)

**Implementation**:
```cpp
class HoverProvider : public QObject {
public:
    QString getHoverInfo(const QString &symbol);

private:
    // Built-in documentation
    QHash<QString, QString> m_documentation = {
        {"define", "(define name value)\n(define (name args...) body...)\n\nDefines a variable or function."},
        {"lambda", "(lambda (args...) body...)\n\nCreates an anonymous function."},
        {"if", "(if condition then-expr else-expr)\n\nConditional expression."},
        // ... more
    };
};

// Show on mouse hover with delay
void Editor::mouseMoveEvent(QMouseEvent *event) {
    QTextCursor cursor = cursorForPosition(event->pos());
    QString symbol = getSymbolAtCursor(cursor);

    if (!symbol.isEmpty()) {
        QString info = m_hoverProvider->getHoverInfo(symbol);
        QToolTip::showText(event->globalPos(), info, this);
    }
}
```

---

## Phase 3: Project System (Priority: Medium)

### 3.1 Project Files

**Project file format** (`project.scmproj`):
```json
{
    "name": "MySchemeProject",
    "version": "1.0.0",
    "main": "src/main.scm",
    "sources": [
        "src/*.scm",
        "lib/*.scm"
    ],
    "include": [
        "include/"
    ],
    "output": "build/",
    "compiler": {
        "path": "lisp.exe",
        "flags": ["-O2"]
    }
}
```

### 3.2 Project Explorer Panel

**Implementation**:
```cpp
class ProjectExplorer : public QDockWidget {
    QTreeView *m_treeView;
    QFileSystemModel *m_model;

    // Context menu actions
    QAction *m_newFile;
    QAction *m_newFolder;
    QAction *m_rename;
    QAction *m_delete;
    QAction *m_openInExplorer;
};
```

### 3.3 Multi-Tab Editor

**Implementation**:
```cpp
class TabEditor : public QTabWidget {
public:
    Editor* openFile(const QString &path);
    void closeTab(int index);
    bool hasUnsavedChanges();

private:
    QHash<QString, Editor*> m_openFiles;

    // Tab with close button and modified indicator
    void updateTabText(int index) {
        Editor *editor = qobject_cast<Editor*>(widget(index));
        QString title = editor->fileName();
        if (editor->isModified()) {
            title = "• " + title;
        }
        setTabText(index, title);
    }
};
```

---

## Phase 4: Build System (Priority: Medium)

### 4.1 Build Configurations

```cpp
class BuildConfiguration {
public:
    QString name;           // "Debug", "Release"
    QString compilerPath;
    QStringList compilerFlags;
    QString outputDir;
    bool optimize;
    bool debugSymbols;
};

class BuildManager : public QObject {
public:
    void build(BuildConfiguration config);
    void rebuild();
    void clean();

signals:
    void buildStarted();
    void buildProgress(int percent, const QString &message);
    void buildFinished(bool success);
    void errorFound(const Error &error);
};
```

### 4.2 Build Output Panel

```cpp
class BuildOutputPanel : public QDockWidget {
    QPlainTextEdit *m_output;
    QProgressBar *m_progress;

    // Clickable error links
    void appendError(const Error &error) {
        QString link = QString("<a href=\"%1:%2\">%1:%2</a>: %3")
            .arg(error.file)
            .arg(error.line)
            .arg(error.message);
        m_output->appendHtml(link);
    }
};
```

---

## Phase 5: Debugging (Priority: Medium)

### 5.1 Debug Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Debugging System                          │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐  │
│  │   Editor     │←──→│   Debug      │←──→│  Interpreter │  │
│  │   UI         │    │   Controller │    │  with hooks  │  │
│  └──────────────┘    └──────────────┘    └──────────────┘  │
│        │                    │                    │          │
│        │                    ↓                    │          │
│        │             ┌──────────────┐           │          │
│        └────────────→│  Variables   │←──────────┘          │
│                      │  Panel       │                       │
│                      └──────────────┘                       │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

### 5.2 Breakpoints

```cpp
class BreakpointManager : public QObject {
public:
    void addBreakpoint(const QString &file, int line);
    void removeBreakpoint(const QString &file, int line);
    void toggleBreakpoint(const QString &file, int line);
    bool hasBreakpoint(const QString &file, int line);

    // Conditional breakpoints
    void setCondition(const QString &file, int line, const QString &condition);

signals:
    void breakpointHit(const QString &file, int line);
};
```

### 5.3 Variables Panel

```cpp
class VariablesPanel : public QDockWidget {
    QTreeView *m_treeView;
    VariablesModel *m_model;

    // Show:
    // - Local variables
    // - Function parameters
    // - Global variables
    // - Watch expressions
};
```

---

## Phase 6: Version Control (Priority: Low)

### 6.1 Git Integration

```cpp
class GitProvider : public QObject {
public:
    // Basic operations
    void commit(const QString &message);
    void push();
    void pull();

    // Status
    QList<FileStatus> getStatus();
    QString getDiff(const QString &file);

    // Branch operations
    QStringList getBranches();
    void checkout(const QString &branch);
    void createBranch(const QString &name);
};
```

### 6.2 Git Panel

```cpp
class GitPanel : public QDockWidget {
    QListView *m_changedFiles;
    QPlainTextEdit *m_commitMessage;
    QPushButton *m_commitButton;

    // Show file status with icons
    // Stage/unstage files
    // View diff
};
```

---

## Phase 7: Additional Features (Priority: Low)

### 7.1 Themes and Customization

```cpp
class ThemeManager : public QObject {
public:
    void setTheme(const QString &themeName);
    QStringList availableThemes();

    // Built-in themes
    // - Dark (VS Code dark)
    // - Light (VS Code light)
    // - Monokai
    // - Solarized Dark/Light
    // - Dracula
};
```

### 7.2 Settings Dialog

```cpp
class SettingsDialog : public QDialog {
    // Editor settings
    QSpinBox *m_fontSize;
    QFontComboBox *m_fontFamily;
    QSpinBox *m_tabSize;
    QCheckBox *m_showLineNumbers;
    QCheckBox *m_highlightCurrentLine;

    // Compiler settings
    QLineEdit *m_compilerPath;
    QLineEdit *m_compilerFlags;

    // Theme selection
    QComboBox *m_themeSelector;
};
```

### 7.3 Find and Replace

```cpp
class FindReplaceDialog : public QDialog {
    // Find in file
    // Find in project
    // Replace
    // Regex support
    // Case sensitivity
};
```

### 7.4 Terminal Integration

```cpp
class TerminalPanel : public QDockWidget {
    QTermWidget *m_terminal;  // Or custom terminal widget

    // Run commands
    // Interactive REPL
};
```

---

## Implementation Roadmap

### Sprint 1 (2 weeks): Editor Enhancements
- [ ] Bracket matching
- [ ] Rainbow parentheses
- [ ] Improved line numbers
- [ ] Basic code folding

### Sprint 2 (2 weeks): Code Intelligence
- [ ] Symbol parsing
- [ ] Basic code completion
- [ ] Go to definition (same file)

### Sprint 3 (2 weeks): Error Handling
- [ ] Inline error markers
- [ ] Error panel with clickable links
- [ ] Hover documentation

### Sprint 4 (2 weeks): Project System
- [ ] Project file format
- [ ] Project explorer panel
- [ ] Multi-tab editor

### Sprint 5 (2 weeks): Build System
- [ ] Build configurations
- [ ] Build output panel
- [ ] One-click build and run

### Sprint 6 (3 weeks): Debugging
- [ ] Breakpoint system
- [ ] Step execution
- [ ] Variables panel

### Sprint 7 (2 weeks): Polish
- [ ] Themes
- [ ] Settings dialog
- [ ] Find/Replace
- [ ] Git integration

---

## Technical Dependencies

### Required Qt Modules
- Qt Widgets (current)
- Qt Concurrent (for background parsing)
- Qt Network (for LSP support, future)

### Optional Dependencies
- QScintilla (advanced editor component)
- TreeSitter (fast incremental parsing)
- LibGit2 (Git integration)

---

## Alternative Approach: Language Server Protocol (LSP)

Instead of implementing all features in the editor, create an LSP server for Scheme:

```
┌─────────────────┐         ┌─────────────────┐
│   SchemeEdit    │←─JSON──→│  Scheme LSP     │
│   (LSP Client)  │   RPC   │  Server         │
└─────────────────┘         └─────────────────┘
```

**Benefits**:
- Works with any LSP-compatible editor (VS Code, Vim, Emacs)
- Separates language features from editor UI
- Industry standard

**LSP Features**:
- textDocument/completion
- textDocument/hover
- textDocument/definition
- textDocument/references
- textDocument/formatting
- textDocument/diagnostics

---

## Conclusion

Making SchemeEdit like Rider is a significant undertaking but achievable in phases. The recommended priority order is:

1. **Editor enhancements** (immediate value, low effort)
2. **Code completion** (high value, medium effort)
3. **Error highlighting** (high value, medium effort)
4. **Project system** (enables larger projects)
5. **Debugging** (complex but valuable)
6. **Version control** (nice to have)

Alternatively, investing in LSP support would make the language features portable across multiple editors.
