/*
 * findreplacedialog.cpp - Find and Replace Dialog Implementation
 *
 * Essential #7: Find and Replace functionality
 */

#include "findreplacedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTextCursor>
#include <QTextDocument>
#include <QMessageBox>

FindReplaceDialog::FindReplaceDialog(QPlainTextEdit *editor, QWidget *parent)
    : QDialog(parent)
    , m_editor(editor)
    , m_showReplace(false)
{
    setWindowTitle(tr("Find"));
    setupUi();
}

void FindReplaceDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Find row
    QHBoxLayout *findLayout = new QHBoxLayout();
    QLabel *findLabel = new QLabel(tr("Find:"), this);
    findLabel->setFixedWidth(60);
    m_findEdit = new QLineEdit(this);
    m_findEdit->setPlaceholderText(tr("Enter search text..."));
    findLayout->addWidget(findLabel);
    findLayout->addWidget(m_findEdit);
    mainLayout->addLayout(findLayout);

    // Replace row
    QHBoxLayout *replaceLayout = new QHBoxLayout();
    m_replaceLabel = new QLabel(tr("Replace:"), this);
    m_replaceLabel->setFixedWidth(60);
    m_replaceEdit = new QLineEdit(this);
    m_replaceEdit->setPlaceholderText(tr("Enter replacement text..."));
    replaceLayout->addWidget(m_replaceLabel);
    replaceLayout->addWidget(m_replaceEdit);
    mainLayout->addLayout(replaceLayout);

    // Options
    QHBoxLayout *optionsLayout = new QHBoxLayout();
    m_caseSensitiveCheck = new QCheckBox(tr("Case sensitive"), this);
    m_wholeWordCheck = new QCheckBox(tr("Whole word"), this);
    optionsLayout->addWidget(m_caseSensitiveCheck);
    optionsLayout->addWidget(m_wholeWordCheck);
    optionsLayout->addStretch();
    mainLayout->addLayout(optionsLayout);

    // Status
    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet("color: #666;");
    mainLayout->addWidget(m_statusLabel);

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    m_findPrevBtn = new QPushButton(tr("Find Previous"), this);
    m_findPrevBtn->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F3));
    buttonLayout->addWidget(m_findPrevBtn);

    m_findNextBtn = new QPushButton(tr("Find Next"), this);
    m_findNextBtn->setShortcut(QKeySequence(Qt::Key_F3));
    m_findNextBtn->setDefault(true);
    buttonLayout->addWidget(m_findNextBtn);

    m_replaceBtn = new QPushButton(tr("Replace"), this);
    buttonLayout->addWidget(m_replaceBtn);

    m_replaceAllBtn = new QPushButton(tr("Replace All"), this);
    buttonLayout->addWidget(m_replaceAllBtn);

    m_closeBtn = new QPushButton(tr("Close"), this);
    buttonLayout->addWidget(m_closeBtn);

    mainLayout->addLayout(buttonLayout);

    // Connect signals
    connect(m_findNextBtn, &QPushButton::clicked, this, &FindReplaceDialog::onFindNext);
    connect(m_findPrevBtn, &QPushButton::clicked, this, &FindReplaceDialog::onFindPrevious);
    connect(m_replaceBtn, &QPushButton::clicked, this, &FindReplaceDialog::onReplace);
    connect(m_replaceAllBtn, &QPushButton::clicked, this, &FindReplaceDialog::onReplaceAll);
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::close);
    connect(m_findEdit, &QLineEdit::textChanged, this, &FindReplaceDialog::onTextChanged);
    connect(m_findEdit, &QLineEdit::returnPressed, this, &FindReplaceDialog::onFindNext);

    // Initial state
    showFind();

    // Set reasonable size
    setMinimumWidth(450);
}

void FindReplaceDialog::setFindText(const QString &text)
{
    m_findEdit->setText(text);
    m_findEdit->selectAll();
}

void FindReplaceDialog::showFind()
{
    m_showReplace = false;
    setWindowTitle(tr("Find"));
    m_replaceLabel->hide();
    m_replaceEdit->hide();
    m_replaceBtn->hide();
    m_replaceAllBtn->hide();
    m_findEdit->setFocus();
}

