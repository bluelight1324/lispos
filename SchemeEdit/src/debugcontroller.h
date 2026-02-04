#ifndef DEBUGCONTROLLER_H
#define DEBUGCONTROLLER_H

#include <QObject>
#include <QProcess>
#include <QJsonObject>
#include <QVector>

// Stack frame information
struct StackFrame {
    int id;
    QString name;
    QString file;
    int line;
};

// Variable information
struct Variable {
    QString name;
    QString value;
    QString type;
};

class DebugController : public QObject
{
    Q_OBJECT

public:
    explicit DebugController(QObject *parent = nullptr);
    ~DebugController();

    // State queries
    bool isDebugging() const { return m_isDebugging; }
    bool isPaused() const { return m_isPaused; }
    QString currentFile() const { return m_currentFile; }
    int currentLine() const { return m_currentLine; }

    // Debugging control
    void startDebugging(const QString &file, const QString &compilerPath);
    void stopDebugging();

    void continueExecution();
    void stepInto();
    void stepOver();
    void stepOut();
    void pause();

    // Breakpoint management
    void setBreakpoint(const QString &file, int line);
    void removeBreakpoint(const QString &file, int line);
    void clearAllBreakpoints();

    // Inspection
    void requestStackTrace();
    void requestVariables(int frameIndex = 0);
    void evaluateExpression(const QString &expression, int frameIndex = 0);

signals:
    void debuggingStarted();
    void debuggingStopped();
    void paused(const QString &file, int line, const QString &reason);
    void continued();

    void stackTraceReceived(const QVector<StackFrame> &frames);
    void variablesReceived(const QVector<Variable> &variables);
    void evaluationResult(const QString &expression, const QString &result);

    void outputReceived(const QString &output);
    void errorReceived(const QString &error);

private slots:
    void onProcessStarted();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();

private:
    void sendCommand(const QJsonObject &command);
    void handleResponse(const QJsonObject &response);
    void handleEvent(const QJsonObject &event);
    void parseJsonOutput(const QString &line);

    QProcess *m_process;
    bool m_isDebugging;
    bool m_isPaused;
    QString m_currentFile;
    int m_currentLine;
    QString m_outputBuffer;
};

#endif // DEBUGCONTROLLER_H
