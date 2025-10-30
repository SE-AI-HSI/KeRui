#ifndef LOCALDB_H
#define LOCALDB_H

#include <QDateTime>
#include <QList>
#include <QNetworkInterface>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>

inline QDateTime Str2DateTime(const QString &str) {
    return QDateTime::fromString(str, "yyyy-MM-dd hh:mm:ss.zzz");
}

inline QString DateTime2Str(const QDateTime &dt) {
    return dt.toString("yyyy-MM-dd hh:mm:ss.zzz");
}

class LocalDB {
public:
    struct Code;
    struct Log;
    struct AnalyzeRecord;
    struct UsingTime;

    LocalDB();

    static void init();
    static bool connect();
    static QString getMACAddress();

    static bool insertCode(const QDateTime &t_summit, const QString &file_name, const QString &code);
    static bool insertLog(const QDateTime &t_summit, const QString &log_path, const QString &log,
                          bool exception = false);
    static bool insertAnalyzeRecord(const QDateTime &t_summit, bool finish);
    static bool insertUsingTime(const QDateTime &t_open, const QDateTime &t_finish);

    static bool updateLog(const QDateTime &t_summit, const QString &log);
    static bool updateLog(const QDateTime &t_summit, bool exception);

    static bool deleteCodes(const QDateTime &t_summit);
    static bool deleteLog(const QDateTime &t_summit);
    static bool deleteAnalyzeRecord(const QDateTime &t_summit);
    static bool deleteUsingTime(const QDateTime &opentime);

    static QList<Code> readCodes();
    static QList<Log> readLogs();
    static QList<AnalyzeRecord> readAnalyzeRecords();
    static QList<UsingTime> readUsingTimes();

private:
    /**
     * @brief doTransaction 支持执行单个事务,SELECT语句无需使用事务,使用doQuery
     * @param query !!!必须是已经prepare()
     * @param where 调用该函数的位置
     * @return
     */
    static bool doTransaction(QSqlQuery &query, const QString &where = "");

    static bool doQuery(QSqlQuery &query, const QString &where = "");

public:
    static QSqlDatabase db;

    struct Code {
        QDateTime t_summit;
        QString file_name;
        QString code;
    };

    struct Log {
        QDateTime t_summit;
        QString path;
        QString log;
        bool exception;
    };

    struct AnalyzeRecord {
        QDateTime t_summit;
        bool finish;
    };

    struct UsingTime {
        QDateTime open_time;
        QDateTime finish_time;
    };
};

#endif // LOCALDB_H
