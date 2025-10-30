#include "SettingWidget.h"

#include "AnalyzerClient.h"
#include "GraphicsEffectUtil.h"
#include "Utils.h"
#include "jsonutil.h"
#include "ui_SettingWidget.h"

#define SETTING_PATH "./Setting.json"
#define ANALYZE_CONFIG_PATH "./AnalyzeConfig.json"

SettingWidget::SettingWidget() : QWidget(nullptr), ui(new Ui::SettingWidget) {
    ui->setupUi(this);
    this->setStyleSheet(getGlobalFontQSS());

    setupLayout();
    setWindowTitle(QString::fromLocal8Bit("设置"));

    QRect window_rect = AnalyzerClient::windowRect;
    setFixedSize(window_rect.width() * 0.65, window_rect.height() * 0.85);
    show();
    move(window_rect.x() + (window_rect.width() - width()) / 2,
         window_rect.y() + (window_rect.height() - height()) / 2);

    readSetting();
}

SettingWidget::~SettingWidget() {
    delete ui;
}

QString SettingWidget::getGlobalFontQSS() {
    QString qss = R"(
font-family: {};
font-style: {};
font-weight: {};
font-size: {}px;
)";

    auto setting = JsonUtil::read(SETTING_PATH);

    auto fontFamily = setting["fontFamily"].toString();
    auto fontStyle = setting["fontStyle"].toString();
    auto fontSize = setting["fontSize"].toString();

    QString style = fontStyle.contains("Italic") ? "Italic" : "Regular";
    QString weight = fontStyle.contains("Bold") ? "Bold" : "Normal";
    qss = format(qss, QStringList() << fontFamily << style << weight << fontSize);

    return qss;
}

QVector<int> SettingWidget::getScoreWeights()
{
    auto analyzeConfig = JsonUtil::read(ANALYZE_CONFIG_PATH);
    QVector<int> weights;
    auto scoreWeights = analyzeConfig["scoreWeights"].toObject();
    weights.append( scoreWeights["normative"].toInt() );
    weights.append( scoreWeights["efficiency"].toInt() );
    weights.append( scoreWeights["maintainability"].toInt() );
    weights.append( scoreWeights["security"].toInt() );

    return weights;
}

void SettingWidget::setupLayout() {
    setWindowModality(Qt::ApplicationModal);
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    // 阴影
    ui->widget->setGraphicsEffect(GraphicsEffectUtil::newShadow(this, 64));
    ui->frame->setGraphicsEffect(GraphicsEffectUtil::newShadow(this, 64));
}

void SettingWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && event->pos().y() < ui->TopBar->height()) {
        isDragging = true;
        mousePressPos = event->globalPos() - this->pos();
        event->accept();
    } else {
        QWidget::mousePressEvent(event);
    }
}

void SettingWidget::mouseMoveEvent(QMouseEvent *event) {
    if (isDragging) {
        move(event->globalPos() - mousePressPos);
        event->accept();
    } else {
        QWidget::mouseMoveEvent(event);
    }
}

void SettingWidget::mouseReleaseEvent(QMouseEvent *event) {
    isDragging = false;
    QWidget::mouseReleaseEvent(event);
}

void SettingWidget::updateFontPreview() {
    QString font = R"(
font-family: {};
font-style: {};
font-weight: {};
font-size: {}px;
)";
    QString style = ui->selectedFontStyle->text().contains("Italic") ? "Italic" : "Regular";
    QString weight = ui->selectedFontStyle->text().contains("Bold") ? "Bold" : "Normal";
    font = format(font, QStringList() << ui->selectedFont->text() << style << weight << ui->selectedFontSize->text());
    ui->fontPreview->setStyleSheet(R"(
background-color: rgb(255, 247, 238);
border: 2px solid #ffffff;
border-radius: 5px;
)" + font);
}

