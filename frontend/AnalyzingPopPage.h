#ifndef ANALYZINGPOPPAGE_H
#define ANALYZINGPOPPAGE_H

#include "FileManager.h"
#include "InfoStruct.h"
#include <QDebug>
#include <QPushButton>
#include <QTimer>
#include <QWidget>

namespace Ui {
class AnalyzingPopPage;
}

class AnalyzingPopPage : public QWidget {
    Q_OBJECT

public:
    explicit AnalyzingPopPage(QWidget *parent = nullptr);
    ~AnalyzingPopPage();
    void initial();
public slots:
    void CatchAnaStep(AnalyzeStep astep);
signals:
    void finishAnalyzing();
    void updateStep();
    void analyzeStop();

private:
    Ui::AnalyzingPopPage *ui;
    int _percent;
    int _total;
    QString _stepStr = "";
    QTimer *timer;
    qreal _value = 0.0;
    qreal valueAnimator = 0.0;

    const QString beginStr = QString::fromLocal8Bit("别急，刚刚开始进行呢");
    const QString halfStr = QString::fromLocal8Bit("终于分析到一半了~~~");
    const QString lastStr = QString::fromLocal8Bit("胜利就在眼前!");
};

#endif // ANALYZINGPOPPAGE_H
