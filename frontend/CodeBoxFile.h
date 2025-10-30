#ifndef CODEBOXFILE_H
#define CODEBOXFILE_H
#include "CodeEditor.h"
#include "FileManager.h"
#include "QLinkedList"
#include "SummaryWidget.h"
#include <QString>

class CodeBoxFile {
public:
    typedef QLinkedList<CodeBoxFile>::iterator Iterator;

    CodeBoxFile(const QString &_path, const QString _encoding, CodeEditor *const _editor,
                SummaryWidget *const summary_widget = nullptr)
        : path(_path), encoding(_encoding), editor(_editor), summaryWidget(summary_widget),
          relativelyPath(path.right(path.size() - FileManager::projectPath().size())) {
    }
    inline QString name() const {
        return path.mid(path.lastIndexOf("/") + 1);
    }
    inline void changePath(const QString &abs_path) {
        path = abs_path;
        if (path.contains(FileManager::projectPath())) {
            // 如果文件还在项目内
            relativelyPath = path.right(path.size() - FileManager::projectPath().size());
        } else {
            // 新建空白页,然后保存,这个文件可能没有保存在项目内
            relativelyPath = "";
        }
    }

    /** data members **/
    QString path;           // 文件相对路径//
    QString relativelyPath; // 文件绝对路径//
    const QString encoding; // 文件编码,目前只能识别UTF-8和GBK//
    CodeEditor *editor = nullptr;
    SummaryWidget *summaryWidget = nullptr;

    bool isSaved = true; // 刚打开的时候没有修改过,认为是已保存的//
};

#endif // CODEBOXFILE_H
