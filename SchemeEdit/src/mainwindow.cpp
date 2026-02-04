#include "mainwindow.h"
#include "editor.h"
#include "schemerunner.h"
#include "debugcontroller.h"
#include "findreplacedialog.h"

#include <QApplication>
#include <QInputDialog>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QTextStream>
#include <QFont>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QRegularExpression>
#include <QHeaderView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_editor(new Editor(this))
    , m_outputPane(new QPlainTextEdit(this))
    , m_schemeRunner(new SchemeRunner(this))
    , m_statusLabel(new QLabel(this))
    , m_debugController(new DebugController(this))
    , m_callStackDock(nullptr)
    , m_variablesDock(nullptr)
    , m_callStackList(nullptr)
    , m_findReplaceDialog(nullptr)
    , m_variablesTree(nullptr)
    , m_isModified(false)
{
    setCentralWidget(m_editor);
    setWindowTitle("SchemeEdit");
    resize(1024, 768);

    // Find compiler
    m_compilerPath = findCompiler();

    // Configure output pane
    m_outputPane->setReadOnly(true);
    m_outputPane->setFont(QFont("Consolas", 10));
    m_outputPane->setStyleSheet(
        "QPlainTextEdit { background-color: #1e1e1e; color: #d4d4d4; }"
    );

    createActions();
    createMenus();
    createToolBar();
    createStatusBar();
    createDockWidgets();
    createDebugPanels();

    // Connect editor signals
    connect(m_editor, &QPlainTextEdit::textChanged, this, &MainWindow::documentModified);
    connect(m_editor, &Editor::breakpointToggled, this, &MainWindow::onBreakpointToggled);

    // Connect debug controller signals
    connect(m_debugController, &DebugController::debuggingStarted, this, &MainWindow::onDebuggingStarted);
    connect(m_debugController, &DebugController::debuggingStopped, this, &MainWindow::onDebuggingStopped);
    connect(m_debugController, &DebugController::paused, this, &MainWindow::onDebugPaused);
    connect(m_debugController, &DebugController::continued, this, &MainWindow::onDebugContinued);
    connect(m_debugController, &DebugController::stackTraceReceived, this, &MainWindow::onStackTraceReceived);
    connect(m_debugController, &DebugController::variablesReceived, this, &MainWindow::onVariablesReceived);
    connect(m_debugController, &DebugController::outputReceived, this, &MainWindow::onDebugOutput);
    connect(m_debugController, &DebugController::errorReceived, this, &MainWindow::onDebugError);

    // Connect Scheme runner signals
    connect(m_schemeRunner, &SchemeRunner::outputReady, this, [this](const QString &output) {
        m_outputPane->appendPlainText(output);
    });
    connect(m_schemeRunner, &SchemeRunner::errorOccurred, this, [this](const QString &error) {
        m_outputPane->appendPlainText("❌ Error: " + error);
    });
    connect(m_schemeRunner, &SchemeRunner::finished, this, &MainWindow::onRunFinished);

    setCurrentFile(QString());

    // Show welcome message
    m_outputPane->appendPlainText("═══════════════════════════════════════════════════════");
    m_outputPane->appendPlainText("  Welcome to SchemeEdit!");
    m_outputPane->appendPlainText("  ");
    m_outputPane->appendPlainText("  Keyboard Shortcuts:");
    m_outputPane->appendPlainText("    F5           - Run code");
    m_outputPane->appendPlainText("    Ctrl+Enter   - Run selection/current line");
    m_outputPane->appendPlainText("    Ctrl+B       - Compile to assembly");
    m_outputPane->appendPlainText("    Ctrl+Shift+B - Compile and run");
    m_outputPane->appendPlainText("    Ctrl+L       - Clear output");
    m_outputPane->appendPlainText("  ");
    m_outputPane->appendPlainText("  Check the Examples menu for sample programs!");
    m_outputPane->appendPlainText("═══════════════════════════════════════════════════════");
    m_outputPane->appendPlainText("");
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}

QString MainWindow::findCompiler() const
{
    QStringList searchPaths = {
        QCoreApplication::applicationDirPath() + "/lisp.exe",
        QCoreApplication::applicationDirPath() + "/../lisp.exe",
        QCoreApplication::applicationDirPath() + "/../../LispCompiler/build/Release/lisp.exe",
        "I:/lispos/LispCompiler/build/Release/lisp.exe",
        "C:/lispos/LispCompiler/build/Release/lisp.exe",
    };

    for (const QString &path : searchPaths) {
        if (QFileInfo::exists(path)) {
            return QFileInfo(path).absoluteFilePath();
        }
    }
    return QString();
}

