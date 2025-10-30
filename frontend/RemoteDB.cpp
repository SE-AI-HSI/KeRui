#include "RemoteDB.h"

QSqlDatabase RemoteDB::m_db;
RemoteDB::Callback RemoteDB::updateCoin;

QDateTime DBManager::m_timestamp;
QDateTime DBManager::m_opentime;
std::ofstream *DBManager::m_log;
QString DBManager::now_logpath = "";

bool RemoteDB::init(Callback callback) {
    updateCoin = callback;
    m_db = QSqlDatabase::addDatabase("QMYSQL", "remote");
    return RemoteDB::connect(true);
}

bool RemoteDB::connect(bool disconnect_on_return, bool update_coin) {
    m_db.close();
    m_db.setHostName("42.193.190.147");  // IP地址
    m_db.setDatabaseName("krui");        // 数据库名
    m_db.setUserName("kerui");           // 用户名
    m_db.setPassword("202030500394");    // 密码
    m_db.setPort(3306);
    if (!m_db.open()) {
        QString err = "RemoteDB::connect: " + m_db.lastError().text();
        qDebug() << err;
        DBManager::writeRemoteDBLog(err);
        // QMessageBox::critical(0, QObject::tr("Database error"), m_db.lastError().text());
        if (update_coin)
            updateCoin(false);
        return false;
    } else {
        qDebug() << "Remote DB is available now.";
        if (update_coin)
            updateCoin(true);
        if (disconnect_on_return)
            m_db.close();
        return true;
    }
}

bool RemoteDB::doQuery(QSqlQuery &query, const QString &where) {
    bool success = query.exec();  /// exec
    QString err = query.lastError().text();

    if (!success) {
        qDebug() << where << err;
        DBManager::writeRemoteDBLog(where + ": " + err);
    }
    return success;
}

bool RemoteDB::doTransaction(QSqlQuery &query, const QString &where) {
    if (!m_db.transaction())
        return false;

    bool success = query.exec();  /// exec

    QString query_err = query.lastError().text();
    QString db_err = m_db.lastError().text();

    if (!success) {
        qDebug() << where << "err1" << query_err;
        DBManager::writeRemoteDBLog(where + ": err1 " + query_err);
        if (!m_db.rollback()) {
            qDebug() << where << "err2" << db_err;
            DBManager::writeRemoteDBLog(where + ": err2 " + db_err);
        }
    }
    if (!m_db.commit()) {
        qDebug() << where << "err3" << db_err;
        DBManager::writeRemoteDBLog(where + ": err3 " + db_err);
        return false;
    }
    return success;
}

bool RemoteDB::insertCodes(QDateTime t_summit, QString fileName, const QString &code) {
    updateCoin(true);
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO CODE_FILES(t_summit,file_name,mac,code) VALUES(?,?,?,?)");
    query.addBindValue(t_summit.toString("yyyy-MM-dd hh:mm:ss.zzz"));
    query.addBindValue(fileName);
    query.addBindValue(LocalDB::getMACAddress());
    query.addBindValue(code);
    return doTransaction(query, "RemoteDB::insertCodes");

    return false;
}

bool RemoteDB::insertLog(QDateTime t_summit, const QString &log, bool exception) {
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO LOG_FILES(t_summit,mac,log,exception) VALUES(?,?,?,?)");
    query.addBindValue(t_summit.toString("yyyy-MM-dd hh:mm:ss.zzz"));
    query.addBindValue(LocalDB::getMACAddress());
    query.addBindValue(log);
    query.addBindValue(exception);
    return doTransaction(query, "RemoteDB::insertLog");

    return false;
}

bool RemoteDB::insertUsingTime(QDateTime opentime, QDateTime finishtime) {
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO USINGTIME(mac,open_time, finish_time) VALUES(?,?,?)");
    query.addBindValue(LocalDB::getMACAddress());
    query.addBindValue(DateTime2Str(opentime));
    query.addBindValue(DateTime2Str(finishtime));
    return doTransaction(query, "RemoteDB::insertUsingTime");

    return false;
}

bool DBManager::initDB(RemoteDB::Callback callback) {
    m_opentime = QDateTime::currentDateTime();  // 开始初始化打开软件的时间
    LocalDB::init();
    return RemoteDB::init(callback);
}

