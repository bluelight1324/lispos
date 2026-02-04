#include "debugcontroller.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QFileInfo>
#include <QDebug>

DebugController::DebugController(QObject *parent)
    : QObject(parent)
    , m_process(nullptr)
    , m_isDebugging(false)
    , m_isPaused(false)
    , m_currentLine(0)
{
}

DebugController::~DebugController()
{
    stopDebugging();
}

void DebugController::startDebugging(const QString &file, const QString &compilerPath)
{
    if (m_isDebugging) {
        stopDebugging();
    }

    m_process = new QProcess(this);

    connect(m_process, &QProcess::started,
            this, &DebugController::onProcessStarted);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &DebugController::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred,
            this, &DebugController::onProcessError);
    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &DebugController::onReadyReadStandardOutput);
    connect(m_process, &QProcess::readyReadStandardError,
            this, &DebugController::onReadyReadStandardError);

    m_currentFile = file;
    m_currentLine = 1;
    m_outputBuffer.clear();

    // Start with --debug-json flag for IDE integration
    QStringList args;
    args << "--debug-json" << file;

    m_process->start(compilerPath, args);
}

void DebugController::stopDebugging()
{
    if (m_process) {
        if (m_process->state() != QProcess::NotRunning) {
            m_process->terminate();
            if (!m_process->waitForFinished(3000)) {
                m_process->kill();
            }
        }
        m_process->deleteLater();
        m_process = nullptr;
    }

    m_isDebugging = false;
    m_isPaused = false;
    emit debuggingStopped();
}

void DebugController::continueExecution()
{
    QJsonObject cmd;
    cmd["type"] = "request";
    cmd["command"] = "continue";
    sendCommand(cmd);
    m_isPaused = false;
    emit continued();
}

void DebugController::stepInto()
{
    QJsonObject cmd;
    cmd["type"] = "request";
    cmd["command"] = "stepIn";
    sendCommand(cmd);
    m_isPaused = false;
}

void DebugController::stepOver()
{
    QJsonObject cmd;
    cmd["type"] = "request";
    cmd["command"] = "stepOver";
    sendCommand(cmd);
    m_isPaused = false;
}

void DebugController::stepOut()
{
    QJsonObject cmd;
    cmd["type"] = "request";
    cmd["command"] = "stepOut";
    sendCommand(cmd);
    m_isPaused = false;
}

void DebugController::pause()
{
    // Send break signal
    QJsonObject cmd;
    cmd["type"] = "request";
    cmd["command"] = "pause";
    sendCommand(cmd);
}

void DebugController::setBreakpoint(const QString &file, int line)
{
    QJsonObject cmd;
    cmd["type"] = "request";
    cmd["command"] = "setBreakpoint";

    QJsonObject args;
    args["file"] = file;
    args["line"] = line;
    cmd["arguments"] = args;

    sendCommand(cmd);
}

void DebugController::removeBreakpoint(const QString &file, int line)
{
    QJsonObject cmd;
    cmd["type"] = "request";
    cmd["command"] = "removeBreakpoint";

    QJsonObject args;
    args["file"] = file;
    args["line"] = line;
    cmd["arguments"] = args;

    sendCommand(cmd);
}

void DebugController::clearAllBreakpoints()
{
    QJsonObject cmd;
    cmd["type"] = "request";
    cmd["command"] = "clearBreakpoints";
    sendCommand(cmd);
}

void DebugController::requestStackTrace()
{
    QJsonObject cmd;
    cmd["type"] = "request";
    cmd["command"] = "stackTrace";
    sendCommand(cmd);
}

void DebugController::requestVariables(int frameIndex)
{
    QJsonObject cmd;
    cmd["type"] = "request";
    cmd["command"] = "variables";

    QJsonObject args;
    args["frameId"] = frameIndex;
    cmd["arguments"] = args;

    sendCommand(cmd);
}

void DebugController::evaluateExpression(const QString &expression, int frameIndex)
{
    QJsonObject cmd;
    cmd["type"] = "request";
    cmd["command"] = "evaluate";

    QJsonObject args;
    args["expression"] = expression;
    args["frameId"] = frameIndex;
    cmd["arguments"] = args;

    sendCommand(cmd);
}

