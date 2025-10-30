#include "PieChart.h"

#include <QFont>
#include <QPainter>
#include <QPainterPath>

#define SIZE 150

PieChart::PieChart(QWidget *parent) : QFrame(parent) {
    setupLayout();
    setupStyle();
}

void PieChart::reset() {
    score->setText("?");
}

void PieChart::setScore(int s) {
    s = std::min(100, s);
    s = std::max(0, s);

    // 100分的时候字体要小一些(因为三位数比两位数宽),虽然应该不会有100分的代码//
    // 另外,0分的时候可以用加大加粗的红色字体//
    score->setStyleSheet("background-color: #ffffff;"
                         "border-radius: " +
                         QString::number(score->width() / 2) +
                         "px;"
                         "color: #6f6f6f;"
                         "font: " +
                         QString::number(s == 100 ? 56 : 64) + "px Arial;");
    score->setNum(s);

    sweepAngle = s * 3.6f;
    startAngle = 450.0f - sweepAngle;

    update(); // 不update会有奇怪的效果//
}

void PieChart::setupLayout() {
    setFixedSize(SIZE, SIZE);

    int scoreSize = SIZE - 26;
    score = new QLabel("?", this); // 使用QLabel显示分数的数值//
    score->setFixedSize(scoreSize, scoreSize);
    score->move(13, 13);
    score->setAlignment(Qt::AlignCenter);
    score->setStyleSheet("background-color: #ffffff;"
                         "border-radius: " +
                         QString::number(score->width() / 2) +
                         "px;"
                         "color: #6f6f6f;"
                         "font: 64px Arial;");
}

void PieChart::setupStyle() {
    setObjectName("this");
    setStyleSheet("#this{"
                  "background-color: #ffe6c9;"
                  "border-radius: " +
                  QString::number(SIZE / 2) +
                  "px;"
                  "}");
}

void PieChart::paintEvent(QPaintEvent *e) {
    QFrame::paintEvent(e);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true); // 启用抗锯齿//

    QColor color(255, 206, 150);
    painter.setBrush(QBrush(color));

    QPainterPath path;
    path.moveTo(SIZE / 2, SIZE / 2); // 移动到圆心//
    path.arcTo(rect(), startAngle, sweepAngle);
    painter.fillPath(path, painter.brush());
}