bool DBManager::pushLocalDataToRemote() {
    if (!RemoteDB::connect(false, false))
        return false;
    else {
        // 读取本地数据库里面的文件
        // codes
        QList<LocalDB::Code> localCodes = LocalDB::readCodes();
        // qDebug() <<"localCodes numbers:" << localCodes.size();
        for (auto code : localCodes) {
            // 依次存入远程数据库
            if (RemoteDB::insertCodes(code.t_summit, code.file_name, code.code)) {
                LocalDB::deleteCodes(code.t_summit);
            }
        }
        // log
        // 分析的log文件和记录是一一对应的
        QList<LocalDB::Log> localLogs = LocalDB::readLogs();
        // qDebug() << "localLogs numbers:" << localLogs.size();
        QList<LocalDB::AnalyzeRecord> localAnalyzeRecords = LocalDB::readAnalyzeRecords();
        for (int i = 0; i < localLogs.size(); ++i) {
            if (!localAnalyzeRecords[i].finish) {
                // 没有分析结束，说明异常中断，此时的log记录没有内容，需要自己读取！
                // 要保存log路径才行！
                if (RemoteDB::insertLog(localLogs[i].t_summit, FileManager::read(localLogs[i].path, true),
                                        localLogs[i].exception)) {
                    LocalDB::deleteLog(localLogs[i].t_summit);
                    LocalDB::deleteAnalyzeRecord(localLogs[i].t_summit);
                }
            } else {
                // 分析正常结束
                // 直接上传log到远程即可
                if (RemoteDB::insertLog(localLogs[i].t_summit, localLogs[i].log, localLogs[i].exception)) {
                    LocalDB::deleteLog(localLogs[i].t_summit);
                    LocalDB::deleteAnalyzeRecord(localLogs[i].t_summit);
                }
            }
        }
        // 使用时间
        QList<LocalDB::UsingTime> usingtimes = LocalDB::readUsingTimes();
        // qDebug() << "usingtimes numbers:" << usingtimes.size();
        for (auto usingtime : usingtimes) {
            if (RemoteDB::insertUsingTime(usingtime.open_time, usingtime.finish_time)) {
                LocalDB::deleteUsingTime(usingtime.open_time);
            }
        }
    }
    return true;
}

bool DBManager::newAnalyze(QStringList &filepaths) {
    m_timestamp = QDateTime::currentDateTime();  // 每次分析文件前初始化时间戳
    if (!RemoteDB::connect()) {                  // 远程连接不上，代码存到本地
        // qDebug() << "codes >> localDB";
        for (QString path : filepaths) {
            QString name = path.right(path.size() - FileManager::getCurrentPath().size() - 1);  // 截取文件名
            LocalDB::insertCode(m_timestamp, name, FileManager::read(path, true));
        }
    } else {  // 远程连接上了，代码存到远程
        for (QString path : filepaths) {
            QString name =
                path.right(path.size() - FileManager::getCurrentPath().size() - 1);  // 通过文件路径截取文件名部分
            RemoteDB::insertCodes(m_timestamp, name, FileManager::read(path, true));
        }
    }
    // 先写好log文件，在开头标时间戳——但是log文件的文件名貌似是开始分析的时间戳
    now_logpath =
        QCoreApplication::applicationDirPath() + "/log/" + m_timestamp.toString("yyyy-MM-dd_hh.mm.ss") + ".txt";
    m_log = FileManager::write(now_logpath, "");
    // 存入本地的分析记录
    LocalDB::insertAnalyzeRecord(m_timestamp, false);
    return false;
}

bool DBManager::successfulAnalyze() {  // 若是连接到远程,删掉本次分析记录，把log直接传到远程
                                       // 若是没有连接到远程，需要把本地记录改成成功，把log文件直接读出来保存在本地
    if (RemoteDB::connect()) {
        LocalDB::deleteAnalyzeRecord(m_timestamp);
        return RemoteDB::insertLog(m_timestamp, FileManager::read(now_logpath, true), true);
    } else {
        LocalDB::deleteAnalyzeRecord(m_timestamp);
        LocalDB::insertAnalyzeRecord(m_timestamp, true);
        return LocalDB::insertLog(m_timestamp, now_logpath, FileManager::read(now_logpath, true), true);
    }
}

bool DBManager::pushUsingTime() {
    if (!RemoteDB::connect()) {  // 远程连接不上，存到本地
        //        qDebug() << "using time >> localDB";
        return LocalDB::insertUsingTime(m_opentime, QDateTime::currentDateTime());
    } else {  // 远程连接上了，存到远程
        return RemoteDB::insertUsingTime(m_opentime, QDateTime::currentDateTime());
    }
}

QString DBManager::getLogPath() {
    return now_logpath;
}

std::ofstream *DBManager::getLogFilePointer() {
    return m_log;
}

void DBManager::writeRemoteDBLog(const QString &text) {
    QFile file(QApplication::applicationDirPath() + "/DB_log.txt");
    file.open(QIODevice::Append | QIODevice::Text);
    QTextStream out(&file);
    out << "[Remote]: |" << QDateTime::currentDateTime().toString() << "| " << text;
    out << "\n\n";
    file.close();
}

void DBManager::writeLocalDBLog(const QString &text) {
    QFile file(QApplication::applicationDirPath() + "/DB_log.txt");
    file.open(QIODevice::Append | QIODevice::Text);
    QTextStream out(&file);
    out << "[Local]: |" << QDateTime::currentDateTime().toString() << "| " << text;
    out << "\n\n";
    file.close();
}
