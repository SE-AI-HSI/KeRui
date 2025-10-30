#include "AnalyzeResultHandler.h"

#include <QDebug>

#include "AnalyzeThread.h"

#define Qf(s) QString::fromLocal8Bit(s)
#define Qn(i) QString::number(i)
#define Endl "<br>"  // html格式换行符,用\n会导致超链接失效//

AnalyzeResultHandler::AnalyzeResultHandler() {
    if (false) {
    }
}

void AnalyzeResultHandler::InitialARHandler() {
    warningInfoHash.clear();
    errorInfoHash.clear();
    cloneSets.clear();
    codeGenerateInfo.clear();
    clonePositionHash.clear();
    FileManager::id2filePath.clear();
}

void AnalyzeResultHandler::CatchWarning(IWarningInfo winfo) {
    //    qDebug()<<"WarningInfo-filePath"<<Qf(winfo.m_file.c_str()).replace("\\","/");

    warningInfoHash.insert(winfo.m_file, winfo);
}

void AnalyzeResultHandler::CatchError(IErrorInfo einfo) {
    //    qDebug()<<"ErrorInfo-filePath" << Qf(einfo.m_file.c_str()).replace("\\","/");
    errorInfoHash.insert(einfo.m_file, einfo);
}

void AnalyzeResultHandler::CatchClonePosotion(ClonePosition cP) {
    clonePositionHash.insert(cP.m_file, cP);
}

void AnalyzeResultHandler::CatchAnaStep(AnalyzeStep stepInfo) {
    emit analyzeStep(stepInfo);
}

void AnalyzeResultHandler::CatchCloneSet(CloneSetInfo cS) {
    QPair<int, QList<QList<ClonePosition>>> newSet;
    newSet.first = cS.m_degree;
    newSet.second = QList<QList<ClonePosition>>();
    for (auto cP : cS.m_cloneSets) {
        bool flag = true;
        for (auto &list : newSet.second) {
            if (cP.m_file == list.first().m_file) {
                list.append(cP);
                flag = false;
                break;
            }
        }
        if (flag) {
            newSet.second.append(QList<ClonePosition>{cP});
        }
    }
    // 按照同一个文件放在一起
    cloneSets.append(newSet);
}

const QList<IWarningInfo> &AnalyzeResultHandler::getFileWarningInfo(QString filePath) {
    fileWarningInfo.clear();
    int fileIndex = 0;
    for (; fileIndex < FileManager::id2filePath.size(); ++fileIndex) {
        QString fileName = FileManager::id2filePath[fileIndex];
        if (fileName == filePath) {
            fileWarningInfo = warningInfoHash.values(fileIndex);
            //    qDebug() << warningInfoHash.keys();
            //    qDebug() << fileWarningInfo.size();
            return fileWarningInfo;
        }
    }
    return fileWarningInfo;  // empty
}

const QList<IErrorInfo> &AnalyzeResultHandler::getFileErrorInfo(QString filePath) {
    fileErrorInfo.clear();
    int fileIndex = 0;
    for (; fileIndex < FileManager::id2filePath.size(); ++fileIndex) {
        QString fileName = FileManager::id2filePath[fileIndex];
        if (fileName == filePath) {
            fileErrorInfo = errorInfoHash.values(fileIndex);
            //    qDebug() << fileErrorInfo.size();

            //    qDebug() << errorInfoHash.keys();
            return fileErrorInfo;
        }
    }
    return fileErrorInfo;  // empty
}

const QVector<QPair<int, QList<QList<ClonePosition>>>> &AnalyzeResultHandler::getFileCloneSets() {
    return cloneSets;
}

std::vector<CodeGenerateInfo> &AnalyzeResultHandler::getCodeGenerateInfo() {
    return codeGenerateInfo;
}

QList<int> AnalyzeResultHandler::getFileSummary(const QString &filePath) {
    QList<int> list;
    getFileErrorInfo(filePath);
    getFileWarningInfo(filePath);
    list << fileErrorInfo.size() << 0 << 0 << 0 << 0;
    for (IWarningInfo &s : fileWarningInfo) {
        if (s.m_warningtype & ScoreUnitType::NORMATIVE)
            ++list[1];
        if (s.m_warningtype & ScoreUnitType::EFFICIENCY)
            ++list[2];
        if (s.m_warningtype & ScoreUnitType::MAINTAINABILITY)
            ++list[3];
        if (s.m_warningtype & ScoreUnitType::SECURITY)
            ++list[4];
    }
    return list;
}

