#include "CCTextEdit.h"
#include <QDebug>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCursor>

int CCTextEdit::_lineHeight;

CCTextEdit::CCTextEdit(QWidget *parent) : QTextEdit(parent) {
    setupStyle();
    setReadOnly(true);

    _lineHeight = fontMetrics().lineSpacing();
}

int CCTextEdit::lineCount() {
    return document()->blockCount();
}

int CCTextEdit::minHeight() {
    return document()->blockCount() * _lineHeight + 12;
}

void CCTextEdit::setText(const QString &text) {
    setPlainText(text);
    setLineHeight();
}

void CCTextEdit::insertNewLines(int line_below, int n) {
    gotoLine(line_below);
    QTextCursor c = textCursor();
    c.movePosition(QTextCursor::EndOfBlock);
    setTextCursor(c);
    for (int i = 0; i < n; ++i)
        insertPlainText("\n");
}

QString CCTextEdit::getText(int line_beg, int line_end) {
    QString res;
    for (; line_beg <= line_end; ++line_beg) {
        res += document()->findBlockByNumber(line_beg - 1).text();
        if (line_beg != line_end)
            res += "\r\n";
    }
    return res;
}

void CCTextEdit::replaceText(const QString &t, int line_beg, int line_end) {
    for (; line_end >= line_beg; --line_end) {
        QTextCursor c(document()->findBlockByNumber(line_end - 1));
        c.clearSelection();
        c.select(QTextCursor::LineUnderCursor);
        c.removeSelectedText();
        if (line_end != line_beg)
            c.deletePreviousChar();
    }
    QTextCursor c(document()->findBlockByNumber(line_beg - 1));
    c.insertText(t);
}

void CCTextEdit::deleteLine(int line) {
    QTextCursor c(document()->findBlockByNumber(line - 1));
    c.select(QTextCursor::LineUnderCursor);
    c.removeSelectedText();
    c.deletePreviousChar();
}

void CCTextEdit::setupStyle() {
    setLineWrapMode(NoWrap);                                // 关闭自动文字折行//
    setFont(QFont("Source Code Pro, Microsoft Yahei", 15)); // 25px行高

    setStyleSheet("background-color: transparent;border: none;color: rgb(25, 25, 25);");
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void CCTextEdit::wheelEvent(QWheelEvent *e) {
    // 保持空
}

void CCTextEdit::setLineHeight() {
    QTextCursor c = textCursor();
    QTextBlockFormat blockFormat = c.blockFormat();
    blockFormat.setLineHeight(_lineHeight, QTextBlockFormat::FixedHeight);

    c.select(QTextCursor::Document);
    c.mergeBlockFormat(blockFormat);
    c.clearSelection();
    setTextCursor(c);
}

void CCTextEdit::gotoLine(int line) {
    QTextCursor c = textCursor();
    int pos = document()->findBlockByNumber(line - 1).position();
    c.setPosition(pos, QTextCursor::MoveAnchor);
    setTextCursor(c);
}