QString MainWindow::findExamplesDir() const
{
    QStringList searchPaths = {
        QCoreApplication::applicationDirPath() + "/examples",
        QCoreApplication::applicationDirPath() + "/../examples",
        QCoreApplication::applicationDirPath() + "/../../SchemeEdit/examples",
        "I:/lispos/SchemeEdit/examples",
    };

    for (const QString &path : searchPaths) {
        if (QDir(path).exists()) {
            return QDir(path).absolutePath();
        }
    }
    return QString();
}

void MainWindow::createActions()
{
    // File actions
    m_newAction = new QAction(tr("&New"), this);
    m_newAction->setShortcuts(QKeySequence::New);
    m_newAction->setStatusTip(tr("Create a new file"));
    connect(m_newAction, &QAction::triggered, this, &MainWindow::newFile);

    m_openAction = new QAction(tr("&Open..."), this);
    m_openAction->setShortcuts(QKeySequence::Open);
    m_openAction->setStatusTip(tr("Open an existing file"));
    connect(m_openAction, &QAction::triggered, this, &MainWindow::openFile);

    m_saveAction = new QAction(tr("&Save"), this);
    m_saveAction->setShortcuts(QKeySequence::Save);
    m_saveAction->setStatusTip(tr("Save the document"));
    connect(m_saveAction, &QAction::triggered, this, &MainWindow::saveFile);

    m_saveAsAction = new QAction(tr("Save &As..."), this);
    m_saveAsAction->setShortcuts(QKeySequence::SaveAs);
    m_saveAsAction->setStatusTip(tr("Save under a new name"));
    connect(m_saveAsAction, &QAction::triggered, this, &MainWindow::saveFileAs);

    m_exitAction = new QAction(tr("E&xit"), this);
    m_exitAction->setShortcuts(QKeySequence::Quit);
    m_exitAction->setStatusTip(tr("Exit the application"));
    connect(m_exitAction, &QAction::triggered, this, &QWidget::close);

    // Edit actions
    m_undoAction = new QAction(tr("&Undo"), this);
    m_undoAction->setShortcuts(QKeySequence::Undo);
    connect(m_undoAction, &QAction::triggered, m_editor, &QPlainTextEdit::undo);

    m_redoAction = new QAction(tr("&Redo"), this);
    m_redoAction->setShortcuts(QKeySequence::Redo);
    connect(m_redoAction, &QAction::triggered, m_editor, &QPlainTextEdit::redo);

    m_cutAction = new QAction(tr("Cu&t"), this);
    m_cutAction->setShortcuts(QKeySequence::Cut);
    connect(m_cutAction, &QAction::triggered, m_editor, &QPlainTextEdit::cut);

    m_copyAction = new QAction(tr("&Copy"), this);
    m_copyAction->setShortcuts(QKeySequence::Copy);
    connect(m_copyAction, &QAction::triggered, m_editor, &QPlainTextEdit::copy);

    m_pasteAction = new QAction(tr("&Paste"), this);
    m_pasteAction->setShortcuts(QKeySequence::Paste);
    connect(m_pasteAction, &QAction::triggered, m_editor, &QPlainTextEdit::paste);

    m_selectAllAction = new QAction(tr("Select &All"), this);
    m_selectAllAction->setShortcuts(QKeySequence::SelectAll);
    connect(m_selectAllAction, &QAction::triggered, m_editor, &QPlainTextEdit::selectAll);

    // Find/Replace actions (Essential #7)
    m_findAction = new QAction(tr("&Find..."), this);
    m_findAction->setShortcuts(QKeySequence::Find);
    m_findAction->setStatusTip(tr("Find text (Ctrl+F)"));
    connect(m_findAction, &QAction::triggered, this, &MainWindow::showFindDialog);

    m_replaceAction = new QAction(tr("&Replace..."), this);
    m_replaceAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_H));
    m_replaceAction->setStatusTip(tr("Find and replace text (Ctrl+H)"));
    connect(m_replaceAction, &QAction::triggered, this, &MainWindow::showReplaceDialog);

    m_findNextAction = new QAction(tr("Find &Next"), this);
    m_findNextAction->setShortcut(QKeySequence(Qt::Key_F3));
    m_findNextAction->setStatusTip(tr("Find next occurrence (F3)"));
    connect(m_findNextAction, &QAction::triggered, this, &MainWindow::findNext);

    m_findPrevAction = new QAction(tr("Find &Previous"), this);
    m_findPrevAction->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F3));
    m_findPrevAction->setStatusTip(tr("Find previous occurrence (Shift+F3)"));
    connect(m_findPrevAction, &QAction::triggered, this, &MainWindow::findPrevious);

    m_goToLineAction = new QAction(tr("&Go to Line..."), this);
    m_goToLineAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_G));
    m_goToLineAction->setStatusTip(tr("Go to line (Ctrl+G)"));
    connect(m_goToLineAction, &QAction::triggered, this, &MainWindow::goToLine);

    // Scheme actions
    m_runAction = new QAction(tr("▶ &Run"), this);
    m_runAction->setShortcut(QKeySequence(Qt::Key_F5));
    m_runAction->setStatusTip(tr("Run the Scheme code (F5)"));
    connect(m_runAction, &QAction::triggered, this, &MainWindow::runScheme);

    m_runSelectionAction = new QAction(tr("Run &Selection"), this);
    m_runSelectionAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return));
    m_runSelectionAction->setStatusTip(tr("Run selected Scheme code (Ctrl+Enter)"));
    connect(m_runSelectionAction, &QAction::triggered, this, &MainWindow::runSelection);

    m_compileAction = new QAction(tr("⚙ &Compile to Assembly"), this);
    m_compileAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_B));
    m_compileAction->setStatusTip(tr("Compile to MASM x64 assembly (Ctrl+B)"));
    connect(m_compileAction, &QAction::triggered, this, &MainWindow::compileScheme);

    m_compileRunAction = new QAction(tr("⚡ Compile && Run"), this);
    m_compileRunAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_B));
    m_compileRunAction->setStatusTip(tr("Compile, link, and run (Ctrl+Shift+B)"));
    connect(m_compileRunAction, &QAction::triggered, this, &MainWindow::compileAndRun);

    m_clearOutputAction = new QAction(tr("&Clear Output"), this);
    m_clearOutputAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));
    connect(m_clearOutputAction, &QAction::triggered, this, &MainWindow::clearOutput);

    // Debug actions
    m_debugStartAction = new QAction(tr("Start &Debugging"), this);
    m_debugStartAction->setShortcut(QKeySequence(Qt::Key_F9));
    m_debugStartAction->setStatusTip(tr("Start debugging (F9)"));
    connect(m_debugStartAction, &QAction::triggered, this, &MainWindow::startDebugging);

    m_debugStopAction = new QAction(tr("S&top Debugging"), this);
    m_debugStopAction->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F5));
    m_debugStopAction->setStatusTip(tr("Stop debugging (Shift+F5)"));
    m_debugStopAction->setEnabled(false);
    connect(m_debugStopAction, &QAction::triggered, this, &MainWindow::stopDebugging);

    m_debugContinueAction = new QAction(tr("&Continue"), this);
    m_debugContinueAction->setShortcut(QKeySequence(Qt::Key_F5));
    m_debugContinueAction->setStatusTip(tr("Continue execution (F5)"));
    m_debugContinueAction->setEnabled(false);
    connect(m_debugContinueAction, &QAction::triggered, this, &MainWindow::debugContinue);

    m_debugStepIntoAction = new QAction(tr("Step &Into"), this);
    m_debugStepIntoAction->setShortcut(QKeySequence(Qt::Key_F11));
    m_debugStepIntoAction->setStatusTip(tr("Step into (F11)"));
    m_debugStepIntoAction->setEnabled(false);
    connect(m_debugStepIntoAction, &QAction::triggered, this, &MainWindow::debugStepInto);

    m_debugStepOverAction = new QAction(tr("Step &Over"), this);
    m_debugStepOverAction->setShortcut(QKeySequence(Qt::Key_F10));
    m_debugStepOverAction->setStatusTip(tr("Step over (F10)"));
    m_debugStepOverAction->setEnabled(false);
    connect(m_debugStepOverAction, &QAction::triggered, this, &MainWindow::debugStepOver);

    m_debugStepOutAction = new QAction(tr("Step O&ut"), this);
    m_debugStepOutAction->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F11));
    m_debugStepOutAction->setStatusTip(tr("Step out (Shift+F11)"));
    m_debugStepOutAction->setEnabled(false);
    connect(m_debugStepOutAction, &QAction::triggered, this, &MainWindow::debugStepOut);

    m_debugToggleBreakpointAction = new QAction(tr("Toggle &Breakpoint"), this);
    m_debugToggleBreakpointAction->setShortcut(QKeySequence(Qt::Key_F9));
    m_debugToggleBreakpointAction->setStatusTip(tr("Toggle breakpoint (F9)"));
    connect(m_debugToggleBreakpointAction, &QAction::triggered, this, &MainWindow::debugToggleBreakpoint);

    // Help actions
    m_helpAction = new QAction(tr("&Quick Start Guide"), this);
    m_helpAction->setShortcut(QKeySequence::HelpContents);
    connect(m_helpAction, &QAction::triggered, this, &MainWindow::showHelp);

    m_aboutAction = new QAction(tr("&About SchemeEdit"), this);
    m_aboutAction->setStatusTip(tr("About SchemeEdit"));
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::about);

    m_aboutQtAction = new QAction(tr("About &Qt"), this);
    connect(m_aboutQtAction, &QAction::triggered, qApp, &QApplication::aboutQt);
}