void DebugController::sendCommand(const QJsonObject &command)
{
    if (!m_process || m_process->state() != QProcess::Running) {
        return;
    }

    QJsonDocument doc(command);
    QString jsonStr = doc.toJson(QJsonDocument::Compact) + "\n";
    m_process->write(jsonStr.toUtf8());
}

void DebugController::onProcessStarted()
{
    m_isDebugging = true;
    m_isPaused = true;  // Starts paused at entry
    emit debuggingStarted();
}

void DebugController::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode)
    Q_UNUSED(exitStatus)

    m_isDebugging = false;
    m_isPaused = false;
    emit debuggingStopped();
}

void DebugController::onProcessError(QProcess::ProcessError error)
{
    QString errorMsg;
    switch (error) {
        case QProcess::FailedToStart:
            errorMsg = "Failed to start debugger";
            break;
        case QProcess::Crashed:
            errorMsg = "Debugger crashed";
            break;
        case QProcess::WriteError:
            errorMsg = "Write error communicating with debugger";
            break;
        case QProcess::ReadError:
            errorMsg = "Read error communicating with debugger";
            break;
        default:
            errorMsg = "Unknown debugger error";
            break;
    }
    emit errorReceived(errorMsg);
}

void DebugController::onReadyReadStandardOutput()
{
    m_outputBuffer += QString::fromUtf8(m_process->readAllStandardOutput());

    // Process complete lines
    int newlinePos;
    while ((newlinePos = m_outputBuffer.indexOf('\n')) != -1) {
        QString line = m_outputBuffer.left(newlinePos).trimmed();
        m_outputBuffer = m_outputBuffer.mid(newlinePos + 1);

        if (!line.isEmpty()) {
            if (line.startsWith('{')) {
                parseJsonOutput(line);
            } else {
                emit outputReceived(line);
            }
        }
    }
}

void DebugController::onReadyReadStandardError()
{
    QString error = QString::fromUtf8(m_process->readAllStandardError());
    emit errorReceived(error);
}

void DebugController::parseJsonOutput(const QString &line)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        // Not valid JSON, treat as regular output
        emit outputReceived(line);
        return;
    }

    QJsonObject obj = doc.object();
    QString type = obj["type"].toString();

    if (type == "response") {
        handleResponse(obj);
    } else if (type == "event") {
        handleEvent(obj);
    }
}

void DebugController::handleResponse(const QJsonObject &response)
{
    QString command = response["command"].toString();

    if (command == "stackTrace") {
        QVector<StackFrame> frames;
        QJsonArray stackFrames = response["body"].toObject()["stackFrames"].toArray();

        for (const QJsonValue &val : stackFrames) {
            QJsonObject frameObj = val.toObject();
            StackFrame frame;
            frame.id = frameObj["id"].toInt();
            frame.name = frameObj["name"].toString();
            frame.file = frameObj["file"].toString();
            frame.line = frameObj["line"].toInt();
            frames.append(frame);
        }

        emit stackTraceReceived(frames);
    }
    else if (command == "variables") {
        QVector<Variable> variables;
        QJsonArray vars = response["body"].toObject()["variables"].toArray();

        for (const QJsonValue &val : vars) {
            QJsonObject varObj = val.toObject();
            Variable var;
            var.name = varObj["name"].toString();
            var.value = varObj["value"].toString();
            var.type = varObj["type"].toString();
            variables.append(var);
        }

        emit variablesReceived(variables);
    }
    else if (command == "evaluate") {
        QString expression = response["body"].toObject()["expression"].toString();
        QString result = response["body"].toObject()["result"].toString();
        emit evaluationResult(expression, result);
    }
}

void DebugController::handleEvent(const QJsonObject &event)
{
    QString eventType = event["event"].toString();
    QJsonObject body = event["body"].toObject();

    if (eventType == "stopped") {
        QString reason = body["reason"].toString();
        QString file = body["file"].toString();
        int line = body["line"].toInt();

        m_currentFile = file;
        m_currentLine = line;
        m_isPaused = true;

        emit paused(file, line, reason);

        // Auto-request stack trace when stopped
        requestStackTrace();
    }
    else if (eventType == "continued") {
        m_isPaused = false;
        emit continued();
    }
    else if (eventType == "terminated") {
        stopDebugging();
    }
    else if (eventType == "output") {
        QString output = body["output"].toString();
        emit outputReceived(output);
    }
}