void SettingWidget::insertWarningItem(int w_code, const QString &description, bool is_masked) {
    auto widget = new QWidget(this);
    auto layout = new QHBoxLayout(widget);
    auto labelCode = new QLabel(this);
    auto labelDescription = new QLabel(this);
    auto pushButton = new QPushButton(this);

    std::vector<QWidget *> vect = {widget, labelCode, labelDescription, pushButton};

    labelCode->setText(QString::number(w_code));
    labelDescription->setText(description);

    pushButton->setObjectName(QString::number(w_code));
    pushButton->setText(is_masked ? QString::fromLocal8Bit("恢复") : QString::fromLocal8Bit("隐藏"));
    pushButton->setCheckable(true);
    pushButton->setChecked(is_masked);
    pushButton->setStyleSheet("QPushButton{color: #5bcbee;} QPushButton:checked{color: #ff6b00;}");
    pushButton->setCursor(Qt::PointingHandCursor);

    connect(pushButton, &QPushButton::clicked, [=]() {
        if (pushButton->text() == QString::fromLocal8Bit("隐藏")) {
            pushButton->setText(QString::fromLocal8Bit("恢复"));
            pushButton->setChecked(true);
        } else {
            pushButton->setText(QString::fromLocal8Bit("隐藏"));
            pushButton->setChecked(false);
        }
    });

    layout->addWidget(labelCode, 3);
    layout->addWidget(labelDescription, 8);
    layout->addWidget(pushButton, 2);

    ui->scrollContainer->insertWidget(0, widget);

    warningItems.push_back(vect);
}

void SettingWidget::resetAllWarning() {
    if (ui->selectErrorBtn->isChecked()) {
        for (auto &&item : warningItems) {
            auto pushButton = (QPushButton *)item.at(3);
            pushButton->setText(QString::fromLocal8Bit("隐藏"));
            pushButton->setChecked(false);
        }
    }
}

void SettingWidget::readSetting() {
    auto setting = JsonUtil::read(SETTING_PATH);

    auto fontFamily = setting["fontFamily"].toString();
    auto fontStyle = setting["fontStyle"].toString();
    auto fontSize = setting["fontSize"].toString();

    ui->selectedFont->setText(fontFamily);
    ui->selectedFontStyle->setText(fontStyle);
    ui->selectedFontSize->setText(fontSize);

    updateFontPreview();

    auto analyzeConfig = JsonUtil::read(ANALYZE_CONFIG_PATH);

    auto scoreWeights = analyzeConfig["scoreWeights"].toObject();
    int normative = scoreWeights["normative"].toInt();
    int efficiency = scoreWeights["efficiency"].toInt();
    int maintainability = scoreWeights["maintainability"].toInt();
    int security = scoreWeights["security"].toInt();
    if(normative == 0){
        ui->NormalizationSlider->setValue(0);
        ui->NormalizationWeight->setValue(0);
    } else{
        ui->NormalizationSlider->setValue(normative);
    }
    if(efficiency == 0){
        ui->EfficiencySlider->setValue(0);
        ui->EfficiencyWeight->setValue(0);
    } else{
        ui->EfficiencySlider->setValue(efficiency);
    }
    if(maintainability == 0){
        ui->MaintainablitySlider->setValue(0);
        ui->MaintainablityWeight->setValue(0);
    } else{
        ui->MaintainablitySlider->setValue(maintainability);
    }
    if(security == 0){
        ui->SecuritySlider->setValue(0);
        ui->SecurityWeight->setValue(0);
    } else{
        ui->SecuritySlider->setValue(security);
    }
    auto maskedWarnings = analyzeConfig["maskedWarnings"].toArray();
    for (auto &&val : maskedWarnings) {
        int w_code = val.toInt();
        insertWarningItem(w_code, "", true);
    }
}

void SettingWidget::saveSetting() {
    auto setting = JsonUtil::read(SETTING_PATH);

    setting["fontFamily"] = ui->selectedFont->text();
    setting["fontStyle"] = ui->selectedFontStyle->text();
    setting["fontSize"] = ui->selectedFontSize->text();

    JsonUtil::write(SETTING_PATH, setting);

    auto analyzeConfig = JsonUtil::read(ANALYZE_CONFIG_PATH);
    auto scoreWeights = analyzeConfig["scoreWeights"].toObject();

    scoreWeights["normative"] = ui->NormalizationWeight->value();
    scoreWeights["efficiency"] = ui->EfficiencyWeight->value();
    scoreWeights["maintainability"] = ui->MaintainablityWeight->value();
    scoreWeights["security"] = ui->SecurityWeight->value();

    analyzeConfig["scoreWeights"] = scoreWeights;

    QJsonArray maskedWarnings;
    for (auto &&vect : warningItems) {
        auto pushButton = (QPushButton *)vect.at(3);
        if (pushButton->isChecked()) {
            maskedWarnings.append(pushButton->objectName());
        }
    }

    analyzeConfig["maskedWarnings"] = maskedWarnings;
    JsonUtil::write(ANALYZE_CONFIG_PATH, analyzeConfig);
}