void MainWindow::createExamplesMenu()
{
    m_examplesMenu = menuBar()->addMenu(tr("E&xamples"));

    QString examplesDir = findExamplesDir();
    if (examplesDir.isEmpty()) {
        QAction *noExamples = m_examplesMenu->addAction(tr("(No examples found)"));
        noExamples->setEnabled(false);
        return;
    }

    QDir dir(examplesDir);
    QStringList filters;
    filters << "*.scm" << "*.ss";
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Name);

    for (const QFileInfo &fileInfo : files) {
        QString name = fileInfo.baseName();
        // Make the name more readable
        name.replace('_', ' ');
        // Remove leading numbers like "01_"
        if (name.length() > 3 && name[2] == ' ') {
            name = name.mid(3);
        }
        name[0] = name[0].toUpper();

        QAction *action = m_examplesMenu->addAction(name);
        action->setData(fileInfo.absoluteFilePath());
        action->setStatusTip(tr("Load example: %1").arg(fileInfo.fileName()));
        connect(action, &QAction::triggered, this, [this, action]() {
            loadExample(action->data().toString());
        });
    }
}

void MainWindow::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(m_newAction);
    fileMenu->addAction(m_openAction);
    fileMenu->addAction(m_saveAction);
    fileMenu->addAction(m_saveAsAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exitAction);

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(m_undoAction);
    editMenu->addAction(m_redoAction);
    editMenu->addSeparator();
    editMenu->addAction(m_cutAction);
    editMenu->addAction(m_copyAction);
    editMenu->addAction(m_pasteAction);
    editMenu->addSeparator();
    editMenu->addAction(m_selectAllAction);
    editMenu->addSeparator();
    editMenu->addAction(m_findAction);
    editMenu->addAction(m_replaceAction);
    editMenu->addAction(m_findNextAction);
    editMenu->addAction(m_findPrevAction);
    editMenu->addSeparator();
    editMenu->addAction(m_goToLineAction);

    QMenu *schemeMenu = menuBar()->addMenu(tr("&Scheme"));
    schemeMenu->addAction(m_runAction);
    schemeMenu->addAction(m_runSelectionAction);
    schemeMenu->addSeparator();
    schemeMenu->addAction(m_compileAction);
    schemeMenu->addAction(m_compileRunAction);
    schemeMenu->addSeparator();
    schemeMenu->addAction(m_clearOutputAction);

    createExamplesMenu();

    // Debug menu
    m_debugMenu = menuBar()->addMenu(tr("&Debug"));
    m_debugMenu->addAction(m_debugStartAction);
    m_debugMenu->addAction(m_debugStopAction);
    m_debugMenu->addSeparator();
    m_debugMenu->addAction(m_debugContinueAction);
    m_debugMenu->addAction(m_debugStepIntoAction);
    m_debugMenu->addAction(m_debugStepOverAction);
    m_debugMenu->addAction(m_debugStepOutAction);
    m_debugMenu->addSeparator();
    m_debugMenu->addAction(m_debugToggleBreakpointAction);

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(m_helpAction);
    helpMenu->addSeparator();
    helpMenu->addAction(m_aboutAction);
    helpMenu->addAction(m_aboutQtAction);
}

