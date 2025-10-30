#ifndef ANALYSISLABEL_H
#define ANALYSISLABEL_H

#include "Enums.h"
#include "HighlighterLabel.h"
#include "InfoStruct.h"
#include <QMouseEvent>
#include <QSpinBox>
#include <QTextBrowser>

class LabelData;
class AnalysisLabelEventProxy;

class AnalysisLabel : public QTextBrowser {
    Q_OBJECT
public:
    AnalysisLabel(QWidget *parent, QWidget *linePainter, LabelData *data);
    ~AnalysisLabel();
    void setupBottomBar(int page, int max_page);

private:
    // init
    void setupStyle(LabelType &t);
    void setText();

    // events
    void connectEvents();
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *e);

private:
    // 负责绘制连线的是LabelManager这个窗口//
    // 标签移动的时候需要调用linePainter的update()更新界面//
    QWidget *const linePainter;

    bool isDragging = false;
    QPoint mousePressPos;

    static int lineHeight; // 标签中一行文本的高度//

    LabelHighlighter *highlighter;

public:
    LabelData *const data;
    static AnalysisLabelEventProxy eventProxy;
    QSpinBox *spinBox;
};

//////////    /////////  //    ///  /////////  //////     /////////  ///////
//////   //     ///      //   ///     ///      ///   //   ///        ///    //
/////   //     ///       //  ///     ///      ///   //   /////////  ///////
////   //     ///         /////     ///      ///   //   ///        /////
///////   /////////       ///   /////////  ///////    /////////  ///  ////

/**
 * @class LabelData
 * @brief AnalyzeResultCatcher收到结果之后将其格式化为LabelData(然后emit出去),
 * 而不是直接转为AnalysisLabel,因为它只负责处理数据,
 * 而AnalysisLabel的显示由LabelManager管理
 */

class LabelData {
public:
    LabelData(LabelType labelType, int analysisType, int _row, int _column, const QString &_text,
              CloneSetInfo *cns = nullptr, int w_code = 0)
        : cloneNodeSet(cns) {
        ltype = labelType, atype = analysisType;
        row = _row, column = _column;
        text = new QString(_text); // AnalysisLabel用完text之后会负责delete//
        warningCode = w_code;
    }
    LabelType ltype;
    int atype = 0; // 规范性,执行效率等//
    int row = 0;
    int column = 0;
    QString *text;
    CloneSetInfo *const cloneNodeSet;
    int warningCode;
};

//////////    /////////  //    ///  /////////  //////     /////////  ///////
//////   //     ///      //   ///     ///      ///   //   ///        ///    //
/////   //     ///       //  ///     ///      ///   //   /////////  ///////
////   //     ///         /////     ///      ///   //   ///        /////
///////   /////////       ///   /////////  ///////    /////////  ///  ////

class AnalysisLabelEventProxy : public QObject {
    Q_OBJECT
signals:
    void jumpToLine(QObject *const parent, int row);
};

#endif // ANALYSISLABEL_H
