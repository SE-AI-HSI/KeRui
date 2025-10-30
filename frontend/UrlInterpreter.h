#ifndef URLINTERPRETER_H
#define URLINTERPRETER_H
#include "UrlDef.h"
#include <QObject>
#include <QString>
#include <QUrl>

class UrlInterpreterEventproxy;

/**
 * @class UrlInterpreter
 * @brief 点击标签中的一个超链接,qt返回一个QUrl,这个类用于
 * ①生成描述各种操作(如跳转到某行,跳转到下一个重复块)的Url(实际上就是QString)
 * ②解释这些Url,返回一个数组和数组的长度,数组中包含了Url中的数据
 */

class UrlInterpreter {
public:
    static void interpret(QObject *const source, QObject *const sourceParent, const QUrl &qUrl);

    static QString urlOfJumpToRow(int row);
    static QString urlOfJumpToRowColumn(int row, int column);
    static QString urlOfJumpToBlockBegin(int rowS, int columnS, int rowE, int columnE);
    static QString urlOfJumpToBlockEnd(int rowS, int columnS, int rowE, int columnE);

    static QString urlOfToPrevLabel();
    static QString urlOfToNextLabel();
    static QString urlOfToSpecificLabel(int index);

    static QString urlOfOptimizeClone(int block_beg_row, int block_beg_col);

    static QString urlOfMaskWarning(int w_code);

    static QString urlOfJumpFile(QString file_path);

public:
    static UrlInterpreterEventproxy eventProxy;
};

//////////    /////////  //    ///  /////////  //////     /////////  ///////
//////   //     ///      //   ///     ///      ///   //   ///        ///    //
/////   //     ///       //  ///     ///      ///   //   /////////  ///////
////   //     ///         /////     ///      ///   //   ///        /////
///////   /////////       ///   /////////  ///////    /////////  ///  ////

class UrlInterpreterEventproxy : public QObject {
    Q_OBJECT
signals:
    void jumpToLines(QObject *const sourceParent, int rowS, int col = 0, int lineCount = 1);
    void jumpToLabel(QObject *const source, QObject *const sourceParent, bool next, int specific = 0);
    void optimizeClone(QObject *const source, QObject *const sourceParent, int block_beg_row, int block_beg_col);
    void maskWarning(QObject *const sourceParent, int w_code);
    void jumpToFile(QString file_path);
};

#endif // URLINTERPRETER_H
