#ifndef LINENUMBERLIST_H
#define LINENUMBERLIST_H

#include <QLabel>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>

/**
 * @class LineNumberList
 * @brief 代码的行号列表,实现添加/删除行,以及点击事件等
 */

class LineNumberList : public QLabel {
    Q_OBJECT
public:
    explicit LineNumberList(QWidget *parent, int _lineHeight);
    ~LineNumberList();
    void setLineCount(int n);

private:
    // init
    void setupStyle();
    // events
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

signals:
    void selectLinesSignal(int lowerBound, int upperBound);
    void wheel(QWheelEvent *event);

private:
    int curLines = 0;
    static int lineHeight;
    std::pair<int, int> selectedLines; // 选中的行<lowerBound,upperBound>
    bool isDragging = false;
    int clickedLine = 0; // 鼠标按下时所在的行号//
};

#endif // LINENUMBERLIST_H
