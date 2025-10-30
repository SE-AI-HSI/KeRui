#include "AnalyzingPopPage.h"
#include "ui_AnalyzingPopPage.h"

AnalyzingPopPage::AnalyzingPopPage(QWidget *parent) : QWidget(parent), ui(new Ui::AnalyzingPopPage) {
    ui->setupUi(this);

    ui->popDlg->hide();
    connect(ui->popPage_closeBtn, &QPushButton::clicked, this, [=]() {
        ui->popDlg->hide();
    });
    connect(ui->popPage_closeBtn2, &QPushButton::clicked, this, [=]() {
        ui->popDlg->hide();
    });
    connect(ui->popPage_cancleBtn, &QPushButton::clicked, this, [=]() {
        emit finishAnalyzing();
    });

    // 使用timer实现框框停一会儿消失//
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [=]() {
        _value += 0.125;
        valueAnimator = 1.0 - pow(1.0 - _value, 3);
        if (_value > 1.01) { // 由于精度的问题,不能直接比较>1//
            timer->stop();
            QTimer::singleShot(150, this, [=]() {
                this->hide();
            });
        }
    });

    connect(ui->closeBtn, &QPushButton::clicked, this, [=]() {
        int x = ui->Box->width() / 2 - ui->popDlg->width() / 2;
        int y = ui->Box->height() / 2 - ui->popDlg->height() / 2;
        ui->popDlg->move(x, y);
        ui->popDlg->show();
    });

    connect(this, &AnalyzingPopPage::updateStep, this, [=]() {
        ui->PercentageBar->setFixedWidth(valueAnimator * _percent * ui->PercentageBarBg->width() / _total);
        QString str = _stepStr;
        if (_total > 1) {
            if (_percent > 0.2 * _total && _percent < 0.3 * _total)
                str = beginStr;
            else {
                if (_percent > 0.5 * _total && _percent < 0.6 * _total)
                    str = halfStr;
                else {
                    if (_percent > 0.9 * _total)
                        str = lastStr;
                }
            }
        }
        ui->percentText->setText(QString::number(valueAnimator * _percent) + "/" +
                                 QString::number(valueAnimator * _total));
        ui->AnalyzeStepText->setText(str);
        if (_stepStr == QString::fromLocal8Bit("完成")) {
            ui->percentText->setText("");
            ui->AnalyzeStepText->setText(QString::fromLocal8Bit("好喽，完成啦^_^"));
            timer->start(10);
        }
    });

    connect(ui->popPage_cancleBtn, &QPushButton::clicked, this, [=]() {
        emit analyzeStop();
        hide();
    });
}

AnalyzingPopPage::~AnalyzingPopPage() {
    delete ui;
}

void AnalyzingPopPage::CatchAnaStep(AnalyzeStep astep) {
    _percent = astep.m_currentStep + 1;
    _stepStr = astep.m_stepName;
    _stepStr.replace(FileManager::projectPath().replace('/', '\\') + '\\', "");
    _total = astep.m_totalStep + 1;
    emit updateStep();
}

void AnalyzingPopPage::initial() {
    _percent = 0;
    _value = 0;
    _total = 100;
    ui->percentText->setText("??/??");
    ui->PercentageBar->setFixedWidth(0);
    ui->AnalyzeStepText->setText(QString::fromLocal8Bit("分析准备中~~~"));
    ui->popDlg->hide();
    valueAnimator = 1;
}