void MainWindow::createToolBar()
{
    QToolBar *fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(m_newAction);
    fileToolBar->addAction(m_openAction);
    fileToolBar->addAction(m_saveAction);

    QToolBar *editToolBar = addToolBar(tr("Edit"));
    editToolBar->addAction(m_cutAction);
    editToolBar->addAction(m_copyAction);
    editToolBar->addAction(m_pasteAction);

    QToolBar *schemeToolBar = addToolBar(tr("Scheme"));
    schemeToolBar->addAction(m_runAction);
    schemeToolBar->addAction(m_runSelectionAction);

    // Debug toolbar
    m_debugToolBar = addToolBar(tr("Debug"));
    m_debugToolBar->addAction(m_debugStartAction);
    m_debugToolBar->addAction(m_debugStopAction);
    m_debugToolBar->addSeparator();
    m_debugToolBar->addAction(m_debugContinueAction);
    m_debugToolBar->addAction(m_debugStepIntoAction);
    m_debugToolBar->addAction(m_debugStepOverAction);
    m_debugToolBar->addAction(m_debugStepOutAction);
}

void MainWindow::createStatusBar()
{
    m_statusLabel->setText("Ready");
    m_statusLabel->setStyleSheet("color: green; font-weight: bold;");
    statusBar()->addPermanentWidget(m_statusLabel);
    statusBar()->showMessage(tr("Welcome to SchemeEdit - Check the Examples menu to get started!"));
}

