#include "AnalyzerClient.h"
#include "ui_AnalyzerClient.h"
#include <QScreen>
#include <QTimer>
#include <QVBoxLayout>

AnalyzerClientEventProxy AnalyzerClient::eventProxy;
QRect AnalyzerClient::windowRect;
Server *AnalyzerClient::server = nullptr;

AnalyzerClient::AnalyzerClient(QWidget *parent) : QMainWindow(parent), ui(new Ui::AnalyzerClient), toast(this) {
    ui->setupUi(this);
    mainWidget = new MainWidget(this);

    setWindowFlag(Qt::FramelessWindowHint);
    setWindowIcon(QIcon(":/images/logo.png"));

    // 设置layout不用重写resizeEvent来调整子窗口大小
    QVBoxLayout *layout = new QVBoxLayout(ui->Content);
    layout->addWidget(mainWidget);
    layout->setMargin(0);

    connectEvents();

    show();
    windowRect = geometry();
}

AnalyzerClient::~AnalyzerClient() {
    delete ui;
}

void AnalyzerClient::updateCoin(bool connected) {
    if (connected) {
        ui->DBCoin->setStyleSheet("image: url(:/upload/images/upload/success.svg);");
    } else {
        ui->DBCoin->setStyleSheet("image: url(:/upload/images/upload/failed.svg);");
    }
}

void AnalyzerClient::connectMessageSender() {
    mainWidget->connectSender();
}

void AnalyzerClient::messagesTansfor(QStringList messages) {
    qDebug() << QString::fromLocal8Bit("可以接收消息");
}

void AnalyzerClient::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && event->pos().y() < ui->TopBar->height()) {
        isDragging = true;
        mousePressPos = event->globalPos() - this->pos();
        event->accept();
    } else {
        QMainWindow::mousePressEvent(event);
    }
}

void AnalyzerClient::mouseMoveEvent(QMouseEvent *event) {
    if (isDragging) {
        move(event->globalPos() - mousePressPos);
        event->accept();
    } else {
        QMainWindow::mouseMoveEvent(event);
    }
}

void AnalyzerClient::mouseReleaseEvent(QMouseEvent *event) {
    isDragging = false;
    QMainWindow::mouseReleaseEvent(event);
    windowRect = geometry();
}

void AnalyzerClient::on_Maximize_clicked() { // 最大化
    if (this->geometry().width() < QGuiApplication::screens().at(0)->geometry().width()) {
        showMinimized();
        showMaximized();
    } else {
        showMaximized();
        showNormal();
    }
    QTimer::singleShot(250, this, [=] {
        windowRect = geometry();
    });
}

void AnalyzerClient::on_Minimize_clicked() { // 最小化
    showMinimized();
    QTimer::singleShot(250, this, [=] {
        windowRect = geometry();
    });
}

void AnalyzerClient::on_Close_clicked() { // 关闭
    if (server != nullptr)
        delete server;
    DBManager::pushUsingTime();
    QCoreApplication::quit();
}

void AnalyzerClient::connectEvents() {
    connect(&eventProxy, &AnalyzerClientEventProxy::showToast, this, [=](Toast::Type t, QString text) {
        toast.showToast(t, text);
    });
}
