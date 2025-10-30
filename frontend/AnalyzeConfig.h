#ifndef ANALYZECONFIG_H
#define ANALYZECONFIG_H

#include <QJsonObject>

/**
 * @class AnalyzeConfig
 * @brief 分析配置，如四个维度的分数权重和过滤的警告
 */

class AnalyzeConfig {
public:
    static void maskWarning(int w_code);
    static void resumeWarning(int w_code);

private:
    static QJsonObject readConfig();
    static void writeConfig(const QJsonObject &obj);
    static QJsonObject createDefaultConfig();
};

#endif // ANALYZECONFIG_H
