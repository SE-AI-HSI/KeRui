#include "LocalDB.h"
#include "RemoteDB.h"
#include <QDebug>

QSqlDatabase LocalDB::db;

LocalDB::LocalDB() {
}

bool LocalDB::connect() {
    if (db.isOpen())
        return true;
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("LocalDB");
    return db.open();
}

QString LocalDB::getMACAddress() {
    // 参考https://blog.csdn.net/weixin_44092851/article/details/124814913
    QList<QNetworkInterface> nets = QNetworkInterface::allInterfaces();
    for (QNetworkInterface &net : nets) {
        if (net.flags().testFlag(QNetworkInterface::IsUp) && net.flags().testFlag(QNetworkInterface::IsRunning) &&
            !net.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            for (int j = 0; j < net.addressEntries().size(); j++) {
                if (net.addressEntries().at(j).ip() != QHostAddress::LocalHost &&
                    net.addressEntries().at(j).ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    return net.hardwareAddress();
                }
            }
        }
    }
    return "";
}

bool LocalDB::insertCode(const QDateTime &t_summit, const QString &file_name, const QString &code) {
    if (!connect())
        return false;
    QSqlQuery query(db);
    query.prepare("INSERT INTO code_files(t_summit,file_name,code) VALUES(?,?,?)");
    query.addBindValue(DateTime2Str(t_summit));
    query.addBindValue(file_name);
    query.addBindValue(code);

    return doTransaction(query, "LocalDB::insertCode");
}

bool LocalDB::insertLog(const QDateTime &t_summit, const QString &log_path, const QString &log, bool exception) {
    if (!connect())
        return false;
    QSqlQuery query(db);
    query.prepare("INSERT INTO log_files(t_summit,path,log,exception) VALUES(?,?,?,?)");
    query.addBindValue(DateTime2Str(t_summit));
    query.addBindValue(log_path);
    query.addBindValue(log);
    query.addBindValue(exception);

    return doTransaction(query, "LocalDB::writeLog");
}

bool LocalDB::insertAnalyzeRecord(const QDateTime &t_summit, bool finish) {
    if (!connect())
        return false;
    QSqlQuery query(db);
    query.prepare("INSERT INTO analyze_record(t_summit,finish) VALUES(?,?)");
    query.addBindValue(DateTime2Str(t_summit));
    query.addBindValue(finish);

    return doTransaction(query, "LocalDB::writeAnalyzeRecord");
}

bool LocalDB::insertUsingTime(const QDateTime &t_open, const QDateTime &t_finish) {
    if (!connect())
        return false;
    QSqlQuery query(db);
    query.prepare("INSERT INTO using_time(open_time,finish_time) VALUES(?,?)");
    query.addBindValue(DateTime2Str(t_open));
    query.addBindValue(DateTime2Str(t_finish));

    return doTransaction(query, "LocalDB::insertUsingTime");
}

bool LocalDB::updateLog(const QDateTime &t_summit, const QString &log) {
    if (!connect())
        return false;
    QSqlQuery query(db);
    query.prepare("UPDATE log_files SET log=? WHERE t_summit=?");
    query.addBindValue(log);
    query.addBindValue(t_summit);

    return doTransaction(query, "LocalDB::updateLog");
}

bool LocalDB::updateLog(const QDateTime &t_summit, bool exception) {
    if (!connect())
        return false;
    QSqlQuery query(db);
    query.prepare("UPDATE log_files SET exception=? WHERE t_summit=?");
    query.addBindValue(exception);
    query.addBindValue(t_summit);

    return doTransaction(query, "LocalDB::updateLog(bool exception)");
}

bool LocalDB::deleteCodes(const QDateTime &t_summit) {
    if (!connect())
        return false;
    QSqlQuery query(db);
    query.prepare("DELETE FROM code_files WHERE t_summit=?");
    query.addBindValue(DateTime2Str(t_summit));

    return doTransaction(query, "LocalDB::deleteCodes");
}

bool LocalDB::deleteLog(const QDateTime &t_summit) {
    if (!connect())
        return false;
    QSqlQuery query(db);
    query.prepare("DELETE FROM log_files WHERE t_summit=?");
    query.addBindValue(DateTime2Str(t_summit));

    return doTransaction(query, "LocalDB::deleteLog");
}

bool LocalDB::deleteAnalyzeRecord(const QDateTime &t_summit) {
    if (!connect())
        return false;
    QSqlQuery query(db);
    query.prepare("DELETE FROM analyze_record WHERE t_summit=?");
    query.addBindValue(DateTime2Str(t_summit));

    return doTransaction(query, "LocalDB::deleteAnalyzeRecord");
}

