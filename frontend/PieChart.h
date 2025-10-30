#ifndef PIECHART_H
#define PIECHART_H

#include <QFrame>
#include <QLabel>

/**
 * @class PieChart
 * @brief 显示分数的圆饼图,因为需要重写paintEvent所以单独作一个类
 */

class PieChart : public QFrame {
    Q_OBJECT
public:
    PieChart(QWidget *parent);
    void reset();
    void setScore(int s);

private:
    // init
    void setupPainter();
    void setupLayout();
    void setupStyle();
    // events
    void paintEvent(QPaintEvent *) override;

private:
    QLabel *score; // 用于显示分数的数值//

    float startAngle = .0f;
    float sweepAngle = .0f;
};

#endif // PIECHART_H
