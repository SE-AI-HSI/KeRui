#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "AnalyzeResultHandler.h"
#include "AnalyzeThread.h"
#include "AnalyzingPopPage.h"
#include "CodeBox.h"
#include "CodeEditor.h"
#include "FileLoadingPopPage.h"
#include "FileManager.h"
#include "RemoteDB.h"
#include "ScoreWidget.h"
#include "WelcomePage.h"
#include <QSplitter>
#include <QWidget>

namespace Ui {
class MainWidget;
}

class MainWidget : public QWidget {
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

    void connectSender();

private slots:
    void on_CloseProjectBtn_clicked();

private:
    // init
    void setupLayout();
    void setupStyle();
    // events
    void connectEvents();
    void resizeEvent(QResizeEvent *event);
    void receiveScoresAndComment(int scores[5], QString comment);
    // others
    void swapWelcomePage(bool showWelcomePage);

private:
    Ui::MainWidget *ui;
    WelcomePage *welcomePage;           // 没有打开文件时右侧显示的内容//
    AnalyzingPopPage *analyzingPopPage; // 分析的时候提示正在分析中//
    QSplitter *horizontalSplitter;      // 左边是FileManager,右边是verticalSplitter或者welcomePage//
    QSplitter *verticalSplitter;        // 顶部是ScoreWidget,中间是CodeBox,底部是未实现//
    FileManager *fileManager;
    ScoreWidget *scoreWidget;
    CodeBox *codeBox;

    AnalyzeThread *analyzeThread;                   // 分析线程//
    AnalyzeStopThread *analyzeStopThreas = nullptr; // 中断线程//
    FileLoadingPopPage *fileLoadingPopPage;
};

#endif // MAINWIDGET_H
