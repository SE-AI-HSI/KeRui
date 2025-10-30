#ifndef SETTINGWIDGET_H
#define SETTINGWIDGET_H

#include <QWidget>
#include <vector>

namespace Ui {
class SettingWidget;
}

class SettingWidget : public QWidget {
    Q_OBJECT

public:
    explicit SettingWidget();
    ~SettingWidget();
    static QString getGlobalFontQSS();

    ///Normalization Efficiency Maintanability Security
    static QVector<int> getScoreWeights();

private slots:
    void on_Close_clicked();

    void on_Cancel_clicked();

    void on_Finish_clicked();

    void on_listWidget_currentTextChanged(const QString &currentText);

    void on_listWidget_2_currentTextChanged(const QString &currentText);

    void on_listWidget_3_currentTextChanged(const QString &currentText);

    void on_lineEdit_textChanged(const QString &arg1);

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_selectErrorBtn_clicked();

    void on_NormalizationSlider_valueChanged(int value);

    void on_EfficiencySlider_valueChanged(int value);

    void on_MaintainablitySlider_valueChanged(int value);

    void on_SecuritySlider_valueChanged(int value);

    void on_NormalizationWeight_valueChanged(int arg1);

    void on_EfficiencyWeight_valueChanged(int arg1);

    void on_MaintainablityWeight_valueChanged(int arg1);

    void on_SecurityWeight_valueChanged(int arg1);

private:
    void setupLayout();
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void updateFontPreview();
    void insertWarningItem(int w_code, const QString& description, bool is_masked);
    void resetAllWarning();
    void readSetting();
    void saveSetting();

private:
    Ui::SettingWidget *ui;
    std::vector<std::vector<QWidget*>> warningItems;

    bool isDragging = false;
    QPoint mousePressPos;
};

#endif // SETTINGWIDGET_H
