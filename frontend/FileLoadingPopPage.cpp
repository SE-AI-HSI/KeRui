#include "FileLoadingPopPage.h"
#include "ui_FileLoadingPopPage.h"
#include <QDebug>

FileLoadingPopPage::FileLoadingPopPage(QWidget *parent) : QWidget(parent), ui(new Ui::FileLoadingPopPage) {
    ui->setupUi(this);
}

FileLoadingPopPage::~FileLoadingPopPage() {
    delete ui;
}

void FileLoadingPopPage::popUp() {
    timeID = startTimer(200);
}

void FileLoadingPopPage::pushDown() {
    killTimer(timeID);
}

// void FileLoadingPopPage::timerEvent(QTimerEvent *event)
//{//qDebug() << "???";
//     if(event->timerId() == timeID){
////        qDebug() << "???";
//        switch(stepFlag){
//        case 0 :
//            ui->dot_1->setStyleSheet(imageFullUrl);
//            ++stepFlag;
//            break;
//        case 1 :
//            ui->dot_2->setStyleSheet(imageFullUrl);
//            ++stepFlag;
//            break;
//        case 2 :
//            ui->dot_3->setStyleSheet(imageFullUrl);
//            ++stepFlag;
//            break;
//        case 3 :
//            ui->dot_4->setStyleSheet(imageFullUrl);
//            ++stepFlag;
//            break;
//        case 4 :
//            ui->dot_5->setStyleSheet(imageFullUrl);
//            ++stepFlag;
//            break;
//        case 5:
//            ui->dot_1->setStyleSheet(imageEmptyUrl);
//            ui->dot_2->setStyleSheet(imageEmptyUrl);
//            ui->dot_3->setStyleSheet(imageEmptyUrl);
//            ui->dot_4->setStyleSheet(imageEmptyUrl);
//            ui->dot_5->setStyleSheet(imageEmptyUrl);
//            stepFlag = 0;
//        }
//        QCoreApplication::processEvents();
//    }
//}

// FileLoadingThread::FileLoadingThread(QWidget* widget, QObject *parent):
//     QThread{parent},timer(new QTimer(this)), fileLoadingPopPage(new FileLoadingPopPage(widget))
//{
//         fileLoadingPopPage->setGeometry(0, 0, widget->width(), widget->height());
// }

// FileLoadingThread::~FileLoadingThread()
//{
//     timer->stop();
//     fileLoadingPopPage->hide();
// }

// void FileLoadingThread::run()
//{
//     timer->start();
//     fileLoadingPopPage->show();
//     QTimer::singleShot(500, this, [=](){
//         fileLoadingPopPage->updateImage();
//     });
// }
