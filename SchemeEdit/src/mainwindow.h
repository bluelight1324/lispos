#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QDockWidget>
#include <QProcess>
#include <QLabel>
#include <QListWidget>
#include <QTreeWidget>

class Editor;
class SchemeRunner;
class DebugController;
class FindReplaceDialog;
struct StackFrame;
struct Variable;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void newFile();
    void openFile();
    bool saveFile();
    bool saveFileAs();
    void about();
    void showHelp();

    void runScheme();
    void runSelection();
    void compileScheme();
    void compileAndRun();
    void clearOutput();

    // Debug slots
    void startDebugging();
    void stopDebugging();
    void debugContinue();
    void debugStepInto();
    void debugStepOver();
    void debugStepOut();
    void debugToggleBreakpoint();

    void onDebuggingStarted();
    void onDebuggingStopped();
    void onDebugPaused(const QString &file, int line, const QString &reason);
    void onDebugContinued();
    void onStackTraceReceived(const QVector<StackFrame> &frames);
    void onVariablesReceived(const QVector<Variable> &variables);
    void onDebugOutput(const QString &output);
    void onDebugError(const QString &error);
    void onBreakpointToggled(int line, bool added);

    void loadExample(const QString &examplePath);

    // Find/Replace slots
    void showFindDialog();
    void showReplaceDialog();
    void findNext();
    void findPrevious();
    void goToLine();

    void documentModified();
    void updateWindowTitle();
    void onRunFinished(int exitCode);

private:
    void createActions();
    void createMenus();
    void createToolBar();
    void createStatusBar();
    void createDockWidgets();
    void createExamplesMenu();
    void createDebugPanels();

    void loadFile(const QString &fileName);
    bool saveToFile(const QString &fileName);
    bool maybeSave();
    void setCurrentFile(const QString &fileName);
    QString findCompiler() const;
    QString findExamplesDir() const;

    void updateDebugActions(bool debugging, bool paused);

    Editor *m_editor;
    QPlainTextEdit *m_outputPane;
    QDockWidget *m_outputDock;
    SchemeRunner *m_schemeRunner;
    QLabel *m_statusLabel;

    // Debug components
    DebugController *m_debugController;
    QDockWidget *m_callStackDock;
    QDockWidget *m_variablesDock;
    QListWidget *m_callStackList;
    QTreeWidget *m_variablesTree;

    QString m_currentFile;
    bool m_isModified;
    QString m_compilerPath;
    FindReplaceDialog *m_findReplaceDialog;

    // Actions
    QAction *m_newAction;
    QAction *m_openAction;
    QAction *m_saveAction;
    QAction *m_saveAsAction;
    QAction *m_exitAction;

    QAction *m_undoAction;
    QAction *m_redoAction;
    QAction *m_cutAction;
    QAction *m_copyAction;
    QAction *m_pasteAction;
    QAction *m_selectAllAction;
    QAction *m_findAction;
    QAction *m_replaceAction;
    QAction *m_findNextAction;
    QAction *m_findPrevAction;
    QAction *m_goToLineAction;

    QAction *m_runAction;
    QAction *m_runSelectionAction;
    QAction *m_compileAction;
    QAction *m_compileRunAction;
    QAction *m_clearOutputAction;

    // Debug actions
    QAction *m_debugStartAction;
    QAction *m_debugStopAction;
    QAction *m_debugContinueAction;
    QAction *m_debugStepIntoAction;
    QAction *m_debugStepOverAction;
    QAction *m_debugStepOutAction;
    QAction *m_debugToggleBreakpointAction;

    QAction *m_helpAction;
    QAction *m_aboutAction;
    QAction *m_aboutQtAction;

    QMenu *m_examplesMenu;
    QMenu *m_debugMenu;
    QToolBar *m_debugToolBar;
};

#endif // MAINWINDOW_H
