#include "CreateProjectWidget.h"
#include "AnalyzerClient.h"
#include "GraphicsEffectUtil.h"
#include "ui_CreateProjectWidget.h"
#include <QFileDialog>
#include <functional>

CreateProjectWidget::CreateProjectWidget(std::function<void(QString, QString)> _callback)
    : QWidget(nullptr), ui(new Ui::CreateProjectWidget) {
    ui->setupUi(this);
    setupLayout();
    connectEvents();

    QRect window_rect = AnalyzerClient::windowRect;
    show();
    move(window_rect.x() + (window_rect.width() - width()) / 2,
         window_rect.y() + (window_rect.height() - height()) / 2);

    callback = _callback;
}

CreateProjectWidget::~CreateProjectWidget() {
    delete ui;
}

void CreateProjectWidget::setupLayout() {
    setWindowModality(Qt::ApplicationModal);
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    // 阴影
    ui->widget->setGraphicsEffect(GraphicsEffectUtil::newShadow(this, 64));

    ui->label_4->hide();
    ui->label_5->hide();
    ui->label_6->hide();
}

void CreateProjectWidget::connectEvents() {
    connect(ui->lineEdit_2, &QLineEdit::textChanged, this, [=]() {
        ui->label_4->hide();
    });
    connect(ui->lineEdit_2, &QLineEdit::textChanged, this, [=]() {
        ui->label_5->hide();
        ui->label_6->hide();
    });
    connect(ui->lineEdit, &QLineEdit::textChanged, this, [=]() {
        int cnt = 1;
        while (existProjectName()) {
            ui->lineEdit_2->setText("CppApplication" + QString::number(cnt));
            cnt++;
        }
    });

    connect(ui->checkBox, &QCheckBox::stateChanged, this, [=](int _) {
        ui->checkBox_2->setVisible(ui->checkBox->isChecked());
    });
}

void CreateProjectWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && event->pos().y() < ui->TopBar->height()) {
        isDragging = true;
        mousePressPos = event->globalPos() - this->pos();
        event->accept();
    } else {
        QWidget::mousePressEvent(event);
    }
}

void CreateProjectWidget::mouseMoveEvent(QMouseEvent *event) {
    if (isDragging) {
        move(event->globalPos() - mousePressPos);
        event->accept();
    } else {
        QWidget::mouseMoveEvent(event);
    }
}

void CreateProjectWidget::mouseReleaseEvent(QMouseEvent *event) {
    isDragging = false;
    QWidget::mouseReleaseEvent(event);
}

bool CreateProjectWidget::existProjectName() {
    QString path = ui->lineEdit->text() + "/" + ui->lineEdit_2->text();
    QFile f(path);
    return f.exists();
}

void CreateProjectWidget::on_Close_clicked() {
    delete this;
}

void CreateProjectWidget::on_Cancel_clicked() {
    delete this;
}

void CreateProjectWidget::on_Finish_clicked() {
    if (ui->lineEdit_2->text().isEmpty()) {
        ui->label_4->show();
    } else {
        if (ui->lineEdit->text().isEmpty()) {
            ui->label_6->show();
        } else {
            QString path = ui->lineEdit->text() + "/" + ui->lineEdit_2->text();
            QDir dir(path);
            if (dir.exists()) {
                ui->label_5->show();
            } else {
                QDir().mkdir(path);
                if (ui->checkBox->isChecked()) {
                    QFile file(path + "/main.cpp");
                    file.open(QIODevice::WriteOnly | QIODevice::Text);

                    if (ui->checkBox_2->isChecked()) {
                        QTextStream out(&file);
                        out <<
                            R"(#include <iostream>
using namespace std;

int main()
{
    cout << "Hello Kerui!" << endl;
    return 0;
})";
                    }
                    file.close();
                    callback(path, "main.cpp");

                } else {
                    callback(path, "");
                }
                delete this;
            }
        }
    }
}

void CreateProjectWidget::on_pushButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("选择项目"), "/",
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        ui->lineEdit->setText(dir);
    }
}
