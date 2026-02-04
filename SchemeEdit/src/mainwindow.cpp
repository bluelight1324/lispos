#include "mainwindow.h"
#include "editor.h"
#include "schemerunner.h"

#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QTextStream>
#include <QFont>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_editor(new Editor(this))
    , m_outputPane(new QPlainTextEdit(this))
    , m_schemeRunner(new SchemeRunner(this))
    , m_isModified(false)
{
    setCentralWidget(m_editor);
    setWindowTitle("SchemeEdit");
    resize(1024, 768);

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

    // Connect editor signals
    connect(m_editor, &QPlainTextEdit::textChanged, this, &MainWindow::documentModified);

    // Connect Scheme runner signals
    connect(m_schemeRunner, &SchemeRunner::outputReady, this, [this](const QString &output) {
        m_outputPane->appendPlainText(output);
    });
    connect(m_schemeRunner, &SchemeRunner::errorOccurred, this, [this](const QString &error) {
        m_outputPane->appendPlainText("Error: " + error);
    });

    setCurrentFile(QString());
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

    // Scheme actions
    m_runAction = new QAction(tr("&Run"), this);
    m_runAction->setShortcut(QKeySequence(Qt::Key_F5));
    m_runAction->setStatusTip(tr("Run the Scheme code"));
    connect(m_runAction, &QAction::triggered, this, &MainWindow::runScheme);

    m_runSelectionAction = new QAction(tr("Run &Selection"), this);
    m_runSelectionAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return));
    m_runSelectionAction->setStatusTip(tr("Run selected Scheme code"));
    connect(m_runSelectionAction, &QAction::triggered, this, &MainWindow::runSelection);

    m_clearOutputAction = new QAction(tr("&Clear Output"), this);
    m_clearOutputAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));
    connect(m_clearOutputAction, &QAction::triggered, this, &MainWindow::clearOutput);

    // Help actions
    m_aboutAction = new QAction(tr("&About"), this);
    m_aboutAction->setStatusTip(tr("About SchemeEdit"));
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::about);

    m_aboutQtAction = new QAction(tr("About &Qt"), this);
    connect(m_aboutQtAction, &QAction::triggered, qApp, &QApplication::aboutQt);
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

    QMenu *schemeMenu = menuBar()->addMenu(tr("&Scheme"));
    schemeMenu->addAction(m_runAction);
    schemeMenu->addAction(m_runSelectionAction);
    schemeMenu->addSeparator();
    schemeMenu->addAction(m_clearOutputAction);

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
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
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
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
           "<p>A simple text editor with Scheme/Lisp support.</p>"
           "<p>Built with Qt and the LispOS Scheme compiler.</p>"
           "<p>Features:</p>"
           "<ul>"
           "<li>Syntax highlighting for Scheme</li>"
           "<li>Run Scheme code with F5</li>"
           "<li>Run selected code with Ctrl+Enter</li>"
           "<li>Integrated output pane</li>"
           "</ul>"));
}

void MainWindow::runScheme()
{
    QString code = m_editor->toPlainText();
    if (!code.isEmpty()) {
        m_outputPane->appendPlainText(">>> Running...\n");
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
