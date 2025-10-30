#include "LineNumberList.h"
#include "TextEdit.h"
#include <QDebug>
#include <QString>

#define FIXED_WIDTH 80

int LineNumberList::lineHeight = 0;

LineNumberList::LineNumberList(QWidget *parent, int _lineHeight) : QLabel{parent} {
    lineHeight = _lineHeight;
    setupStyle();
}

LineNumberList::~LineNumberList() {
}

void LineNumberList::setupStyle() {
    setFixedWidth(FIXED_WIDTH);
    setStyleSheet("background-color: rgb(255, 155, 82);"
                  "border: none;"
                  "padding-right: 4px;");

    setAlignment(Qt::AlignRight);
    setFont(QFont("Source Code Pro, Microsoft Yahei", 15));
}

void LineNumberList::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        clickedLine = event->pos().y() / TextEdit::lineHeight();
        isDragging = true;
        if (clickedLine < curLines) {
            emit selectLinesSignal(clickedLine, clickedLine);
        }
    }
    event->accept();
}

void LineNumberList::mouseMoveEvent(QMouseEvent *event) {
    int releasedLine = event->pos().y() / lineHeight; // 鼠标松开时所在行
    if (isDragging && (clickedLine < curLines || releasedLine < curLines)) {
        selectedLines.first = clickedLine, selectedLines.second = releasedLine;
        if (selectedLines.first > selectedLines.second) {
            std::swap(selectedLines.first, selectedLines.second);
        }
        selectedLines.second = std::min(curLines - 1, selectedLines.second);

        emit selectLinesSignal(selectedLines.first, selectedLines.second);
    }
    event->accept();
}

void LineNumberList::mouseReleaseEvent(QMouseEvent *event) {
    isDragging = false;
    event->accept();
}

void LineNumberList::wheelEvent(QWheelEvent *event) {
    // 发送信号出去让代码区滚动
    emit wheel(event);
}

void LineNumberList::setLineCount(int n) {
    curLines = n;
    QString s = "";
    for (int i = 1; i <= n; ++i) {
        s += QString::number(i);
        if (i != n)
            s += "\n";
    }
    setText(s);
}
