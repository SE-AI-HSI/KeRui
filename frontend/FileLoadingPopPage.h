#ifndef FILELOADINGPOPPAGE_H
#define FILELOADINGPOPPAGE_H

#include <QThread>
#include <QTimer>
#include <QWidget>

namespace Ui {
class FileLoadingPopPage;
}

class FileLoadingPopPage : public QWidget {
    Q_OBJECT

public:
    explicit FileLoadingPopPage(QWidget *parent = nullptr);
    ~FileLoadingPopPage();
    void popUp();
    void pushDown();

private:
    Ui::FileLoadingPopPage *ui;
    int stepFlag = 0;
    int timeID;

    void handleTimeout(); // 超时处理函数
    //    virtual void timerEvent( QTimerEvent *event);

    //    QString imageFullUrl = "image: url(:/loadingPage/images/loadingPage/dot(full).svg);";
    //    QString imageEmptyUrl = "image: url(:/loadingPage/images/loadingPage/dot(empty).svg);";
};

// class FileLoadingThread : public QThread{
//     Q_OBJECT
// public:
//     FileLoadingThread(QWidget* widget,QObject *parent = nullptr);
//     ~FileLoadingThread();
//     void setPP();

// private:
//     void run() override;
//     QTimer *timer;
//     FileLoadingPopPage * fileLoadingPopPage;
// };

#endif // FILELOADINGPOPPAGE_H
