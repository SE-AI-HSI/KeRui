#ifndef LABELMANAGER_H
#define LABELMANAGER_H

#include "AnalyzeResultHandler.h"
#include "Enums.h"
#include <QJsonObject>
#include <QJsonParseError>
#include <QPen>
#include <QTimer>
#include <QWidget>
#include <unordered_map>
#include <vector>

/**
 * @class LabelManager
 * @brief 在CodeEditor中负责绘制连线; 创建和管理显示哪些AnalysisLabel
 * 在TextEdit之上
 */

class LabelManager : public QWidget {
    Q_OBJECT
public:
    explicit LabelManager(QWidget *parent, QWidget *labelParent);
    ~LabelManager();
    void initLabels();
    void updateLabels();
    void highlight(int row, int lineCounter);
    AnalysisLabel *const jumpLabel(QObject *const source, bool next, int specific = 0) const;

    static int flags;
    static bool errorFlag;

    /// 分析信息的处理
    void warningInfo(const QList<IWarningInfo> &warningInfo);
    void errorInfo(const QList<IErrorInfo> &errorInfo);
    void cloneInfo(const QVector<QPair<int, QList<QList<ClonePosition>>>> &cloneSets, QString filePath,
                   QString fileRelativelyPath);
    void sortAndShowLabels();
    void clearLabels(); // 清空重置标签

    static void initInfo();

signals:
    void getFilePath();

private:
    void setupStyle();
    // events
    void connectEvents();
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event);

private:
    QWidget *const labelParent; // 实际上是指向TextEdit的指针//
    std::vector<AnalysisLabel *> labelList;
    // bool labelInitialized = false;//指标签的高度和位置的初始化//

    QPen pen;
    QBrush brush;
    QTimer *timer;
    int highlightLine = 0; // 点击跳转到某行时的高亮//
    int highlightAlpha = 0;
    int highlightLineCounter = 1;

    static int viewportWidth; // 可见窗口的宽度//
    static int viewportHeight;

    int labelBlockSize = 10; // 区块大小,显示标签划分的区块大小

    static QJsonObject rootJson;
};

#endif // LABELMANAGER_H
