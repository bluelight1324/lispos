#include "schemehighlighter.h"

SchemeHighlighter::SchemeHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    // Keywords (special forms)
    m_keywordFormat.setForeground(QColor(197, 134, 192)); // Purple
    m_keywordFormat.setFontWeight(QFont::Bold);
    const QString keywordPatterns[] = {
        QStringLiteral("\\bdefine\\b"),
        QStringLiteral("\\blambda\\b"),
        QStringLiteral("\\bif\\b"),
        QStringLiteral("\\bcond\\b"),
        QStringLiteral("\\belse\\b"),
        QStringLiteral("\\blet\\b"),
        QStringLiteral("\\blet\\*\\b"),
        QStringLiteral("\\bletrec\\b"),
        QStringLiteral("\\bbegin\\b"),
        QStringLiteral("\\bdo\\b"),
        QStringLiteral("\\bcase\\b"),
        QStringLiteral("\\bwhen\\b"),
        QStringLiteral("\\bunless\\b"),
        QStringLiteral("\\band\\b"),
        QStringLiteral("\\bor\\b"),
        QStringLiteral("\\bnot\\b"),
        QStringLiteral("\\bset!\\b"),
        QStringLiteral("\\bquote\\b"),
        QStringLiteral("\\bquasiquote\\b"),
        QStringLiteral("\\bunquote\\b"),
        QStringLiteral("\\bunquote-splicing\\b"),
        QStringLiteral("\\bcase-lambda\\b"),
        QStringLiteral("\\bsyntax-rules\\b"),
        QStringLiteral("\\bdefine-syntax\\b"),
        QStringLiteral("\\blet-syntax\\b"),
        QStringLiteral("\\bletrec-syntax\\b"),
    };
    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = m_keywordFormat;
        m_highlightingRules.append(rule);
    }

    // Built-in functions
    m_builtinFormat.setForeground(QColor(220, 220, 170)); // Yellow
    const QString builtinPatterns[] = {
        // List operations
        QStringLiteral("\\bcar\\b"), QStringLiteral("\\bcdr\\b"),
        QStringLiteral("\\bcons\\b"), QStringLiteral("\\blist\\b"),
        QStringLiteral("\\bappend\\b"), QStringLiteral("\\breverse\\b"),
        QStringLiteral("\\blength\\b"), QStringLiteral("\\bmap\\b"),
        QStringLiteral("\\bfilter\\b"), QStringLiteral("\\bfold\\b"),
        QStringLiteral("\\bfor-each\\b"), QStringLiteral("\\bmember\\b"),
        QStringLiteral("\\bassoc\\b"), QStringLiteral("\\bassq\\b"),
        QStringLiteral("\\bmemq\\b"),
        // Type predicates
        QStringLiteral("\\bnull\\?\\b"), QStringLiteral("\\bpair\\?\\b"),
        QStringLiteral("\\blist\\?\\b"), QStringLiteral("\\bnumber\\?\\b"),
        QStringLiteral("\\bstring\\?\\b"), QStringLiteral("\\bsymbol\\?\\b"),
        QStringLiteral("\\bboolean\\?\\b"), QStringLiteral("\\bprocedure\\?\\b"),
        QStringLiteral("\\bvector\\?\\b"), QStringLiteral("\\bchar\\?\\b"),
        QStringLiteral("\\binteger\\?\\b"), QStringLiteral("\\breal\\?\\b"),
        QStringLiteral("\\bzero\\?\\b"), QStringLiteral("\\bpositive\\?\\b"),
        QStringLiteral("\\bnegative\\?\\b"), QStringLiteral("\\bodd\\?\\b"),
        QStringLiteral("\\beven\\?\\b"), QStringLiteral("\\beq\\?\\b"),
        QStringLiteral("\\beqv\\?\\b"), QStringLiteral("\\bequal\\?\\b"),
        // I/O
        QStringLiteral("\\bdisplay\\b"), QStringLiteral("\\bnewline\\b"),
        QStringLiteral("\\bread\\b"), QStringLiteral("\\bwrite\\b"),
        QStringLiteral("\\bprint\\b"),
        // Math
        QStringLiteral("\\babs\\b"), QStringLiteral("\\bmin\\b"),
        QStringLiteral("\\bmax\\b"), QStringLiteral("\\bsqrt\\b"),
        QStringLiteral("\\bexpt\\b"), QStringLiteral("\\bmodulo\\b"),
        QStringLiteral("\\bremainder\\b"), QStringLiteral("\\bquotient\\b"),
        QStringLiteral("\\bfloor\\b"), QStringLiteral("\\bceiling\\b"),
        QStringLiteral("\\bround\\b"), QStringLiteral("\\btruncate\\b"),
        // Vector operations
        QStringLiteral("\\bmake-vector\\b"), QStringLiteral("\\bvector\\b"),
        QStringLiteral("\\bvector-length\\b"), QStringLiteral("\\bvector-ref\\b"),
        QStringLiteral("\\bvector-set!\\b"),
        // String operations
        QStringLiteral("\\bstring-length\\b"), QStringLiteral("\\bstring-ref\\b"),
        QStringLiteral("\\bstring-append\\b"), QStringLiteral("\\bsubstring\\b"),
        QStringLiteral("\\bstring->list\\b"), QStringLiteral("\\blist->string\\b"),
        QStringLiteral("\\bnumber->string\\b"), QStringLiteral("\\bstring->number\\b"),
        // Apply/call
        QStringLiteral("\\bapply\\b"), QStringLiteral("\\bcall/cc\\b"),
        QStringLiteral("\\bcall-with-current-continuation\\b"),
        QStringLiteral("\\bvalues\\b"), QStringLiteral("\\bcall-with-values\\b"),
    };
    for (const QString &pattern : builtinPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = m_builtinFormat;
        m_highlightingRules.append(rule);
    }

    // Booleans
    m_booleanFormat.setForeground(QColor(86, 156, 214)); // Blue
    m_booleanFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(QStringLiteral("#[tf]\\b"));
    rule.format = m_booleanFormat;
    m_highlightingRules.append(rule);

    // Numbers
    m_numberFormat.setForeground(QColor(181, 206, 168)); // Light green
    rule.pattern = QRegularExpression(QStringLiteral("\\b[+-]?\\d+\\.?\\d*\\b"));
    rule.format = m_numberFormat;
    m_highlightingRules.append(rule);

    // Character literals
    rule.pattern = QRegularExpression(QStringLiteral("#\\\\(newline|space|tab|\\S)"));
    rule.format = m_numberFormat;
    m_highlightingRules.append(rule);

    // Strings
    m_stringFormat.setForeground(QColor(206, 145, 120)); // Orange/brown
    rule.pattern = QRegularExpression(QStringLiteral("\"([^\"\\\\]|\\\\.)*\""));
    rule.format = m_stringFormat;
    m_highlightingRules.append(rule);

    // Quote, quasiquote, unquote
    m_quoteFormat.setForeground(QColor(78, 201, 176)); // Teal
    rule.pattern = QRegularExpression(QStringLiteral("[`',](?=[\\(\\[])"));
    rule.format = m_quoteFormat;
    m_highlightingRules.append(rule);

    rule.pattern = QRegularExpression(QStringLiteral(",@"));
    rule.format = m_quoteFormat;
    m_highlightingRules.append(rule);

    // Comments (must be last to override other rules)
    m_commentFormat.setForeground(QColor(106, 153, 85)); // Green
    m_commentFormat.setFontItalic(true);
    rule.pattern = QRegularExpression(QStringLiteral(";[^\n]*"));
    rule.format = m_commentFormat;
    m_highlightingRules.append(rule);
}

void SchemeHighlighter::highlightBlock(const QString &text)
{
    for (const HighlightingRule &rule : qAsConst(m_highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}