void FindReplaceDialog::showReplace()
{
    m_showReplace = true;
    setWindowTitle(tr("Find and Replace"));
    m_replaceLabel->show();
    m_replaceEdit->show();
    m_replaceBtn->show();
    m_replaceAllBtn->show();
    m_findEdit->setFocus();
}

void FindReplaceDialog::onTextChanged(const QString &text)
{
    bool hasText = !text.isEmpty();
    m_findNextBtn->setEnabled(hasText);
    m_findPrevBtn->setEnabled(hasText);
    m_replaceBtn->setEnabled(hasText);
    m_replaceAllBtn->setEnabled(hasText);

    if (text.isEmpty()) {
        m_statusLabel->clear();
    }
}

void FindReplaceDialog::onFindNext()
{
    doFind(true);
}

void FindReplaceDialog::onFindPrevious()
{
    doFind(false);
}

bool FindReplaceDialog::doFind(bool forward)
{
    QString searchText = m_findEdit->text();
    if (searchText.isEmpty()) {
        m_statusLabel->setText(tr("Enter text to find"));
        return false;
    }

    QTextDocument::FindFlags flags;
    if (!forward) {
        flags |= QTextDocument::FindBackward;
    }
    if (m_caseSensitiveCheck->isChecked()) {
        flags |= QTextDocument::FindCaseSensitively;
    }
    if (m_wholeWordCheck->isChecked()) {
        flags |= QTextDocument::FindWholeWords;
    }

    QTextCursor cursor = m_editor->textCursor();

    // If searching forward, start from current position
    // If searching backward, start from selection start
    if (!forward && cursor.hasSelection()) {
        cursor.setPosition(cursor.selectionStart());
        m_editor->setTextCursor(cursor);
    }

    bool found = m_editor->find(searchText, flags);

    if (!found) {
        // Wrap around
        cursor = m_editor->textCursor();
        if (forward) {
            cursor.movePosition(QTextCursor::Start);
        } else {
            cursor.movePosition(QTextCursor::End);
        }
        m_editor->setTextCursor(cursor);
        found = m_editor->find(searchText, flags);

        if (found) {
            m_statusLabel->setText(tr("Wrapped around"));
        }
    }

    if (found) {
        m_editor->centerCursor();
        if (m_statusLabel->text() != tr("Wrapped around")) {
            m_statusLabel->setText(tr("Found"));
        }
    } else {
        m_statusLabel->setText(tr("Not found: \"%1\"").arg(searchText));
    }

    return found;
}

void FindReplaceDialog::onReplace()
{
    QString searchText = m_findEdit->text();
    QString replaceText = m_replaceEdit->text();

    if (searchText.isEmpty()) {
        return;
    }

    QTextCursor cursor = m_editor->textCursor();

    // Check if current selection matches search text
    if (cursor.hasSelection()) {
        QString selected = cursor.selectedText();
        bool matches = m_caseSensitiveCheck->isChecked()
            ? (selected == searchText)
            : (selected.compare(searchText, Qt::CaseInsensitive) == 0);

        if (matches) {
            cursor.insertText(replaceText);
            m_statusLabel->setText(tr("Replaced"));
        }
    }

    // Find next
    doFind(true);
}

void FindReplaceDialog::onReplaceAll()
{
    QString searchText = m_findEdit->text();
    QString replaceText = m_replaceEdit->text();

    if (searchText.isEmpty()) {
        return;
    }

    QTextDocument::FindFlags flags;
    if (m_caseSensitiveCheck->isChecked()) {
        flags |= QTextDocument::FindCaseSensitively;
    }
    if (m_wholeWordCheck->isChecked()) {
        flags |= QTextDocument::FindWholeWords;
    }

    // Start from beginning
    QTextCursor cursor = m_editor->textCursor();
    cursor.movePosition(QTextCursor::Start);
    m_editor->setTextCursor(cursor);

    int count = 0;

    // Begin edit block for undo
    cursor.beginEditBlock();

    while (m_editor->find(searchText, flags)) {
        QTextCursor foundCursor = m_editor->textCursor();
        foundCursor.insertText(replaceText);
        count++;
    }

    cursor.endEditBlock();

    // Move to start
    cursor.movePosition(QTextCursor::Start);
    m_editor->setTextCursor(cursor);

    m_statusLabel->setText(tr("Replaced %1 occurrence(s)").arg(count));
}
