#include "ScoreWidget.h"
#include "InfoStruct.h"
#include "ui_ScoreWidget.h"
// #include "CppAnalyzer.h"
#include <cmath>

#define NORMATIVE_INDEX 0
#define EFFICIENCY_INDEX 1
#define SECURITY_INDEX 3
#define MAINTAINABILITY_INDEX 2

#define EXPANDED_HEIGHT 256
#define COLLAPSED_HEIGHT 26
#define SELECTBOX_HIGHT 52
#define SELECTBOX_HIDE_HIGHT 18

ScoreWidget::ScoreWidget(QWidget *parent) : QWidget(parent), ui(new Ui::ScoreWidget) {
    ui->setupUi(this);
    timer = new QTimer(this);

    setupLayout();
    setupStyle();
    connectEvents();

    reset();
}

ScoreWidget::~ScoreWidget() {
    delete ui;
}

void ScoreWidget::reset() {
    timer->stop();
    valueAnimator = _value = 0.0;

    pieChart->reset();

    ui->NormativePercentage->setText("?%");
    ui->EfficiencyPercentage->setText("?%");
    ui->SecurityPercentage->setText("?%");
    ui->MaintainabilityPercentage->setText("?%");
    ui->Comment->setPlainText(QString::fromLocal8Bit("点击“分析项目”按钮开始分析。"));
}
//Normalization Efficiency Maintanability Security
void ScoreWidget::setData(int total, int s1, int s2, int s3, int s4, const QString &comment) {
    totalScore = total;
    percentage[0] = s1;
    percentage[1] = s2;
    percentage[2] = s3;
    percentage[3] = s4;

    timer->start(10);
    ui->Comment->setPlainText(comment);
}

void ScoreWidget::setupLayout() {
    ui->Comment->document()->setDocumentMargin(0);

    pieChart = new PieChart(this);
    // 将占位窗口替换为FileTabBar//
    delete ui->widget_3->layout()->replaceWidget(ui->PieChartPlaceHolder, pieChart);

    ui->ScoreBoxstackedWidget->setCurrentIndex(0);
    ui->SelectBoxStackedWidget->setCurrentIndex(0);
    isScoreBoxExpanded = true;
    isSelectBoxExpanded = true;
    ui->selectErrorBtn->setChecked(true);
    ui->selectNormalizationBtn->setChecked(true);
    ui->selectEfficiencyBtn->setChecked(true);
    ui->selectMaintainabliltyBtn->setChecked(true);
    ui->selectSecurityBtn->setChecked(true);
    selectBoxHight = SELECTBOX_HIGHT;
    scoreBoxHight = EXPANDED_HEIGHT;
}

void ScoreWidget::setupStyle() {
    shadow = GraphicsEffectUtil::newShadow(this);
    ui->MainBackground->setGraphicsEffect(shadow);

    pieChart->setGraphicsEffect(GraphicsEffectUtil::newShadow(this, 128, 30));
    ui->Comment->setGraphicsEffect(GraphicsEffectUtil::newShadow(this, 96));

    ui->NormativeBarBg->setGraphicsEffect(GraphicsEffectUtil::newShadow(this, 128, 10));
    ui->EfficiencyBarBg->setGraphicsEffect(GraphicsEffectUtil::newShadow(this, 128, 10));
    ui->SecurityBarBg->setGraphicsEffect(GraphicsEffectUtil::newShadow(this, 128, 10));
    ui->MaintainabilityBarBg->setGraphicsEffect(GraphicsEffectUtil::newShadow(this, 128, 10));
}

