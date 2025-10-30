#include "Utils.h"

QString format(const char *_src, QStringList args) {
    QString src = QString::fromLocal8Bit(_src);
    for (QString &arg : args) {
        int pos = src.indexOf("{");
        if (pos < 0) {
            qDebug() << __FILE__ << ":" << __LINE__
                     << "Too much args in function \"format(QString src, QStringList args)\"";
            assert(false);
        }
        if (src[pos + 1] != QChar('}')) {
            qDebug() << __FILE__ << ":" << __LINE__
                     << "Invalid template in function \"format(QString src, QStringList args)\"";
            assert(false);
        }
        src.replace(pos, 2, arg);
    }
    return src;
}

QVector<int> extractNumbers(QString src) {
    QVector<int> numbers;
    QRegularExpression re("\\d+");
    QRegularExpressionMatchIterator it = re.globalMatch(src);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        numbers.append(match.captured(0).toInt());
    }

    return numbers;
}

QStringList operator<<(QStringList list, char c) {
    list << QString(QChar(c));
    return list;
}

QStringList operator<<(QStringList list, int num) {
    list << QString::number(num);
    return list;
}

QString format(QString src, QStringList args)
{
    for (QString &arg : args) {
        int pos = src.indexOf("{");
        if (pos < 0) {
            qDebug() << __FILE__ << ":" << __LINE__
                     << "Too much args in function \"format(QString src, QStringList args)\"";
            assert(false);
        }
        if (src[pos + 1] != QChar('}')) {
            qDebug() << __FILE__ << ":" << __LINE__
                     << "Invalid template in function \"format(QString src, QStringList args)\"";
            assert(false);
        }
        src.replace(pos, 2, arg);
    }
    return src;
}
