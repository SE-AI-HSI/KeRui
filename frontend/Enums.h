#ifndef ENUMS_H
#define ENUMS_H

enum LabelType { Warning = 0, Clone = 1, Error = 2 };

enum AnalyzeProgress {
    /**
     * 阶段: Initializing
     * 做什么:
     * ①注册AnalyzeResultHandler监听函数[MainWidget|LabelManager]
     * ②重置分数栏(显示问号)[MainWidget]
     */
    Initializing,

    /**
     * 阶段: ProcessingLabels
     * 做什么:
     * ①弹出分析窗口，阻止交互动作
     */
    ProcessingLabels,

    /**
     * 阶段: Finished
     * 做什么:
     */
    Finished
};

#endif // ENUMS_H
