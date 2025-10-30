#ifndef SCOREWIDGET_H
#define SCOREWIDGET_H

#include "GraphicsEffectUtil.h"
#include "PieChart.h"
#include <QTimer>
#include <QWidget>

namespace Ui {
class ScoreWidget;
}

class ScoreWidget : public QWidget {
    Q_OBJECT

public:
    explicit ScoreWidget(QWidget *parent);
    ~ScoreWidget();
    void reset();
    void setData(int total, int s1, int s2, int s3, int s4, const QString &comment);

private:
    // init
    void setupLayout();
    void setupStyle();
    // events
    void connectEvents();
    void resizeEvent(QResizeEvent *event) override;
    // others
    void updatePercentage(bool updateNumber);

private:
    Ui::ScoreWidget *ui;
    QTimer *timer;
    qreal _value = 0.0;
    qreal valueAnimator = 0.0;

    int selectBoxHight;
    int scoreBoxHight;
    bool isScoreBoxExpanded = true;
    bool isSelectBoxExpanded = true;
    int selectBoxFlags = 0b1111;
    bool errorFlag = 1;

    PieChart *pieChart;

    int totalScore = 0;
    int percentage[4];

    QGraphicsDropShadowEffect *shadow; // 折叠和展开的QFrame共用一个shadow//

signals:
    void LabelHide(int falgs, bool error);
};

#endif // SCOREWIDGET_H
