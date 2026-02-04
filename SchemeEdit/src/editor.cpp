#include "editor.h"
#include "schemehighlighter.h"

#include <QPainter>
#include <QTextBlock>
#include <QFont>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QScrollBar>

Editor::Editor(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_lineNumberArea(new LineNumberArea(this))
    , m_currentLineNumber(-1)
    , m_debugLine(-1)
{
    setupEditor();
}

void Editor::setupEditor()
{
    // Set monospace font - Rider-like
    QFont font("JetBrains Mono", 11);
    if (!QFontInfo(font).exactMatch()) {
        font = QFont("Consolas", 11);
    }
    if (!QFontInfo(font).exactMatch()) {
        font = QFont("Courier New", 11);
    }
    font.setFixedPitch(true);
    setFont(font);

    // Set tab width to 2 spaces (Lisp convention)
    QFontMetrics metrics(font);
    setTabStopDistance(2 * metrics.horizontalAdvance(' '));

    // Setup syntax highlighter
    m_highlighter = new SchemeHighlighter(document());

    // Initialize rainbow colors (Rider-inspired palette)
    m_rainbowColors[0] = QColor(255, 215, 0);    // Gold
    m_rainbowColors[1] = QColor(218, 112, 214);  // Orchid
    m_rainbowColors[2] = QColor(135, 206, 235);  // Sky Blue
    m_rainbowColors[3] = QColor(152, 251, 152);  // Pale Green
    m_rainbowColors[4] = QColor(255, 160, 122);  // Light Salmon
    m_rainbowColors[5] = QColor(176, 224, 230);  // Powder Blue

    // Connect signals
    connect(this, &QPlainTextEdit::blockCountChanged,
            this, &Editor::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest,
            this, &Editor::updateLineNumberArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged,
            this, &Editor::onCursorPositionChanged);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    // Rider-like dark theme
    setStyleSheet(
        "QPlainTextEdit {"
        "  background-color: #2b2b2b;"  /* Rider dark background */
        "  color: #a9b7c6;"             /* Rider text color */
        "  selection-background-color: #214283;"
        "  border: none;"
        "}"
    );

    // Set cursor width for better visibility
    setCursorWidth(2);
}

void Editor::keyPressEvent(QKeyEvent *event)
{
    // Auto-close brackets
    if (event->text() == "(") {
        autoCloseBracket('(');
        return;
    }
    if (event->text() == "[") {
        autoCloseBracket('[');
        return;
    }
    if (event->text() == "\"") {
        QTextCursor cursor = textCursor();
        cursor.insertText("\"\"");
        cursor.movePosition(QTextCursor::Left);
        setTextCursor(cursor);
        return;
    }

    // Auto-indent on Enter
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        autoIndent();
        return;
    }

    // Skip over closing bracket if typing it
    if (event->text() == ")" || event->text() == "]" || event->text() == "\"") {
        QTextCursor cursor = textCursor();
        if (!cursor.atEnd()) {
            QChar nextChar = document()->characterAt(cursor.position());
            if (nextChar == event->text().at(0)) {
                cursor.movePosition(QTextCursor::Right);
                setTextCursor(cursor);
                return;
            }
        }
    }

    QPlainTextEdit::keyPressEvent(event);
}

void Editor::autoCloseBracket(QChar bracket)
{
    QTextCursor cursor = textCursor();
    QChar closeBracket = (bracket == '(') ? ')' : ']';
    cursor.insertText(QString(bracket) + QString(closeBracket));
    cursor.movePosition(QTextCursor::Left);
    setTextCursor(cursor);
}

void Editor::autoIndent()
{
    QTextCursor cursor = textCursor();
    QString currentLine = cursor.block().text();

    // Count leading spaces
    int indent = 0;
    for (QChar c : currentLine) {
        if (c == ' ') indent++;
        else if (c == '\t') indent += 2;
        else break;
    }

    // Count unclosed parentheses on current line
    int parenBalance = 0;
    for (QChar c : currentLine) {
        if (c == '(' || c == '[') parenBalance++;
        else if (c == ')' || c == ']') parenBalance--;
    }

    // Add extra indent for unclosed parens
    if (parenBalance > 0) {
        indent += 2;
    }

    // Insert newline with proper indentation
    cursor.insertText("\n" + QString(indent, ' '));
    setTextCursor(cursor);
}

