#ifndef UTILS_H
#define UTILS_H
#include <QDebug>
#include <QRegularExpression>
#include <QString>
#include <QStringList>

QStringList operator<<(QStringList list, int num);
QStringList operator<<(QStringList list, char c);

/**
 * @usage format("{}: {}", QStringList() << Line << 12)
 * @return "Line: 12"
 */
QString format(const char *_src, QStringList args);
QString format(QString src, QStringList args);

QVector<int> extractNumbers(QString src);

#endif // UTILS_H
