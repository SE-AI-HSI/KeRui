#include "AnalyzeConfig.h"
#include "FileManager.h"
#include <QCoreApplication>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>

#define CONFIG_PATH QCoreApplication::applicationDirPath() + "/AnalyzeConfig.json"

void AnalyzeConfig::maskWarning(int w_code) {
    QJsonObject cfg = readConfig();
    QJsonArray maskedWarnings = cfg["maskedWarnings"].toArray();
    for (const QJsonValue &v : maskedWarnings) {
        if (v.toInt() == w_code) {
            return;
        }
    }
    maskedWarnings.append(w_code);
    cfg["maskedWarnings"] = maskedWarnings;
    writeConfig(cfg);
}

void AnalyzeConfig::resumeWarning(int w_code) {
    QJsonObject cfg = readConfig();
    QJsonArray maskedWarnings = cfg["maskedWarnings"].toArray();
    for (int i = 0; i < maskedWarnings.size(); ++i) {
        if (maskedWarnings[i] == w_code) {
            maskedWarnings.removeAt(i);
            break;
        }
    }
    cfg["maskedWarnings"] = maskedWarnings;
    writeConfig(cfg);
}

QJsonObject AnalyzeConfig::readConfig() {
    QString cfg = FileManager::read(CONFIG_PATH, true);
    if (cfg.isEmpty()) {
        return createDefaultConfig();
    } else {
        QJsonParseError jsonError;
        QJsonDocument doc = QJsonDocument::fromJson(cfg.toUtf8(), &jsonError);

        if (jsonError.error != QJsonParseError::NoError && !doc.isNull()) {
            return createDefaultConfig();
        }
        return doc.object();
    }
}

void AnalyzeConfig::writeConfig(const QJsonObject &obj) {
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson();

    QFile file(CONFIG_PATH);
    file.open(QIODevice::WriteOnly);
    file.write(data);
    file.close();
}

QJsonObject AnalyzeConfig::createDefaultConfig() {
    QJsonObject rootObj;

    QJsonObject scoreWeights;
    scoreWeights["normative"] = 100;
    scoreWeights["efficiency"] = 100;
    scoreWeights["maintainability"] = 100;
    scoreWeights["security"] = 100;
    rootObj["scoreWeights"] = scoreWeights;

    QJsonArray maskedWarnings;
    rootObj["maskedWarnings"] = maskedWarnings;

    writeConfig(rootObj);
    return rootObj;
}