void Editor::onCursorPositionChanged()
{
    highlightCurrentLine();
    highlightMatchingBrackets();
    m_lineNumberArea->update();
}

int Editor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    // Add space for line number + some padding
    int space = 15 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * qMax(digits, 3);
    return space;
}

void Editor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void Editor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy) {
        m_lineNumberArea->scroll(0, dy);
    } else {
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());
    }

    if (rect.contains(viewport()->rect())) {
        updateLineNumberAreaWidth(0);
    }
}

void Editor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);

    QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(),
                                        lineNumberAreaWidth(), cr.height()));
}

void Editor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        // Rider-like current line highlight (subtle)
        QColor lineColor = QColor(50, 50, 50);
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    m_currentLineNumber = textCursor().blockNumber();

    setExtraSelections(extraSelections);
}

void Editor::highlightMatchingBrackets()
{
    QList<QTextEdit::ExtraSelection> selections = extraSelections();

    // Remove old bracket highlights (keep current line highlight which is first)
    while (selections.size() > 1) {
        selections.removeLast();
    }

    QTextCursor cursor = textCursor();
    int pos = cursor.position();
    QChar charAtCursor = document()->characterAt(pos);
    QChar charBefore = (pos > 0) ? document()->characterAt(pos - 1) : QChar();

    int matchPos = -1;
    int bracketPos = -1;

    // Check character at cursor position
    if (charAtCursor == '(' || charAtCursor == '[') {
        bracketPos = pos;
        matchPos = findMatchingBracket(pos, charAtCursor,
                                       charAtCursor == '(' ? ')' : ']', true);
    } else if (charAtCursor == ')' || charAtCursor == ']') {
        bracketPos = pos;
        matchPos = findMatchingBracket(pos, charAtCursor,
                                       charAtCursor == ')' ? '(' : '[', false);
    }
    // Check character before cursor
    else if (charBefore == '(' || charBefore == '[') {
        bracketPos = pos - 1;
        matchPos = findMatchingBracket(pos - 1, charBefore,
                                       charBefore == '(' ? ')' : ']', true);
    } else if (charBefore == ')' || charBefore == ']') {
        bracketPos = pos - 1;
        matchPos = findMatchingBracket(pos - 1, charBefore,
                                       charBefore == ')' ? '(' : '[', false);
    }

    if (matchPos >= 0 && bracketPos >= 0) {
        // Highlight both brackets
        QTextEdit::ExtraSelection sel1, sel2;

        // Calculate nesting depth for rainbow color
        int depth = 0;
        int startPos = qMin(bracketPos, matchPos);
        for (int i = 0; i < startPos; i++) {
            QChar c = document()->characterAt(i);
            if (c == '(' || c == '[') depth++;
            else if (c == ')' || c == ']') depth--;
        }
        QColor bracketColor = m_rainbowColors[depth % RAINBOW_COLORS_COUNT];

        // First bracket
        sel1.format.setForeground(bracketColor);
        sel1.format.setFontWeight(QFont::Bold);
        sel1.format.setBackground(QColor(60, 60, 60));
        sel1.cursor = textCursor();
        sel1.cursor.setPosition(bracketPos);
        sel1.cursor.setPosition(bracketPos + 1, QTextCursor::KeepAnchor);

        // Matching bracket
        sel2.format.setForeground(bracketColor);
        sel2.format.setFontWeight(QFont::Bold);
        sel2.format.setBackground(QColor(60, 60, 60));
        sel2.cursor = textCursor();
        sel2.cursor.setPosition(matchPos);
        sel2.cursor.setPosition(matchPos + 1, QTextCursor::KeepAnchor);

        selections.append(sel1);
        selections.append(sel2);
    }

    setExtraSelections(selections);
}