void MainWindow::createDockWidgets()
{
    m_outputDock = new QDockWidget(tr("Output"), this);
    m_outputDock->setWidget(m_outputPane);
    m_outputDock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea, m_outputDock);
}

void MainWindow::newFile()
{
    if (maybeSave()) {
        m_editor->clear();
        setCurrentFile(QString());
    }
}

void MainWindow::openFile()
{
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open Scheme File"), QString(),
            tr("Scheme Files (*.scm *.ss *.rkt);;All Files (*)"));
        if (!fileName.isEmpty()) {
            loadFile(fileName);
        }
    }
}

bool MainWindow::saveFile()
{
    if (m_currentFile.isEmpty()) {
        return saveFileAs();
    }
    return saveToFile(m_currentFile);
}

bool MainWindow::saveFileAs()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save Scheme File"), QString(),
        tr("Scheme Files (*.scm);;All Files (*)"));
    if (fileName.isEmpty()) {
        return false;
    }
    return saveToFile(fileName);
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About SchemeEdit"),
        tr("<h2>SchemeEdit 1.0</h2>"
           "<p>A full-featured Scheme development environment.</p>"
           "<p>Built with Qt and the LispOS R7RS Scheme compiler.</p>"
           "<p><b>Features:</b></p>"
           "<ul>"
           "<li>Syntax highlighting for Scheme</li>"
           "<li>Run code with F5 or selected code with Ctrl+Enter</li>"
           "<li>Compile to x64 MASM assembly</li>"
           "<li>Integrated output pane with results</li>"
           "<li>Example programs in the Examples menu</li>"
           "</ul>"
           "<p><b>Keyboard Shortcuts:</b></p>"
           "<table>"
           "<tr><td>F5</td><td>Run code</td></tr>"
           "<tr><td>Ctrl+Enter</td><td>Run selection</td></tr>"
           "<tr><td>Ctrl+B</td><td>Compile to assembly</td></tr>"
           "<tr><td>Ctrl+Shift+B</td><td>Compile and run</td></tr>"
           "<tr><td>Ctrl+L</td><td>Clear output</td></tr>"
           "</table>"));
}

void MainWindow::showHelp()
{
    QMessageBox::information(this, tr("Quick Start Guide"),
        tr("<h2>SchemeEdit Quick Start</h2>"
           "<h3>1. Getting Started</h3>"
           "<p>Check the <b>Examples</b> menu for sample programs with explanations. "
           "Select any example to load it into the editor.</p>"
           "<h3>2. Running Code</h3>"
           "<ul>"
           "<li><b>F5</b> - Run the entire file</li>"
           "<li><b>Ctrl+Enter</b> - Run selected text or current line</li>"
           "</ul>"
           "<h3>3. Compiling</h3>"
           "<ul>"
           "<li><b>Ctrl+B</b> - Compile to MASM x64 assembly</li>"
           "<li><b>Ctrl+Shift+B</b> - Compile, assemble, link, and run</li>"
           "</ul>"
           "<h3>4. Basic Scheme Syntax</h3>"
           "<pre>"
           "; This is a comment\n"
           "(+ 1 2 3)           ; Addition\n"
           "(define x 42)       ; Define variable\n"
           "(define (f x) (* x x)) ; Define function\n"
           "(f 5)               ; Call function\n"
           "</pre>"
           "<h3>5. Examples</h3>"
           "<p>The Examples menu contains:</p>"
           "<ul>"
           "<li>Hello World - Basic output</li>"
           "<li>Variables - Defining and using values</li>"
           "<li>Functions - Creating reusable code</li>"
           "<li>Conditionals - If/cond/and/or</li>"
           "<li>Recursion - Self-referencing functions</li>"
           "<li>Lists - Working with data structures</li>"
           "<li>Higher-Order - Functions as values</li>"
           "<li>Complete Program - Prime number generator</li>"
           "</ul>"));
}

