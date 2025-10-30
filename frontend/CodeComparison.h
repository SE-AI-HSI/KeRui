#ifndef CODECOMPARISON_H
#define CODECOMPARISON_H

#include "CCTextEdit.h"
#include <QBrush>
#include <QColor>
#include <QFrame>
#include <QPainter>
#include <QWidget>
#include <vector>

enum ComparisonType;
struct Comparison;
class CCHighlighter;

/**
 * @class CodeComparison
 * @brief 优化前后代码对比窗口
 */

namespace Ui {
class CodeComparison;
}

class CodeComparison : public QWidget {
    Q_OBJECT

public:
    explicit CodeComparison();
    ~CodeComparison();

private slots:
    void on_Close_clicked();

    void on_Minimize_clicked();

    void on_Maximize_clicked();

private:
    /// inits
    void setText();
    void setupLayout();
    void setButtons();
    /// ui
    QWidget *LineNumberList(int num_lines);
    /// events
    void connectEvents();
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    Ui::CodeComparison *ui;
    bool isDragging = false;
    QPoint mousePressPos;

    CCTextEdit *LTextEdit;
    CCTextEdit *RTextEdit;
    CCHighlighter *Highlighter;
    QWidget *lineNumberList;

    std::vector<Comparison> comps;
};

enum ComparisonType { Insert, Modify, Delete };

struct Comparison {
    Comparison(ComparisonType t, int lrb, int lre, int rrb, int rre) {
        type = t;
        lRowBeg = lrb, lRowEnd = lre, rRowBeg = rrb, rRowEnd = rre;
        isApplied = false;
        oldText = "";
    }
    ComparisonType type;
    int lRowBeg;
    int lRowEnd;
    int rRowBeg;
    int rRowEnd;
    bool isApplied;
    QString oldText;
};

//////////    /////////  //    ///  /////////  //////     /////////  ///////
//////   //     ///      //   ///     ///      ///   //   ///        ///    //
/////   //     ///       //  ///     ///      ///   //   /////////  ///////
////   //     ///         /////     ///      ///   //   ///        /////
///////   /////////       ///   /////////  ///////    /////////  ///  ////

/**
 * @class CCHighlighter
 * @brief 高亮代码对比窗口中的代码
 */

class CCHighlighter : public QFrame {
public:
    CCHighlighter(QWidget *parent, std::vector<Comparison> &comps, int width);

private:
    void paintEvent(QPaintEvent *e) override;

private:
    QBrush brush;
    QPen pen;
    std::vector<Comparison> &comps;
    const int TEXT_EDIT_WIDTH;
};

#endif // CODECOMPARISON_H
