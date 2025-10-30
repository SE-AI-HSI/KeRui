#include "SummaryWidget.h"
#include "FileManager.h"
#include "ui_SummaryWidget.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QTextStream>
SummaryWidgetEventProxy SummaryWidget::eventProxy;

SummaryWidget::SummaryWidget(std::vector<SummaryRowData> &_datas, QWidget *parent)
    : QWidget(parent), ui(new Ui::SummaryWidget), datas(_datas) {
    ui->setupUi(this);
    loadRows();
}

SummaryWidget::~SummaryWidget() {
    clearRows();
    delete ui;
}

bool SummaryWidget::saveSummary(const std::vector<SummaryRowData> &datas) {
    QString projectPath = FileManager::projectPath();
    if (projectPath.isEmpty()) {
        return false;
    } else {
        QFile file(projectPath + "/" + SUMMARY_FILE_NAME);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return false;
        QTextStream output(&file);
        output.setCodec("UTF-8");

        QJsonObject container;
        for (int i = 0; i < datas.size(); ++i) {
            const SummaryRowData &data = datas[i];
            QJsonObject obj;
            obj.insert("numErrors", QJsonValue(data.numErrors));
            obj.insert("numNorm", QJsonValue(data.numNorm));
            obj.insert("numEffi", QJsonValue(data.numEffi));
            obj.insert("numMain", QJsonValue(data.numMain));
            obj.insert("numSecu", QJsonValue(data.numSecu));

            container.insert(data.filePath, QJsonValue(obj));
        }
        QJsonDocument d = QJsonDocument(container);
        output << d.toJson();
        file.close();
        return true;
    }
}

bool SummaryWidget::readSummary(std::vector<SummaryRowData> &datas) {
    QString projectPath = FileManager::projectPath();
    if (projectPath.isEmpty()) {
        return false;
    } else {
        QFile file(projectPath + "/" + SUMMARY_FILE_NAME);
        if (!file.open(QIODevice::ReadOnly))
            return false;
        QTextStream input(&file);
        input.setCodec("UTF-8");

        QPair<QString, QString> text = FileManager::read(projectPath + "/" + SUMMARY_FILE_NAME);
        QJsonParseError jsonError;
        QJsonDocument doc = QJsonDocument::fromJson(text.second.toUtf8(), &jsonError);
        if (jsonError.error != QJsonParseError::NoError && !doc.isNull()) {
            qDebug() << "SummaryWidget::Json格式错误！" << jsonError.error;
            return false;
        }
        QJsonObject obj = doc.object();
        QStringList keys = obj.keys();
        SummaryRowData data;
        for (QString &k : keys) {
            QJsonObject t = obj[k].toObject();
            data.filePath = k;
            data.numErrors = t["numErrors"].toInt();
            data.numNorm = t["numNorm"].toInt();
            data.numEffi = t["numEffi"].toInt();
            data.numMain = t["numMain"].toInt();
            data.numSecu = t["numSecu"].toInt();
            datas.emplace_back(data);
        }
        file.close();
        return true;
    }
}

void SummaryWidget::on_pushButton_pressed() { /// 文件名升序
    if (ui->pushButton->isChecked()) {
        ui->pushButton->setChecked(false);
    }
    ui->pushButton_2->setChecked(false);
    resetSortButtons();

    clearRows();
    for (int i = 1; i < datas.size(); ++i) {
        SummaryRowData temp = datas[i];
        int j = i;
        for (; j >= 1 && temp.fileName() < datas[j - 1].fileName(); --j) {
            datas[j] = datas[j - 1];
        }
        datas[j] = temp;
    }
    loadRows();
}

void SummaryWidget::on_pushButton_2_pressed() { /// 文件名降序
    if (ui->pushButton_2->isChecked()) {
        ui->pushButton_2->setChecked(false);
    }
    ui->pushButton->setChecked(false);
    resetSortButtons();

    clearRows();
    for (int i = 1; i < datas.size(); ++i) {
        SummaryRowData temp = datas[i];
        int j = i;
        for (; j >= 1 && temp.fileName() > datas[j - 1].fileName(); --j) {
            datas[j] = datas[j - 1];
        }
        datas[j] = temp;
    }
    loadRows();
}