int Editor::findMatchingBracket(int pos, QChar openChar, QChar closeChar, bool forward)
{
    int depth = 1;
    int docLength = document()->characterCount();

    if (forward) {
        for (int i = pos + 1; i < docLength && depth > 0; i++) {
            QChar c = document()->characterAt(i);
            if (c == openChar) depth++;
            else if (c == closeChar) depth--;
            if (depth == 0) return i;
        }
    } else {
        for (int i = pos - 1; i >= 0 && depth > 0; i--) {
            QChar c = document()->characterAt(i);
            if (c == openChar) depth++;
            else if (c == closeChar) depth--;
            if (depth == 0) return i;
        }
    }

    return -1;
}

void Editor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(m_lineNumberArea);

    // Rider-like gutter background
    painter.fillRect(event->rect(), QColor(49, 51, 53));

    // Draw separator line
    painter.setPen(QColor(60, 63, 65));
    painter.drawLine(m_lineNumberArea->width() - 1, event->rect().top(),
                     m_lineNumberArea->width() - 1, event->rect().bottom());

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    QFont normalFont = font();
    QFont currentFont = font();
    currentFont.setBold(true);

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            int lineHeight = fontMetrics().height();
            int centerY = top + lineHeight / 2;

            // Draw breakpoint marker (red circle)
            if (m_breakpoints.contains(blockNumber)) {
                painter.setBrush(QColor(220, 50, 50));  // Red
                painter.setPen(Qt::NoPen);
                painter.drawEllipse(QPoint(8, centerY), 5, 5);
            }

            // Draw debug execution line marker (yellow arrow)
            if (blockNumber == m_debugLine) {
                painter.setBrush(QColor(255, 255, 0));  // Yellow
                painter.setPen(Qt::NoPen);
                QPolygon arrow;
                arrow << QPoint(4, centerY - 4)
                      << QPoint(12, centerY)
                      << QPoint(4, centerY + 4);
                painter.drawPolygon(arrow);
            }

            // Current line number is highlighted (Rider style)
            if (blockNumber == m_currentLineNumber) {
                painter.setFont(currentFont);
                painter.setPen(QColor(255, 255, 255));  // White for current line

                // Draw current line indicator (only if not debug line)
                if (blockNumber != m_debugLine && !m_breakpoints.contains(blockNumber)) {
                    int indicatorY = top + (lineHeight - 6) / 2;
                    painter.fillRect(2, indicatorY, 4, 6, QColor(255, 204, 0));  // Yellow bar
                }
            } else {
                painter.setFont(normalFont);
                painter.setPen(QColor(128, 128, 128));  // Gray for other lines
            }

            painter.drawText(0, top, m_lineNumberArea->width() - 10, lineHeight,
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

// Breakpoint management
void Editor::toggleBreakpoint(int line)
{
    if (m_breakpoints.contains(line)) {
        m_breakpoints.remove(line);
        emit breakpointToggled(line, false);
    } else {
        m_breakpoints.insert(line);
        emit breakpointToggled(line, true);
    }
    m_lineNumberArea->update();
}

bool Editor::hasBreakpoint(int line) const
{
    return m_breakpoints.contains(line);
}

QList<int> Editor::getBreakpoints() const
{
    return m_breakpoints.values();
}

void Editor::clearAllBreakpoints()
{
    m_breakpoints.clear();
    m_lineNumberArea->update();
}

// Debug line indicator
void Editor::setDebugLine(int line)
{
    m_debugLine = line;
    m_lineNumberArea->update();

    // Scroll to debug line if needed
    if (line >= 0) {
        QTextBlock block = document()->findBlockByLineNumber(line);
        if (block.isValid()) {
            QTextCursor cursor(block);
            setTextCursor(cursor);
            centerCursor();
        }
    }
}

void Editor::clearDebugLine()
{
    m_debugLine = -1;
    m_lineNumberArea->update();
}

// Handle clicks in the line number area for breakpoint toggling
void Editor::lineNumberAreaMousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // Find which line was clicked
        QTextBlock block = firstVisibleBlock();
        int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());

        while (block.isValid()) {
            int bottom = top + qRound(blockBoundingRect(block).height());

            if (event->pos().y() >= top && event->pos().y() < bottom) {
                toggleBreakpoint(block.blockNumber());
                break;
            }

            block = block.next();
            top = bottom;
        }
    }
}