void MainWindow::loadExample(const QString &examplePath)
{
    if (maybeSave()) {
        loadFile(examplePath);
        m_outputPane->appendPlainText("───────────────────────────────────────────────────────");
        m_outputPane->appendPlainText("📚 Loaded example: " + QFileInfo(examplePath).fileName());
        m_outputPane->appendPlainText("   Press F5 to run this example!");
        m_outputPane->appendPlainText("───────────────────────────────────────────────────────");
        m_outputPane->appendPlainText("");
    }
}

void MainWindow::onRunFinished(int exitCode)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    if (exitCode == 0) {
        m_outputPane->appendPlainText("");
        m_outputPane->appendPlainText("✓ Execution completed successfully [" + timestamp + "]");
        m_statusLabel->setText("Ready");
        m_statusLabel->setStyleSheet("color: green;");
    } else {
        m_outputPane->appendPlainText("");
        m_outputPane->appendPlainText("✗ Execution failed with exit code " + QString::number(exitCode) + " [" + timestamp + "]");
        m_statusLabel->setText("Error");
        m_statusLabel->setStyleSheet("color: red;");
    }
    m_outputPane->appendPlainText("───────────────────────────────────────────────────────");
    m_outputPane->appendPlainText("");
}

void MainWindow::compileScheme()
{
    if (m_compilerPath.isEmpty()) {
        m_outputPane->appendPlainText("❌ Error: Compiler not found!");
        m_outputPane->appendPlainText("   Please ensure LispCompiler is built at I:/lispos/LispCompiler/build/Release/lisp.exe");
        return;
    }

    // Save file first if needed
    if (m_currentFile.isEmpty()) {
        if (!saveFileAs()) {
            return;
        }
    } else if (m_isModified) {
        saveFile();
    }

    QString asmFile = m_currentFile;
    asmFile.replace(QRegularExpression("\\.scm$", QRegularExpression::CaseInsensitiveOption), ".asm");

    m_outputPane->appendPlainText("═══════════════════════════════════════════════════════");
    m_outputPane->appendPlainText("⚙ Compiling: " + QFileInfo(m_currentFile).fileName());
    m_outputPane->appendPlainText("  Output:    " + QFileInfo(asmFile).fileName());
    m_outputPane->appendPlainText("───────────────────────────────────────────────────────");

    QProcess compiler;
    compiler.start(m_compilerPath, QStringList() << "-c" << m_currentFile << "-o" << asmFile);
    compiler.waitForFinished(30000);

    QString output = QString::fromUtf8(compiler.readAllStandardOutput());
    QString error = QString::fromUtf8(compiler.readAllStandardError());

    if (!output.isEmpty()) {
        m_outputPane->appendPlainText(output);
    }
    if (!error.isEmpty()) {
        m_outputPane->appendPlainText("❌ " + error);
    }

    if (compiler.exitCode() == 0) {
        m_outputPane->appendPlainText("✓ Compilation successful!");
        m_outputPane->appendPlainText("  Generated: " + asmFile);
        m_statusLabel->setText("Compiled");
        m_statusLabel->setStyleSheet("color: green;");
    } else {
        m_outputPane->appendPlainText("✗ Compilation failed");
        m_statusLabel->setText("Compile Error");
        m_statusLabel->setStyleSheet("color: red;");
    }
    m_outputPane->appendPlainText("═══════════════════════════════════════════════════════");
    m_outputPane->appendPlainText("");
}

void MainWindow::compileAndRun()
{
    // For now, just run interpreted (full compile+link requires ml64 and link)
    m_outputPane->appendPlainText("═══════════════════════════════════════════════════════");
    m_outputPane->appendPlainText("⚡ Compile & Run (Interpreted Mode)");
    m_outputPane->appendPlainText("───────────────────────────────────────────────────────");
    runScheme();
}

void MainWindow::runScheme()
{
    QString code = m_editor->toPlainText();
    if (!code.isEmpty()) {
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
        m_outputPane->appendPlainText("▶ Running... [" + timestamp + "]");
        m_outputPane->appendPlainText("");
        m_statusLabel->setText("Running...");
        m_statusLabel->setStyleSheet("color: blue;");
        m_schemeRunner->run(code);
    }
}

void MainWindow::runSelection()
{
    QString code = m_editor->textCursor().selectedText();
    if (code.isEmpty()) {
        // If no selection, run current line
        QTextCursor cursor = m_editor->textCursor();
        cursor.select(QTextCursor::LineUnderCursor);
        code = cursor.selectedText();
    }
    if (!code.isEmpty()) {
        // Replace paragraph separators with newlines
        code.replace(QChar::ParagraphSeparator, '\n');
        m_outputPane->appendPlainText(">>> " + code.left(50) + (code.length() > 50 ? "..." : "") + "\n");
        m_schemeRunner->run(code);
    }
}

