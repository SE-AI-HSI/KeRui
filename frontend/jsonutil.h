#ifndef JSONUTIL_H
#define JSONUTIL_H
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMessageBox>
#include <QTextStream>

/*!
    @brief 提供读取和写入json文件的接口，当json文件丢失或解析失败时会弹出对话框提示
    @author GuoZhenjun
    @date 2023-05-09 09:58:33
*/

class JsonUtil {
public:
    /*!
    @brief 读取json文件，如果出现异常会返回一个空的QJsonObject
    @param jsonPath json文件绝对路径
    @return QJsonObject json文件的根对象
    @author GuoZhenjun
    @date 2023-05-09 10:40:26
    */
    static QJsonObject read(const QString &jsonPath);

    /*!
    @brief 将QJsonObject写入json文件
    @param jsonPath 待写入的json文件绝对路径
    @param obj 数据源
    @return bool 是否写入成功
    @author GuoZhenjun
    @date 2023-05-09 10:42:06
    */
    static bool write(const QString &jsonPath, const QJsonObject &obj);
};

#endif // JSONUTIL_H
