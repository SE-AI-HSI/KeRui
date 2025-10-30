#ifndef REMOTEDB_H
#define REMOTEDB_H

#include "FileManager.h"
#include "LocalDB.h"
#include <QApplication>
#include <QMessageBox>
#include <QTime>

class RemoteDB {
public:
    typedef std::function<void(bool)> Callback;

    static bool init(Callback callback);
    static bool connect(bool disconnect_on_return = false, bool update_coin = true);

    static bool insertCodes(QDateTime t_summit, QString fileName, const QString &code);
    static bool insertLog(QDateTime t_summit, const QString &log, bool exception = false);
    static bool insertUsingTime(QDateTime opentime, QDateTime finishtime);

private:
    static QSqlDatabase m_db;
    static Callback updateCoin;

    static bool doQuery(QSqlQuery &query, const QString &where);

    static bool doTransaction(QSqlQuery &query, const QString &where);
};

class DBManager {
public:
    DBManager();

    static bool initDB(RemoteDB::Callback callback); // 初始化本地数据库和连接远程数据库

    static bool pushLocalDataToRemote(); // 将本地没有上传的信息传递到远程数据库

    static bool newAnalyze(QStringList &filePath); // 传过来每个文件的路径，根据路径获取文件内容

    static bool successfulAnalyze(); // 成功分析文件

    static bool pushUsingTime(); // 储存使用时间

    static QString getLogPath();

    static std::ofstream *getLogFilePointer();

    static void writeRemoteDBLog(const QString &text);

    static void writeLocalDBLog(const QString &text);

private:
    static QDateTime m_timestamp;
    static QDateTime m_opentime;
    static std::ofstream *m_log;
    static QString now_logpath;
};

#endif // REMOTEDB_H
