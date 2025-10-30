#include "UrlInterpreter.h"
#include "Utils.h"
#include <QDebug>
#include <QtCore/QRegExp>
#include <QString>
#include <QWidget>

#define Qf(s) QString::fromLocal8Bit(s)
#define Qn(i) QString::number(i)
#define Endl "<br>" // html格式换行符,用\n会导致超链接失效//

UrlInterpreterEventproxy UrlInterpreter::eventProxy;

void UrlInterpreter::interpret(QObject *const source, QObject *const sourceParent, const QUrl &qUrl) {
    QString url = qUrl.toString();
    auto numbers = extractNumbers(url);

    switch (numbers.at(0)) {
    case JUMP_TO_ROW:
        emit eventProxy.jumpToLines(sourceParent, numbers.at(1));
        break;
    case JUMP_TO_ROW_COLUMN:
        emit eventProxy.jumpToLines(sourceParent, numbers.at(1), numbers.at(2));
        break;
    case JUMP_TO_BLOCK:
        emit eventProxy.jumpToLines(sourceParent, numbers.at(1), numbers.at(2), numbers.at(3));
        break;
    case TO_PREV_LABEL:
        emit eventProxy.jumpToLabel(source, sourceParent, false);
        break;
    case TO_NEXT_LABEL:
        emit eventProxy.jumpToLabel(source, sourceParent, true);
        break;
    case TO_SPECIFIC_LABEL:
        emit eventProxy.jumpToLabel(source, sourceParent, false, numbers.at(1));
        break;
    case OPTIMIZE_CLONE:
        emit eventProxy.optimizeClone(source, sourceParent, numbers.at(1), numbers.at(2));
        break;
    case MASK_WARNING:
        emit eventProxy.maskWarning(sourceParent, numbers.at(1));
        break;
    case JUMP_TO_FILE:
        emit eventProxy.jumpToFile(url.mid(url.indexOf('/') + 1));
        break;
    default:
        qDebug() << __FILE__ << ":" << __LINE__ << "Encountered incorrect URL definition:" << url;
        assert(false);
    }
}

QString UrlInterpreter::urlOfJumpToRow(int row) {
    return format("<a href=\"{}/{}\">[第{}行]</a>", QStringList() << JUMP_TO_ROW << row << row);
}

QString UrlInterpreter::urlOfJumpToRowColumn(int row, int col) {
    return format("<a href=\"{}/{}/{}>[行: {} 列: {}]</a>", QStringList()
                                                                << JUMP_TO_ROW_COLUMN << row << col << row << col);
}

QString UrlInterpreter::urlOfJumpToBlockBegin(int rowS, int colS, int rowE, int colE) {
    return format("<a href=\"{}/{}/{}/{}\">[行: {} 列: {}]</a><br>",
                  QStringList() << JUMP_TO_BLOCK << rowS << colS << rowE - rowS + 1 << rowS << colS);
}

QString UrlInterpreter::urlOfJumpToBlockEnd(int rowS, int columnS, int rowE, int columnE) {
    return format("<a href=\"{}/{}/{}/{}\">[行: {} 列: {}]</a><br>",
                  QStringList() << JUMP_TO_BLOCK << rowS << columnS << rowE - rowS + 1 << rowE << columnE);
}

QString UrlInterpreter::urlOfToPrevLabel() {
    return format("{}", QStringList() << TO_PREV_LABEL);
}

QString UrlInterpreter::urlOfToNextLabel() {
    return format("{}", QStringList() << TO_NEXT_LABEL);
}

QString UrlInterpreter::urlOfToSpecificLabel(int index) {
    return format("{}/{}", QStringList() << TO_SPECIFIC_LABEL << index);
}

QString UrlInterpreter::urlOfOptimizeClone(int block_beg_row, int block_beg_col) {
    return format("<br><a href=\"{}/{}/{}\">【测试】点击优化重复块</a><br>",
                  QStringList() << OPTIMIZE_CLONE << block_beg_row << block_beg_col);
}

QString UrlInterpreter::urlOfMaskWarning(int w_code) {
    return format(" <a href=\"{}/{}\">|过滤该类警告|</a><br>", QStringList() << MASK_WARNING << w_code);
}

QString UrlInterpreter::urlOfJumpFile(QString file_path) {
    return format("<a href=\"{}/{}\">{}</a><br>",
                  QStringList() << JUMP_TO_FILE << file_path << file_path.mid(file_path.lastIndexOf("/") + 1));
}
