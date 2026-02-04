#ifndef EDITOR_H
#define EDITOR_H

#include <QPlainTextEdit>
#include <QTimer>
#include <QSet>

class LineNumberArea;
class SchemeHighlighter;

class Editor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit Editor(QWidget *parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

    // Bracket matching
    void highlightMatchingBrackets();
    int findMatchingBracket(int pos, QChar openChar, QChar closeChar, bool forward);

    // Breakpoint management
    void toggleBreakpoint(int line);
    bool hasBreakpoint(int line) const;
    QList<int> getBreakpoints() const;
    void clearAllBreakpoints();

    // Debug line indicator
    void setDebugLine(int line);
    void clearDebugLine();
    int debugLine() const { return m_debugLine; }

    // Line number area click handling
    void lineNumberAreaMousePressEvent(QMouseEvent *event);

signals:
    void breakpointToggled(int line, bool added);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);
    void onCursorPositionChanged();

private:
    void setupEditor();
    void autoIndent();
    void autoCloseBracket(QChar bracket);

    QWidget *m_lineNumberArea;
    SchemeHighlighter *m_highlighter;
    int m_currentLineNumber;
    int m_debugLine;  // Current debug execution line (-1 if not debugging)

    // Breakpoints (line numbers with breakpoints)
    QSet<int> m_breakpoints;

    // Rainbow parentheses colors
    static const int RAINBOW_COLORS_COUNT = 6;
    QColor m_rainbowColors[RAINBOW_COLORS_COUNT];
};

class LineNumberArea : public QWidget
{
public:
    explicit LineNumberArea(Editor *editor) : QWidget(editor), m_editor(editor) {}

    QSize sizeHint() const override
    {
        return QSize(m_editor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        m_editor->lineNumberAreaPaintEvent(event);
    }

    void mousePressEvent(QMouseEvent *event) override
    {
        m_editor->lineNumberAreaMousePressEvent(event);
    }

private:
    Editor *m_editor;
};

#endif // EDITOR_H