void ScoreWidget::connectEvents() {
    // 点击折叠与展开//
    connect(ui->CollapseBtn, &QPushButton::clicked, [=] {
        if (isScoreBoxExpanded == true) { // 点击之前的状态是展开
            isScoreBoxExpanded = false;
            scoreBoxHight = COLLAPSED_HEIGHT;
            ui->CollapsedBackground->setGraphicsEffect(shadow);
            ui->ScoreBoxstackedWidget->setCurrentIndex(1);
            ui->ScoreBoxstackedWidget->setFixedHeight(COLLAPSED_HEIGHT);
            setFixedHeight(scoreBoxHight + selectBoxHight);

            ui->CollapseBtn->setStyleSheet("border-image: url(:/images/select.svg);");
            ui->CollapseBtn_2->setStyleSheet("border-image: url(:/images/select.svg);");
        } else {
            isScoreBoxExpanded = true;
            scoreBoxHight = EXPANDED_HEIGHT;
            ui->MainBackground->setGraphicsEffect(shadow);
            ui->ScoreBoxstackedWidget->setCurrentIndex(0);
            ui->ScoreBoxstackedWidget->setFixedHeight(EXPANDED_HEIGHT);
            setFixedHeight(scoreBoxHight + selectBoxHight);

            ui->CollapseBtn->setStyleSheet("border-image: url(:/images/selectd.svg);");
            ui->CollapseBtn_2->setStyleSheet("border-image: url(:/images/selectd.svg);");
        }
    });
    connect(ui->CollapseBtn_2, &QPushButton::clicked, [=] {
        if (isScoreBoxExpanded == true) { // 点击之前的状态是展开
            isScoreBoxExpanded = false;
            scoreBoxHight = COLLAPSED_HEIGHT;
            ui->CollapsedBackground->setGraphicsEffect(shadow);
            ui->ScoreBoxstackedWidget->setCurrentIndex(1);
            ui->ScoreBoxstackedWidget->setFixedHeight(COLLAPSED_HEIGHT);
            setFixedHeight(scoreBoxHight + selectBoxHight);

            ui->CollapseBtn->setStyleSheet("border-image: url(:/images/select.svg);");
            ui->CollapseBtn_2->setStyleSheet("border-image: url(:/images/select.svg);");
        } else {
            isScoreBoxExpanded = true;
            scoreBoxHight = EXPANDED_HEIGHT;
            ui->MainBackground->setGraphicsEffect(shadow);
            ui->ScoreBoxstackedWidget->setCurrentIndex(0);
            ui->ScoreBoxstackedWidget->setFixedHeight(EXPANDED_HEIGHT);
            setFixedHeight(scoreBoxHight + selectBoxHight);

            ui->CollapseBtn->setStyleSheet("border-image: url(:/images/selectd.svg);");
            ui->CollapseBtn_2->setStyleSheet("border-image: url(:/images/selectd.svg);");
        }
    });
    // 点击筛选按钮//
    connect(ui->SelectBtn, &QPushButton::clicked, [=] {
        if (isSelectBoxExpanded == false) {
            isSelectBoxExpanded = true;
            selectBoxHight = SELECTBOX_HIGHT;
            ui->SelectBoxStackedWidget->setCurrentIndex(0);
            ui->SelectBoxStackedWidget->setFixedHeight(SELECTBOX_HIGHT);
            setFixedHeight(scoreBoxHight + selectBoxHight);
        } else {
            isSelectBoxExpanded = false;
            selectBoxHight = SELECTBOX_HIDE_HIGHT;
            ui->SelectBoxStackedWidget->setCurrentIndex(1);
            ui->SelectBoxStackedWidget->setFixedHeight(SELECTBOX_HIDE_HIGHT);
            setFixedHeight(scoreBoxHight + selectBoxHight);
        }
    });
    // 使用timer实现动态增长的分数//
    connect(timer, &QTimer::timeout, [=] {
        pieChart->setScore(valueAnimator * totalScore);
        updatePercentage(true);
        _value += 0.0125;
        valueAnimator = 1.0 - pow(1.0 - _value, 3);
        if (_value > 1.01) { // 由于精度的问题,不能直接比较>1//
            timer->stop();
        }
    });
    // 屏蔽Error标签的点击信号
    connect(ui->selectErrorBtn, &QPushButton::clicked, [=] {
        errorFlag = ui->selectErrorBtn->isChecked();
        emit LabelHide(selectBoxFlags, errorFlag);
    });
    // 屏蔽Normalization规范性标签的点击信号
    connect(ui->selectNormalizationBtn, &QPushButton::clicked, [=] {
        if (ui->selectNormalizationBtn->isChecked() == true)
            selectBoxFlags |= NORMATIVE;
        else
            selectBoxFlags &= ~(NORMATIVE);
        emit LabelHide(selectBoxFlags, errorFlag);
    });
    // 屏蔽Efficiency执行效率标签的点击信号
    connect(ui->selectEfficiencyBtn, &QPushButton::clicked, [=] {
        if (ui->selectEfficiencyBtn->isChecked() == true)
            selectBoxFlags |= EFFICIENCY;
        else
            selectBoxFlags &= ~(EFFICIENCY);
        emit LabelHide(selectBoxFlags, errorFlag);
    });
    // 屏蔽Maintainability可维护性标签的点击信号
    connect(ui->selectMaintainabliltyBtn, &QPushButton::clicked, [=] {
        if (ui->selectMaintainabliltyBtn->isChecked() == true)
            selectBoxFlags |= MAINTAINABILITY;
        else
            selectBoxFlags &= ~(MAINTAINABILITY);
        emit LabelHide(selectBoxFlags, errorFlag);
    });
    // 屏蔽Security安全性标签的点击信号
    connect(ui->selectSecurityBtn, &QPushButton::clicked, [=] {
        if (ui->selectSecurityBtn->isChecked() == true)
            selectBoxFlags |= SECURITY;
        else
            selectBoxFlags &= ~(SECURITY);
        emit LabelHide(selectBoxFlags, errorFlag);
    });
}

void ScoreWidget::resizeEvent(QResizeEvent *) {
    updatePercentage(false);
}

void ScoreWidget::updatePercentage(bool updateNumber) {
    ui->NormativeBar->setFixedWidth(valueAnimator * percentage[NORMATIVE_INDEX] * ui->NormativeBarBg->width() / 100);
    ui->EfficiencyBar->setFixedWidth(valueAnimator * percentage[EFFICIENCY_INDEX] * ui->EfficiencyBarBg->width() / 100);
    ui->SecurityBar->setFixedWidth(valueAnimator * percentage[SECURITY_INDEX] * ui->SecurityBarBg->width() / 100);
    ui->MaintainabilityBar->setFixedWidth(valueAnimator * percentage[MAINTAINABILITY_INDEX] *
                                          ui->MaintainabilityBarBg->width() / 100);

    if (updateNumber) {
        ui->NormativePercentage->setText(QString::number(int(valueAnimator * percentage[NORMATIVE_INDEX])) + "%");
        ui->EfficiencyPercentage->setText(QString::number(int(valueAnimator * percentage[EFFICIENCY_INDEX])) + "%");
        ui->SecurityPercentage->setText(QString::number(int(valueAnimator * percentage[SECURITY_INDEX])) + "%");
        ui->MaintainabilityPercentage->setText(QString::number(int(valueAnimator * percentage[MAINTAINABILITY_INDEX])) +
                                               "%");
    }
}