void SummaryWidget::on_pushButton_3_pressed() { /// 错误数升序
    if (ui->pushButton_3->isChecked()) {
        ui->pushButton_3->setChecked(false);
    }
    ui->pushButton_4->setChecked(false);
    resetSortButtons();

    clearRows();
    for (int i = 1; i < datas.size(); ++i) {
        SummaryRowData temp = datas[i];
        int j = i;
        for (; j >= 1 && temp.numErrors < datas[j - 1].numErrors; --j) {
            datas[j] = datas[j - 1];
        }
        datas[j] = temp;
    }
    loadRows();
}

void SummaryWidget::on_pushButton_4_pressed() { /// 错误数降序
    if (ui->pushButton_4->isChecked()) {
        ui->pushButton_4->setChecked(false);
    }
    ui->pushButton_3->setChecked(false);
    resetSortButtons();

    clearRows();
    for (int i = 1; i < datas.size(); ++i) {
        SummaryRowData temp = datas[i];
        int j = i;
        for (; j >= 1 && temp.numErrors > datas[j - 1].numErrors; --j) {
            datas[j] = datas[j - 1];
        }
        datas[j] = temp;
    }
    loadRows();
}

void SummaryWidget::on_pushButton_5_pressed() { /// 规范性升序
    if (ui->pushButton_5->isChecked()) {
        ui->pushButton_5->setChecked(false);
    }
    ui->pushButton_6->setChecked(false);
    resetSortButtons();

    clearRows();
    for (int i = 1; i < datas.size(); ++i) {
        SummaryRowData temp = datas[i];
        int j = i;
        for (; j >= 1 && temp.numNorm < datas[j - 1].numNorm; --j) {
            datas[j] = datas[j - 1];
        }
        datas[j] = temp;
    }
    loadRows();
}

void SummaryWidget::on_pushButton_6_pressed() { /// 规范性降序
    if (ui->pushButton_6->isChecked()) {
        ui->pushButton_6->setChecked(false);
    }
    ui->pushButton_5->setChecked(false);
    resetSortButtons();

    clearRows();
    for (int i = 1; i < datas.size(); ++i) {
        SummaryRowData temp = datas[i];
        int j = i;
        for (; j >= 1 && temp.numNorm > datas[j - 1].numNorm; --j) {
            datas[j] = datas[j - 1];
        }
        datas[j] = temp;
    }
    loadRows();
}

void SummaryWidget::on_pushButton_7_pressed() { /// 执行效率升序
    if (ui->pushButton_7->isChecked()) {
        ui->pushButton_7->setChecked(false);
    }
    ui->pushButton_8->setChecked(false);
    resetSortButtons();

    clearRows();
    for (int i = 1; i < datas.size(); ++i) {
        SummaryRowData temp = datas[i];
        int j = i;
        for (; j >= 1 && temp.numEffi < datas[j - 1].numEffi; --j) {
            datas[j] = datas[j - 1];
        }
        datas[j] = temp;
    }
    loadRows();
}

void SummaryWidget::on_pushButton_8_pressed() { /// 执行效率降序
    if (ui->pushButton_8->isChecked()) {
        ui->pushButton_8->setChecked(false);
    }
    ui->pushButton_7->setChecked(false);
    resetSortButtons();

    clearRows();
    for (int i = 1; i < datas.size(); ++i) {
        SummaryRowData temp = datas[i];
        int j = i;
        for (; j >= 1 && temp.numEffi > datas[j - 1].numEffi; --j) {
            datas[j] = datas[j - 1];
        }
        datas[j] = temp;
    }
    loadRows();
}

void SummaryWidget::on_pushButton_11_pressed() { /// 可维护性升序
    if (ui->pushButton_11->isChecked()) {
        ui->pushButton_11->setChecked(false);
    }
    ui->pushButton_12->setChecked(false);
    resetSortButtons();

    clearRows();
    for (int i = 1; i < datas.size(); ++i) {
        SummaryRowData temp = datas[i];
        int j = i;
        for (; j >= 1 && temp.numMain < datas[j - 1].numMain; --j) {
            datas[j] = datas[j - 1];
        }
        datas[j] = temp;
    }
    loadRows();
}

void SummaryWidget::on_pushButton_12_pressed() { /// 可维护性降序
    if (ui->pushButton_11->isChecked()) {
        ui->pushButton_11->setChecked(false);
    }
    ui->pushButton_12->setChecked(false);
    resetSortButtons();

    clearRows();
    for (int i = 1; i < datas.size(); ++i) {
        SummaryRowData temp = datas[i];
        int j = i;
        for (; j >= 1 && temp.numMain > datas[j - 1].numMain; --j) {
            datas[j] = datas[j - 1];
        }
        datas[j] = temp;
    }
    loadRows();
}