void MainWindow::clearOutput()
{
    m_outputPane->clear();
}

void MainWindow::documentModified()
{
    m_isModified = true;
    updateWindowTitle();
}

void MainWindow::updateWindowTitle()
{
    QString title = "SchemeEdit";
    if (!m_currentFile.isEmpty()) {
        title = QFileInfo(m_currentFile).fileName() + " - " + title;
    } else {
        title = "Untitled - " + title;
    }
    if (m_isModified) {
        title = "*" + title;
    }
    setWindowTitle(title);
}

void MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("SchemeEdit"),
            tr("Cannot read file %1:\n%2.")
            .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    QTextStream in(&file);
    m_editor->setPlainText(in.readAll());

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 2000);
}

bool MainWindow::saveToFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("SchemeEdit"),
            tr("Cannot write file %1:\n%2.")
            .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return false;
    }

    QTextStream out(&file);
    out << m_editor->toPlainText();

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File saved"), 2000);
    return true;
}

bool MainWindow::maybeSave()
{
    if (!m_isModified) {
        return true;
    }

    const QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("SchemeEdit"),
        tr("The document has been modified.\nDo you want to save your changes?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    switch (ret) {
    case QMessageBox::Save:
        return saveFile();
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    m_currentFile = fileName;
    m_isModified = false;
    updateWindowTitle();
}

void MainWindow::createDebugPanels()
{
    // Call Stack panel
    m_callStackDock = new QDockWidget(tr("Call Stack"), this);
    m_callStackList = new QListWidget(this);
    m_callStackList->setStyleSheet(
        "QListWidget { background-color: #2d2d2d; color: #d4d4d4; }"
    );
    m_callStackDock->setWidget(m_callStackList);
    m_callStackDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, m_callStackDock);
    m_callStackDock->hide();  // Hidden by default

    // Variables panel
    m_variablesDock = new QDockWidget(tr("Variables"), this);
    m_variablesTree = new QTreeWidget(this);
    m_variablesTree->setHeaderLabels(QStringList() << "Name" << "Value" << "Type");
    m_variablesTree->setStyleSheet(
        "QTreeWidget { background-color: #2d2d2d; color: #d4d4d4; }"
    );
    m_variablesTree->header()->setStretchLastSection(true);
    m_variablesDock->setWidget(m_variablesTree);
    m_variablesDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, m_variablesDock);
    m_variablesDock->hide();  // Hidden by default
}

void MainWindow::updateDebugActions(bool debugging, bool paused)
{
    m_debugStartAction->setEnabled(!debugging);
    m_debugStopAction->setEnabled(debugging);
    m_debugContinueAction->setEnabled(debugging && paused);
    m_debugStepIntoAction->setEnabled(debugging && paused);
    m_debugStepOverAction->setEnabled(debugging && paused);
    m_debugStepOutAction->setEnabled(debugging && paused);

    // Disable run actions while debugging
    m_runAction->setEnabled(!debugging);
    m_runSelectionAction->setEnabled(!debugging);
}

// Debug slot implementations
void MainWindow::startDebugging()
{
    if (m_compilerPath.isEmpty()) {
        m_outputPane->appendPlainText("Error: Compiler/debugger not found!");
        return;
    }

    // Save file first
    if (m_currentFile.isEmpty()) {
        if (!saveFileAs()) {
            return;
        }
    } else if (m_isModified) {
        saveFile();
    }

    m_outputPane->appendPlainText("Starting debugger...");
    m_debugController->startDebugging(m_currentFile, m_compilerPath);

    // Send existing breakpoints
    for (int line : m_editor->getBreakpoints()) {
        m_debugController->setBreakpoint(m_currentFile, line + 1);  // 1-based line numbers
    }
}

void MainWindow::stopDebugging()
{
    m_debugController->stopDebugging();
    m_editor->clearDebugLine();
}

void MainWindow::debugContinue()
{
    m_debugController->continueExecution();
    m_editor->clearDebugLine();
}

void MainWindow::debugStepInto()
{
    m_debugController->stepInto();
}

void MainWindow::debugStepOver()
{
    m_debugController->stepOver();
}

void MainWindow::debugStepOut()
{
    m_debugController->stepOut();
}

void MainWindow::debugToggleBreakpoint()
{
    int line = m_editor->textCursor().blockNumber();
    m_editor->toggleBreakpoint(line);
}

void MainWindow::onDebuggingStarted()
{
    m_outputPane->appendPlainText("Debugging started.");
    updateDebugActions(true, true);
    m_callStackDock->show();
    m_variablesDock->show();
    m_statusLabel->setText("Debugging");
    m_statusLabel->setStyleSheet("color: orange;");
}

