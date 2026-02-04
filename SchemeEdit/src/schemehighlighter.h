#ifndef SCHEMEHIGHLIGHTER_H
#define SCHEMEHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

class SchemeHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit SchemeHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> m_highlightingRules;

    QTextCharFormat m_keywordFormat;
    QTextCharFormat m_builtinFormat;
    QTextCharFormat m_stringFormat;
    QTextCharFormat m_numberFormat;
    QTextCharFormat m_commentFormat;
    QTextCharFormat m_booleanFormat;
    QTextCharFormat m_parenFormat;
    QTextCharFormat m_quoteFormat;
};

#endif // SCHEMEHIGHLIGHTER_H