void SummaryWidget::on_pushButton_9_pressed() { /// 安全性升序
    if (ui->pushButton_9->isChecked()) {
        ui->pushButton_9->setChecked(false);
    }
    ui->pushButton_10->setChecked(false);
    resetSortButtons();

    clearRows();
    for (int i = 1; i < datas.size(); ++i) {
        SummaryRowData temp = datas[i];
        int j = i;
        for (; j >= 1 && temp.numSecu < datas[j - 1].numSecu; --j) {
            datas[j] = datas[j - 1];
        }
        datas[j] = temp;
    }
    loadRows();
}

void SummaryWidget::on_pushButton_10_pressed() { /// 安全性降序
    if (ui->pushButton_10->isChecked()) {
        ui->pushButton_10->setChecked(false);
    }
    ui->pushButton_9->setChecked(false);
    resetSortButtons();

    clearRows();
    for (int i = 1; i < datas.size(); ++i) {
        SummaryRowData temp = datas[i];
        int j = i;
        for (; j >= 1 && temp.numSecu > datas[j - 1].numSecu; --j) {
            datas[j] = datas[j - 1];
        }
        datas[j] = temp;
    }
    loadRows();
}
void SummaryWidget::loadRows() {
    int index = 0;
    for (const SummaryRowData &data : datas) {
        rows.emplace_back(new SummaryRow(data));
        ui->verticalLayout_7->insertWidget(index++, rows.back());
    }
}

void SummaryWidget::clearRows() {
    for (QWidget *&l : rows) {
        delete l;
    }
    rows.clear();
}

void SummaryWidget::resetSortButtons() {
    ui->pushButton->setChecked(false);
    ui->pushButton_2->setChecked(false);
    ui->pushButton_3->setChecked(false);
    ui->pushButton_4->setChecked(false);
    ui->pushButton_5->setChecked(false);
    ui->pushButton_6->setChecked(false);
    ui->pushButton_7->setChecked(false);
    ui->pushButton_8->setChecked(false);
    ui->pushButton_9->setChecked(false);
    ui->pushButton_10->setChecked(false);
    ui->pushButton_11->setChecked(false);
    ui->pushButton_12->setChecked(false);
}

//////////    /////////  //    ///  /////////  //////     /////////  ///////
//////   //     ///      //   ///     ///      ///   //   ///        ///    //
/////   //     ///       //  ///     ///      ///   //   /////////  ///////
////   //     ///         /////     ///      ///   //   ///        /////
///////   /////////       ///   /////////  ///////    /////////  ///  ////

SummaryRow::SummaryRow(const SummaryRowData &data) {
#define ROW_HEIGHT 30
#define UNIT QString::fromLocal8Bit("个")
    QPushButton *fileName = new QPushButton(data.fileName());
    fileName->setObjectName(data.filePath); // 记录文件路径

    QLabel *numErrors = new QLabel(QString::number(data.numErrors) + UNIT);
    QLabel *numNorm = new QLabel(QString::number(data.numNorm) + UNIT);
    QLabel *numEffi = new QLabel(QString::number(data.numEffi) + UNIT);
    QLabel *numMain = new QLabel(QString::number(data.numMain) + UNIT);
    QLabel *numSecu = new QLabel(QString::number(data.numSecu) + UNIT);
    /// 设置样式
    setObjectName("Container");
    setStyleSheet("#Container:hover{"
                  "background-color: rgb(255, 204, 175);"
                  "}");

    fileName->setCursor(Qt::PointingHandCursor);
    fileName->setStyleSheet("QPushButton{"
                            "border: none;"
                            "text-align: left;"
                            "padding-left: 6px;"
                            "}"
                            "QPushButton:hover{"
                            "text-decoration: underline;"
                            "}");
    /// 连接点击事件
    connect(fileName, &QPushButton::pressed, this, [=] {
        emit SummaryWidget::eventProxy.openFile(fileName->objectName());
    });
    ///
    setFixedHeight(40);
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(14, 0, 14, 0);
    ///
    layout->addWidget(fileName, 2);
    layout->addWidget(numErrors, 1);
    layout->addWidget(numNorm, 1);
    layout->addWidget(numEffi, 1);
    layout->addWidget(numMain, 1);
    layout->addWidget(numSecu, 1);
}