void MainWindow::onDebuggingStopped()
{
    m_outputPane->appendPlainText("Debugging stopped.");
    updateDebugActions(false, false);
    m_editor->clearDebugLine();
    m_callStackList->clear();
    m_variablesTree->clear();
    m_statusLabel->setText("Ready");
    m_statusLabel->setStyleSheet("color: green;");
}

void MainWindow::onDebugPaused(const QString &file, int line, const QString &reason)
{
    Q_UNUSED(file)
    m_outputPane->appendPlainText(QString("Paused at line %1 (%2)").arg(line).arg(reason));
    m_editor->setDebugLine(line - 1);  // Convert to 0-based
    updateDebugActions(true, true);
    m_statusLabel->setText("Paused");
    m_statusLabel->setStyleSheet("color: yellow;");

    // Request variables for the current frame
    m_debugController->requestVariables(0);
}

void MainWindow::onDebugContinued()
{
    m_editor->clearDebugLine();
    updateDebugActions(true, false);
    m_statusLabel->setText("Running");
    m_statusLabel->setStyleSheet("color: blue;");
}

void MainWindow::onStackTraceReceived(const QVector<StackFrame> &frames)
{
    m_callStackList->clear();
    for (const StackFrame &frame : frames) {
        QString text = QString("#%1 %2 at line %3")
            .arg(frame.id)
            .arg(frame.name)
            .arg(frame.line);
        m_callStackList->addItem(text);
    }
}

void MainWindow::onVariablesReceived(const QVector<Variable> &variables)
{
    m_variablesTree->clear();
    for (const Variable &var : variables) {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, var.name);
        item->setText(1, var.value);
        item->setText(2, var.type);
        m_variablesTree->addTopLevelItem(item);
    }
}

void MainWindow::onDebugOutput(const QString &output)
{
    m_outputPane->appendPlainText(output);
}

void MainWindow::onDebugError(const QString &error)
{
    m_outputPane->appendPlainText("Debug Error: " + error);
}

void MainWindow::onBreakpointToggled(int line, bool added)
{
    if (m_debugController->isDebugging()) {
        if (added) {
            m_debugController->setBreakpoint(m_currentFile, line + 1);
        } else {
            m_debugController->removeBreakpoint(m_currentFile, line + 1);
        }
    }
}

// ============================================================
// Essential #7: Find and Replace
// ============================================================

void MainWindow::showFindDialog()
{
    if (!m_findReplaceDialog) {
        m_findReplaceDialog = new FindReplaceDialog(m_editor, this);
    }

    // If there's selected text, use it as the search term
    QTextCursor cursor = m_editor->textCursor();
    if (cursor.hasSelection()) {
        QString selected = cursor.selectedText();
        if (!selected.contains('\n') && selected.length() < 100) {
            m_findReplaceDialog->setFindText(selected);
        }
    }

    m_findReplaceDialog->showFind();
    m_findReplaceDialog->show();
    m_findReplaceDialog->raise();
    m_findReplaceDialog->activateWindow();
}

void MainWindow::showReplaceDialog()
{
    if (!m_findReplaceDialog) {
        m_findReplaceDialog = new FindReplaceDialog(m_editor, this);
    }

    // If there's selected text, use it as the search term
    QTextCursor cursor = m_editor->textCursor();
    if (cursor.hasSelection()) {
        QString selected = cursor.selectedText();
        if (!selected.contains('\n') && selected.length() < 100) {
            m_findReplaceDialog->setFindText(selected);
        }
    }

    m_findReplaceDialog->showReplace();
    m_findReplaceDialog->show();
    m_findReplaceDialog->raise();
    m_findReplaceDialog->activateWindow();
}

void MainWindow::findNext()
{
    if (m_findReplaceDialog) {
        // Trigger find next in dialog
        m_findReplaceDialog->show();
    } else {
        showFindDialog();
    }
}

void MainWindow::findPrevious()
{
    if (m_findReplaceDialog) {
        // Trigger find previous in dialog
        m_findReplaceDialog->show();
    } else {
        showFindDialog();
    }
}

void MainWindow::goToLine()
{
    bool ok;
    int line = QInputDialog::getInt(this, tr("Go to Line"),
                                    tr("Line number:"),
                                    m_editor->textCursor().blockNumber() + 1,
                                    1, m_editor->document()->blockCount(),
                                    1, &ok);
    if (ok) {
        QTextCursor cursor = m_editor->textCursor();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, line - 1);
        m_editor->setTextCursor(cursor);
        m_editor->centerCursor();
        m_editor->setFocus();
    }
}
