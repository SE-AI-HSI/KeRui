#include "AnalyzeThread.h"

#include <windows.h>

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QProcess>
#include <QTextCodec>

AnalyzeThreadEventProxy AnalyzeThreadEventProxy::eventProxy;

AnalyzeThread::AnalyzeThread(QObject *parent) : QThread{parent} {
}

AnalyzeThread::~AnalyzeThread() {
}

AnalyzeStopThread::AnalyzeStopThread(QObject *parent) : QThread{parent} {
}

AnalyzeStopThread::~AnalyzeStopThread() {
}

void AnalyzeStopThread::run() {
}

void AnalyzeThread::run() {
    emit AnalyzeThreadEventProxy::eventProxy.progressChanged(AnalyzeProgress::Initializing);

    QStringList args;
    QTextCodec *codec = QTextCodec::codecForName("GB2312");
    QByteArray encodedStr = codec->fromUnicode(folderPath);  // 将 QString 转换为 GB2312 编码
    QString decodedStr = codec->toUnicode(encodedStr);

    STARTUPINFOA si = {sizeof(si)};
    PROCESS_INFORMATION pi;

    // auto byteArray = QString("clang-cppana.exe -f %1 -port 12345 -nosimhash() -catchdiag -astopt -detailclone -clonemodel "
    //                          "-cloneIP 42.193.190.147 "
    //                          "-cloneport 25566 "
    //                          "-nodecl -nodeclstmt -noliteral -opnodediff -opnodetype -arrtype -detailfg -threshold 0.6")
    //                      .arg(folderPath)
    //                      .toLocal8Bit();

    auto byteArray = QString("clang-cppana.exe -f %1"
                             // "-cloneIP 42.193.190.147 "
                             // "-cloneport 25566 "
                             )
                         .arg(folderPath)
                         .toLocal8Bit();


    BOOL success = CreateProcessA(NULL, (LPSTR)(byteArray.constData()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

    if (!success) {
        qDebug() << __FILE__ << __LINE__ << ":" << GetLastError();
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    emit AnalyzeThreadEventProxy::eventProxy.progressChanged(AnalyzeProgress::Finished);
}
