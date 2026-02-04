/*
 * findreplacedialog.h - Find and Replace Dialog
 *
 * Essential #7: Find and Replace functionality
 */

#ifndef FINDREPLACEDIALOG_H
#define FINDREPLACEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QPlainTextEdit>

class FindReplaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindReplaceDialog(QPlainTextEdit *editor, QWidget *parent = nullptr);

    void setFindText(const QString &text);
    void showFind();
    void showReplace();

signals:
    void findNext(const QString &text, bool caseSensitive, bool wholeWord);
    void findPrevious(const QString &text, bool caseSensitive, bool wholeWord);
    void replaceNext(const QString &findText, const QString &replaceText, bool caseSensitive, bool wholeWord);
    void replaceAll(const QString &findText, const QString &replaceText, bool caseSensitive, bool wholeWord);

private slots:
    void onFindNext();
    void onFindPrevious();
    void onReplace();
    void onReplaceAll();
    void onTextChanged(const QString &text);

private:
    void setupUi();
    bool doFind(bool forward);
    void highlightAllMatches();

    QPlainTextEdit *m_editor;

    QLineEdit *m_findEdit;
    QLineEdit *m_replaceEdit;
    QLabel *m_replaceLabel;

    QPushButton *m_findNextBtn;
    QPushButton *m_findPrevBtn;
    QPushButton *m_replaceBtn;
    QPushButton *m_replaceAllBtn;
    QPushButton *m_closeBtn;

    QCheckBox *m_caseSensitiveCheck;
    QCheckBox *m_wholeWordCheck;

    QLabel *m_statusLabel;

    bool m_showReplace;
};

#endif // FINDREPLACEDIALOG_H
