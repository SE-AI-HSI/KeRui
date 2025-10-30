#ifndef ANALYZETHREAD_H
#define ANALYZETHREAD_H

// #include "CppAnalyzer.h"
#include "Enums.h"
#include "LabelManager.h"
#include "Serverex.h"
#include <QThread>

class AnalyzeThreadEventProxy;

// static CppAnalyzer g_analyzer;

class AnalyzeThread : public QThread {
    Q_OBJECT
public:
    explicit AnalyzeThread(QObject *parent);
    ~AnalyzeThread();

private:
    void run() override;

signals:
    /**
     * @signal: progressSignal
     * @receiver: LabelManagerSlotProxy -> LabelManager
     */
    void progressSignal(AnalyzeProgress progress);

public:
    QString folderPath = "";
};

class AnalyzeStopThread : public QThread {
    Q_OBJECT
public:
    explicit AnalyzeStopThread(QObject *parent);
    ~AnalyzeStopThread();

private:
    void run() override;
};

//////////    /////////  //    ///  /////////  //////     /////////  ///////
//////   //     ///      //   ///     ///      ///   //   ///        ///    //
/////   //     ///       //  ///     ///      ///   //   /////////  ///////
////   //     ///         /////     ///      ///   //   ///        /////
///////   /////////       ///   /////////  ///////    /////////  ///  ////

class AnalyzeThreadEventProxy : public QObject {
    Q_OBJECT
public:
    static AnalyzeThreadEventProxy eventProxy;
signals:
    void progressChanged(AnalyzeProgress ap);
};

#endif // ANALYZETHREAD_H
