#ifndef ANALYZERCLIENT_H
#define ANALYZERCLIENT_H

#include "MainWidget.h"
#include "Serverex.h"
#include "Toast.h"
#include <QMainWindow>

class AnalyzerClientEventProxy;

QT_BEGIN_NAMESPACE
namespace Ui {
class AnalyzerClient;
}
QT_END_NAMESPACE

class AnalyzerClient : public QMainWindow {
    Q_OBJECT

public:
    AnalyzerClient(QWidget *parent = nullptr);
    ~AnalyzerClient();

    void updateCoin(bool connected);

    void connectMessageSender();

public slots:
    void messagesTansfor(QStringList messages);

private slots:
    void on_Maximize_clicked();

    void on_Minimize_clicked();

    void on_Close_clicked();

private:
    /// events
    void connectEvents();
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    Ui::AnalyzerClient *ui;
    MainWidget *mainWidget;

    bool isDragging = false;
    QPoint mousePressPos;
    Toast toast;

public:
    static AnalyzerClientEventProxy eventProxy;
    static QRect windowRect;
    static Server *server;
};

//////////    /////////  //    ///  /////////  //////     /////////  ///////
//////   //     ///      //   ///     ///      ///   //   ///        ///    //
/////   //     ///       //  ///     ///      ///   //   /////////  ///////
////   //     ///         /////     ///      ///   //   ///        /////
///////   /////////       ///   /////////  ///////    /////////  ///  ////

class AnalyzerClientEventProxy : public QObject {
    Q_OBJECT
signals:
    void showToast(Toast::Type t, QString text);
};

#endif // ANALYZERCLIENT_H