void SettingWidget::on_Close_clicked() {
    delete this;
}

void SettingWidget::on_Cancel_clicked() {
    delete this;
}

void SettingWidget::on_Finish_clicked() {
    saveSetting();
    delete this;
}

void SettingWidget::on_listWidget_currentTextChanged(const QString &currentText) {
    ui->selectedFont->setText(currentText);
    updateFontPreview();
}

void SettingWidget::on_listWidget_2_currentTextChanged(const QString &currentText) {
    ui->selectedFontStyle->setText(currentText);
    updateFontPreview();
}

void SettingWidget::on_listWidget_3_currentTextChanged(const QString &currentText) {
    ui->selectedFontSize->setText(currentText);
    updateFontPreview();
}

void SettingWidget::on_lineEdit_textChanged(const QString &arg1) {
    if (arg1.isEmpty()) {
        for (auto &&vect : warningItems) {
            vect.at(0)->show();
        }
    } else {
        for (auto &&vect : warningItems) {
            auto labelCode = (QLabel *)vect.at(1);
            auto labelDescription = (QLabel *)vect.at(2);
            if (labelCode->text().contains(arg1) || labelDescription->text().contains(arg1))
                vect.at(0)->show();
            else
                vect.at(0)->hide();
        }
    }
}

void SettingWidget::on_pushButton_clicked() {
    ui->stackedWidget->setCurrentWidget(ui->font);
}

void SettingWidget::on_pushButton_2_clicked() {
    ui->stackedWidget->setCurrentWidget(ui->scoreWeight);
}

void SettingWidget::on_pushButton_3_clicked() {
    ui->stackedWidget->setCurrentWidget(ui->warning);
}

void SettingWidget::on_pushButton_4_clicked() {
    ui->stackedWidget->setCurrentWidget(ui->shortcut);
}

void SettingWidget::on_pushButton_5_clicked() {
    ui->stackedWidget->setCurrentWidget(ui->aboutUs);
}

void SettingWidget::on_selectErrorBtn_clicked() {
    resetAllWarning();
}

void SettingWidget::on_NormalizationSlider_valueChanged(int value)
{
//    if(value == 0)  value++;
    ui->NormalizationWeight->setValue(value);
}


void SettingWidget::on_EfficiencySlider_valueChanged(int value)
{
//    if(value == 0)  value++;
    ui->EfficiencyWeight->setValue(value);
}


void SettingWidget::on_MaintainablitySlider_valueChanged(int value)
{
//    if(value == 0)  value++;
    ui->MaintainablityWeight->setValue(value);
}


void SettingWidget::on_SecuritySlider_valueChanged(int value)
{
//    if(value == 0)  value++;
    ui->SecurityWeight->setValue(value);
}


void SettingWidget::on_NormalizationWeight_valueChanged(int arg1)
{
//    if(arg1 == 0)  arg1++;
    ui->NormalizationSlider->setValue(arg1);
}


void SettingWidget::on_EfficiencyWeight_valueChanged(int arg1)
{
//    if(arg1 == 0)  arg1++;
    ui->EfficiencySlider->setValue(arg1);
}


void SettingWidget::on_MaintainablityWeight_valueChanged(int arg1)
{
//    if(arg1 == 0)  arg1++;
    ui->MaintainablitySlider->setValue(arg1);
}


void SettingWidget::on_SecurityWeight_valueChanged(int arg1)
{
//    if(arg1 == 0)  arg1++;
    ui->SecuritySlider->setValue(arg1);
}