//Normalization Efficiency Maintanability Security
void AnalyzeResultHandler::CatchGlobalInfo(GlobalInfo ginfo) {
    emit scoresAndComment(ginfo.m_scores[4], ginfo.m_scores[0], ginfo.m_scores[1], ginfo.m_scores[3], ginfo.m_scores[2],
                          GenerateComment(ginfo));
}

void AnalyzeResultHandler::CatchCodeGenInfo(CodeGenerateInfo cginfo) {
    codeGenerateInfo.emplace_back(cginfo);
}

void AnalyzeResultHandler::CatchFileTable(FileTable filetable) {
    for (int i = 0; i < filetable.m_files.size(); ++i) {
//        qDebug() << __FILE__ << __LINE__ << ":" << i << filetable.m_files[i];
        QString filePath = filetable.m_files[i].c_str();
        filePath.replace('\\', '/');
        FileManager::id2filePath.insert(filetable.m_filesID[i],filePath);
//        qDebug() << i << filetable.m_files[i] << "!!!";
    }
}

void AnalyzeResultHandler::CatchCodeNamingInfo(CodeNamingInfo cNaminginfo) {
    codeNamingInfo.append(cNaminginfo);
}

QString AnalyzeResultHandler::GenerateComment(const GlobalInfo &ginfo) {
    // 总评//
    QString comment = "";
    if (ginfo.m_scores[4] > 90) {
        comment.append(QString::fromLocal8Bit("代码写的很不错哦,再接再厉!\n"));
    } else if (ginfo.m_scores[4] > 80) {
        comment.append(Qf("代码写的很挺好的,但还是有不少要改进的地方。\n"));
    } else if (ginfo.m_scores[4] > 60) {
        comment.append(Qf("代码存在的问题还有点多,要注意修改。\n"));
    } else {
        comment.append(Qf("代码的问题太多了,再尝试修改修改吧!\n"));
    }
    // 各项提示//
    QString problem = "";
    QString ser_problem = "";
    if (ginfo.m_scores[0] < 90) {
        problem.append(Qf("[规范性]"));
    } else if (ginfo.m_scores[0] < 75) {
        ser_problem.append(Qf("[规范性]"));
    }
    if (ginfo.m_scores[1] < 90) {
        problem.append(Qf("[执行效率]"));
    } else if (ginfo.m_scores[1] < 75) {
        ser_problem.append(Qf("[执行效率]"));
    }
    if (ginfo.m_scores[2] < 90) {
        problem.append(Qf("[可维护性]"));
    } else if (ginfo.m_scores[2] < 75) {
        ser_problem.append(Qf("[可维护性]"));
    }
    if (ginfo.m_scores[3] < 90) {
        problem.append(Qf("[安全性]"));
    } else if (ginfo.m_scores[3] < 75) {
        ser_problem.append(Qf("[安全性]"));
    }
    if (!problem.isEmpty()) {
        comment.append(Qf("请留意在"));
        comment.append(problem);
        comment.append(Qf("存在的问题。\n"));
    }
    if (!ser_problem.isEmpty()) {
        comment.append(Qf("其中要特别注意在"));
        comment.append(ser_problem);
        comment.append(Qf("存在的问题。\n"));
    }
    // 注释率问题//
    //    if (ginfo. < 5) {
    //        comment.append(Qf("同时要注意代码的注释太少了,请注意添加注释。\n"));
    //    } else if (ginfo.m_noterate < 10) {
    //        comment.append(Qf("代码的注释略有欠缺,请注意添加。\n"));
    //    } else if (ginfo.m_noterate < 30) {
    //        comment.append(Qf("最好可以在代码关键的地方再稍微加一些注释。\n"));
    //    }
    // 代码行数//
    if (ginfo.m_totalline > 400) {
        comment.append(Qf("代码行数较多,修改的时候要注意上下关系哦。\n"));
    }
    return comment;
}
