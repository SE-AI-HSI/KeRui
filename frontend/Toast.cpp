#include "Toast.h"

#include <QHBoxLayout>

Toast::Toast(QWidget *parent) {
    timer = new QTimer(parent);
    setupLayout(parent);
    setupStyle();
    connectEvents();
}

void Toast::showSuccess(QString success) {
    frame->setStyleSheet("QFrame{"
                         "color: white;"
                         "border-radius: 25px;"
                         "background-color: rgb(90, 166, 255);"
                         "}");
    icon->setStyleSheet("border-image: url(:/images/toast-success.svg);");

    text->setText(success);
    showToast();
}

void Toast::showWarning(QString warning) {
    frame->setStyleSheet("QFrame{"
                         "color: black;"
                         "border-radius: 25px;"
                         "background-color: rgb(255, 188, 57);"
                         "}");
    icon->setStyleSheet("border-image: url(:/images/toast-warning.svg);");

    text->setText(warning);
    showToast();
}

void Toast::showError(QString error) {
    frame->setStyleSheet("QFrame{"
                         "color: white;"
                         "border-radius: 25px;"
                         "background-color: rgb(255, 134, 96);"
                         "}");
    icon->setStyleSheet("border-image: url(:/images/toast-error.svg);");

    text->setText(error);
    showToast();
}

void Toast::showToast(Toast::Type t, const QString &text) {
    if (t == Success) {
        showSuccess(text);
    } else if (t == Warning) {
        showWarning(text);
    } else {
        showError(text);
    }
}

void Toast::setupLayout(QWidget *parent) {
    frame = new QFrame(parent);
    frame->setFixedHeight(50);
    QHBoxLayout *layout = new QHBoxLayout(frame);
    layout->setContentsMargins(30, 0, 30, 0);

    icon = new QLabel(frame);
    icon->setFixedSize(28, 28);

    text = new QLabel(frame);
    text->setAlignment(Qt::AlignCenter);

    layout->addWidget(icon);
    layout->addWidget(text);

    frame->hide();
}

void Toast::setupStyle() {
    opacityEffect = new QGraphicsOpacityEffect(frame);
    frame->setGraphicsEffect(opacityEffect);
}

void Toast::connectEvents() {
    connect(timer, &QTimer::timeout, this, [=] {
        if (frame->y() < 94) {
            frame->move(frame->x(), frame->y() + 1);
        } else if (alpha > 100) {
            alpha -= 1;
        } else if (alpha > 0) {
            alpha -= 2;
            opacityEffect->setOpacity(alpha * 1.0 / 100);
        } else {
            frame->hide();
            timer->stop();
        }
    });
}

void Toast::showToast() {
    frame->show();
    frame->adjustSize();
    frame->setFixedWidth(std::max(frame->width(), 300));
    frame->move((static_cast<QWidget *>(frame->parent())->width() - frame->width()) / 2, 60);
    frame->raise();
    alpha = 300;
    opacityEffect->setOpacity(1.0);
    timer->start(5);
}
