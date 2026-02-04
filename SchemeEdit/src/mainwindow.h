#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QDockWidget>
#include <QProcess>

class Editor;
class SchemeRunner;

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

    void runScheme();
    void runSelection();
    void clearOutput();

    void documentModified();
    void updateWindowTitle();

private:
    void createActions();
    void createMenus();
    void createToolBar();
    void createStatusBar();
    void createDockWidgets();

    void loadFile(const QString &fileName);
    bool saveToFile(const QString &fileName);
    bool maybeSave();
    void setCurrentFile(const QString &fileName);

    Editor *m_editor;
    QPlainTextEdit *m_outputPane;
    QDockWidget *m_outputDock;
    SchemeRunner *m_schemeRunner;

    QString m_currentFile;
    bool m_isModified;

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

    QAction *m_runAction;
    QAction *m_runSelectionAction;
    QAction *m_clearOutputAction;

    QAction *m_aboutAction;
    QAction *m_aboutQtAction;
};

#endif // MAINWINDOW_H
