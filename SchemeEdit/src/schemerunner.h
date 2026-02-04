#ifndef SCHEMERUNNER_H
#define SCHEMERUNNER_H

#include <QObject>
#include <QProcess>
#include <QString>

class SchemeRunner : public QObject
{
    Q_OBJECT

public:
    explicit SchemeRunner(QObject *parent = nullptr);
    ~SchemeRunner();

    void run(const QString &code);
    void stop();

    void setInterpreterPath(const QString &path);
    QString interpreterPath() const;

signals:
    void outputReady(const QString &output);
    void errorOccurred(const QString &error);
    void finished(int exitCode);

private slots:
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QString findInterpreter() const;

    QProcess *m_process;
    QString m_interpreterPath;
    QString m_tempFilePath;
};

#endif // SCHEMERUNNER_H
