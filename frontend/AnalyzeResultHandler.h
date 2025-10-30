#ifndef ANALYZERESULTHANDLER_H
#define ANALYZERESULTHANDLER_H

#include "AnalysisLabel.h"
#include "FileManager.h"
#include "InfoStruct.h"
#include <QLinkedList>
#include <QMultiHash>
#include <QObject>

/**
 * @class AnalyzeResultHandler
 * @brief 接收EventCenter返回的结果,并将其格式化为LabelData,
 * 然后通过newLabelData(LabelData* data)信号发送出去
 */

class AnalyzeResultHandler : public QObject {
    Q_OBJECT
public:
    AnalyzeResultHandler();

    void InitialARHandler();

    /** 获取文件对应分析信息 **/
    const QList<IWarningInfo> &getFileWarningInfo(QString filePath);
    const QList<IErrorInfo> &getFileErrorInfo(QString filePath);
    const QVector<QPair<int, QList<QList<ClonePosition>>>> &getFileCloneSets();
    std::vector<CodeGenerateInfo> &getCodeGenerateInfo();
    QList<int> getFileSummary(const QString &filePath);

public slots:
    // 获得EventCenter传来的信息//
    void CatchWarning(IWarningInfo winfo);
    void CatchError(IErrorInfo einfo);
    void CatchGlobalInfo(GlobalInfo ginfo);
    void CatchAnaStep(AnalyzeStep stepInfo);
    void CatchCloneSet(CloneSetInfo cloneSet);
    void CatchClonePosotion(ClonePosition clonePosition);
    void CatchCodeGenInfo(CodeGenerateInfo cginfo);
    void CatchFileTable(FileTable filetable);
    void CatchCodeNamingInfo(CodeNamingInfo cNaminginfo);

private:
    QMultiHash<int, IWarningInfo> warningInfoHash;
    QList<IWarningInfo> fileWarningInfo;
    QMultiHash<int, IErrorInfo> errorInfoHash;
    QList<IErrorInfo> fileErrorInfo;
    std::vector<CodeGenerateInfo> codeGenerateInfo;
    QVector<QPair<int, QList<QList<ClonePosition>>>> cloneSets; // 按文件整理成链表
    QMultiHash<int, ClonePosition> clonePositionHash;
    QVector<CodeNamingInfo> codeNamingInfo;
    GlobalInfo ginfo;

    // 生成个性化评价//
    QString GenerateComment(const GlobalInfo &ginfo);

signals:
    /**
     * @signal: scoreAndComment
     * @note: registerListener()参数为true才会发送该信号
     * @receiver: ScoreWidget
     * Normalization Efficiency Maintanability Security
     */
    void scoresAndComment(int total, int s1, int s2, int s3, int s4, QString comment);

    // 向mainwidget发送分析阶段信息
    void analyzeStep(AnalyzeStep);
};

#endif // ANALYZERESULTHANDLER_H
