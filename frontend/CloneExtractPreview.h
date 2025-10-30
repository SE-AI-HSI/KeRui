#ifndef CLONEEXTRACTPREVIEW_H
#define CLONEEXTRACTPREVIEW_H

#include "CodeBox.h"
#include "InfoStruct.h"
#include <QMouseEvent>
#include <QWidget>
#include <vector>

/**
 * @class CloneExtractPreview
 * @brief 重复块提取预览窗口
 */

namespace Ui {
class CloneExtractPreview;
}

class CloneExtractPreview : public QWidget {
    Q_OBJECT
    struct Operation;

public:
    explicit CloneExtractPreview(std::vector<CodeGenerateInfo> &cg_list, QString file_path, CodeBox *codeBox);
    ~CloneExtractPreview();

private slots:
    void on_Close_clicked();

    void on_Cancel_clicked();

    void on_Finish_clicked();

private:
    /// init
    void setupLayout();
    void setLineCount(int n);
    /// events
    void connectEvents();
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    /// others
    void readAndScroll(const QString &file_path, int row, int col);
    void initTreeWidget();
    void setTextEditLineSpacing();

private:
    Ui::CloneExtractPreview *ui;
    std::vector<CodeGenerateInfo> cgList;
    QString filePath;
    CodeBox *codeBox;

    bool isDragging = false;
    QPoint mousePressPos;
};

#endif // CLONEEXTRACTPREVIEW_H
