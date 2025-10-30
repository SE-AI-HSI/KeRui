#include "jsonutil.h"

QJsonObject JsonUtil::read(const QString &jsonPath) {
    QFile file(jsonPath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(nullptr, u8"警告", u8"丢失JSON文件: \n" + jsonPath);
        return {};
    }
    QTextStream stream(&file);
    stream.setCodec("UTF-8");

    QString jsonText = stream.readAll();
    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonText.toUtf8(), &jsonError);

    if (jsonError.error != QJsonParseError::NoError && !doc.isNull()) {
        QMessageBox::critical(nullptr, u8"错误",
                              u8"解析JSON(" + jsonPath + u8")失败. 错误信息: \n" + jsonError.errorString());
        return {};
    }

    file.close();
    return doc.object();
}

bool JsonUtil::write(const QString &jsonPath, const QJsonObject &obj) {
    QFile file(jsonPath);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(nullptr, u8"错误", u8"写入JSON文件(" + jsonPath + u8")失败, 检查文件权限。");
        return false;
    }
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson();

    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream << data;

    file.close();
    return true;
}