bool LocalDB::deleteUsingTime(const QDateTime &opentime) {
    if (!connect())
        return false;
    QSqlQuery query(db);
    query.prepare("DELETE FROM using_time WHERE open_time=?");
    query.addBindValue(DateTime2Str(opentime));

    return doTransaction(query, "LocalDB::deleteUsingTime");
}

QList<LocalDB::Code> LocalDB::readCodes() {
    QList<Code> list;
    if (!connect())
        return list;

    QSqlQuery query(db);
    query.prepare("SELECT t_summit,file_name,code FROM code_files");
    doQuery(query, "LocalDB::readCodes");

    while (query.next()) {
        list.append(
            Code{Str2DateTime(query.value(0).toString()), query.value(1).toString(), query.value(2).toString()});
    }
    query.finish();
    return list;
}

QList<LocalDB::Log> LocalDB::readLogs() {
    QList<Log> list;
    if (!connect())
        return list;

    QSqlQuery query(db);
    query.prepare("SELECT t_summit,path,log,exception FROM log_files");
    doQuery(query, "LocalDB::readLogs");

    while (query.next()) {
        list.append(Log{Str2DateTime(query.value(0).toString()), query.value(1).toString(), query.value(2).toString(),
                        query.value(3).toBool()});
    }
    query.finish();
    return list;
}

QList<LocalDB::AnalyzeRecord> LocalDB::readAnalyzeRecords() {
    QList<AnalyzeRecord> list;
    if (!connect())
        return list;

    QSqlQuery query(db);
    query.prepare("SELECT t_summit,finish FROM analyze_record");
    doQuery(query, "LocalDB::readAnalyzeRecords");

    while (query.next()) {
        list.append(AnalyzeRecord{Str2DateTime(query.value(0).toString()), query.value(1).toBool()});
    }
    query.finish();
    return list;
}

QList<LocalDB::UsingTime> LocalDB::readUsingTimes() {
    QList<UsingTime> list;
    if (!connect())
        return list;

    QSqlQuery query(db);
    query.prepare("SELECT open_time,finish_time FROM using_time");
    doQuery(query, "LocalDB::readUsingTimes");

    while (query.next()) {
        list.append(UsingTime{Str2DateTime(query.value(0).toString()), Str2DateTime(query.value(1).toString())});
    }
    query.finish();
    return list;
}

bool LocalDB::doTransaction(QSqlQuery &query, const QString &where) {
    if (!db.transaction())
        return false;

    bool success = query.exec(); /// exec

    QString query_err = query.lastError().text();
    QString db_err = db.lastError().text();

    if (!success) {
        qDebug() << where << "err1" << query_err;
        DBManager::writeLocalDBLog(where + ": err1 " + query_err);
        if (!db.rollback()) {
            qDebug() << where << "err2" << db_err;
            DBManager::writeLocalDBLog(where + ": err2 " + db_err);
        }
    }
    if (!db.commit()) {
        qDebug() << where << "err3" << db_err;
        DBManager::writeLocalDBLog(where + ": err3 " + db_err);
        return false;
    }
    return success;
}

bool LocalDB::doQuery(QSqlQuery &query, const QString &where) {
    bool success = query.exec(); /// exec
    QSqlError err = query.lastError();

    if (!success) {
        qDebug() << where << err.text();
    }
    return success;
}

void LocalDB::init() {
    if (!connect())
        return;
    QStringList tables = db.tables();
    QSqlQuery query(db);
    if (!tables.contains("code_files")) {
        query.prepare(R"(
        CREATE TABLE code_files(
            t_summit DATETIME,
            file_name VARCHAR,
            code TEXT,
            PRIMARY KEY(t_summit, file_name)
        )
        )");
        if (doQuery(query, "LocalDB::initDB 1"))
            qDebug() << "Local DB created table code_files";
    }

    if (!tables.contains("log_files")) {
        query.prepare(R"(
        CREATE TABLE log_files(
            t_summit DATETIME PRIMARY KEY,
            path VARCHAR,
            log TEXT,
            exception BOOLEAN
        )
        )");
        if (doQuery(query, "LocalDB::initDB 2"))
            qDebug() << "Local DB created table log_files";
    }

    if (!tables.contains("analyze_record")) {
        query.prepare(R"(
        CREATE TABLE analyze_record(
            t_summit DATETIME PRIMARY KEY,
            finish BOOLEAN NOT NULL
        )
        )");
        if (doQuery(query, "LocalDB::initDB 3"))
            qDebug() << "Local DB created table analyze_record";
    }

    if (!tables.contains("using_time")) {
        query.prepare(R"(
        CREATE TABLE using_time(
            open_time DATETIME PRIMARY KEY,
            finish_time DATETIME
        )
        )");
        if (doQuery(query, "LocalDB::initDB 4"))
            qDebug() << "Local DB created table using_time";
    }
}
