#include "schemerunner.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryFile>
#include <QTextStream>

SchemeRunner::SchemeRunner(QObject *parent)
    : QObject(parent)
    , m_process(new QProcess(this))
{
    m_interpreterPath = findInterpreter();

    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &SchemeRunner::onReadyReadStandardOutput);
    connect(m_process, &QProcess::readyReadStandardError,
            this, &SchemeRunner::onReadyReadStandardError);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &SchemeRunner::onProcessFinished);
}

SchemeRunner::~SchemeRunner()
{
    stop();
    // Clean up temp file if exists
    if (!m_tempFilePath.isEmpty() && QFile::exists(m_tempFilePath)) {
        QFile::remove(m_tempFilePath);
    }
}

QString SchemeRunner::findInterpreter() const
{
    // Check common locations relative to the application
    QStringList searchPaths = {
        // Relative to application directory
        QCoreApplication::applicationDirPath() + "/lisp.exe",
        QCoreApplication::applicationDirPath() + "/../lisp.exe",
        QCoreApplication::applicationDirPath() + "/../../LispCompiler/build/Release/lisp.exe",
        // Absolute paths (LispOS project)
        "I:/lispos/LispCompiler/build/Release/lisp.exe",
        "C:/lispos/LispCompiler/build/Release/lisp.exe",
        // In PATH
        "lisp.exe"
    };

    for (const QString &path : searchPaths) {
        if (QFileInfo::exists(path)) {
            return QFileInfo(path).absoluteFilePath();
        }
    }

    // Try to find in PATH
    QString pathEnv = qEnvironmentVariable("PATH");
    QStringList pathDirs = pathEnv.split(';', Qt::SkipEmptyParts);
    for (const QString &dir : pathDirs) {
        QString fullPath = QDir(dir).filePath("lisp.exe");
        if (QFileInfo::exists(fullPath)) {
            return fullPath;
        }
    }

    return QString();
}

void SchemeRunner::setInterpreterPath(const QString &path)
{
    m_interpreterPath = path;
}

QString SchemeRunner::interpreterPath() const
{
    return m_interpreterPath;
}

void SchemeRunner::run(const QString &code)
{
    if (m_interpreterPath.isEmpty()) {
        emit errorOccurred("Scheme interpreter not found. Please set the path to lisp.exe.");
        return;
    }

    if (m_process->state() != QProcess::NotRunning) {
        stop();
    }

    // Clean up previous temp file
    if (!m_tempFilePath.isEmpty() && QFile::exists(m_tempFilePath)) {
        QFile::remove(m_tempFilePath);
    }

    // Create temporary file with the Scheme code
    QTemporaryFile tempFile;
    tempFile.setFileTemplate(QDir::tempPath() + "/schemeedit_XXXXXX.scm");
    tempFile.setAutoRemove(false);

    if (!tempFile.open()) {
        emit errorOccurred("Failed to create temporary file for code execution.");
        return;
    }

    m_tempFilePath = tempFile.fileName();

    QTextStream out(&tempFile);
    out << code;
    tempFile.close();

    // Run the interpreter with the temp file
    m_process->start(m_interpreterPath, QStringList() << m_tempFilePath);

    if (!m_process->waitForStarted(5000)) {
        emit errorOccurred("Failed to start Scheme interpreter: " + m_process->errorString());
    }
}

void SchemeRunner::stop()
{
    if (m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(1000);
    }
}

void SchemeRunner::onReadyReadStandardOutput()
{
    QString output = QString::fromUtf8(m_process->readAllStandardOutput());
    if (!output.isEmpty()) {
        emit outputReady(output);
    }
}

void SchemeRunner::onReadyReadStandardError()
{
    QString error = QString::fromUtf8(m_process->readAllStandardError());
    if (!error.isEmpty()) {
        emit errorOccurred(error);
    }
}

void SchemeRunner::onProcessFinished(int exitCode, QProcess::ExitStatus /* exitStatus */)
{
    // Clean up temp file
    if (!m_tempFilePath.isEmpty() && QFile::exists(m_tempFilePath)) {
        QFile::remove(m_tempFilePath);
        m_tempFilePath.clear();
    }

    emit finished(exitCode);
}
