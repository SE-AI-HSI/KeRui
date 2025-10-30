#include "CloneExtractPreview.h"
#include "AnalyzerClient.h"
#include "FileManager.h"
#include "GraphicsEffectUtil.h"
#include "ui_CloneExtractPreview.h"
#include <QDebug>
#include <QFont>
#include <QScrollBar>
#include <QTextBlock>
#include <QTimer>
#include <algorithm>
#include <map>

inline bool compare_func(const CodeGenerateInfo &a, const CodeGenerateInfo &b) {
    if (a.m_beginLine == b.m_beginLine)
        return a.m_beginColum < b.m_beginColum;
    return a.m_beginLine < b.m_beginLine;
}

CloneExtractPreview::CloneExtractPreview(std::vector<CodeGenerateInfo> &cg_list, QString file_path, CodeBox *codeBox)
    : QWidget(nullptr), ui(new Ui::CloneExtractPreview), cgList(cg_list), filePath(file_path), codeBox(codeBox) {
    ui->setupUi(this);
    setupLayout();
    setWindowTitle(QString::fromLocal8Bit("代码优化预览"));

    QRect window_rect = AnalyzerClient::windowRect;
    setFixedSize(window_rect.width() * 0.85, window_rect.height() * 0.95);
    show();
    move(window_rect.x() + (window_rect.width() - width()) / 2,
         window_rect.y() + (window_rect.height() - height()) / 2);

    connectEvents();

    initTreeWidget();
}

CloneExtractPreview::~CloneExtractPreview() {
    delete ui;
}

void CloneExtractPreview::on_Close_clicked() {
    delete this;
}

void CloneExtractPreview::setupLayout() {
    setWindowModality(Qt::ApplicationModal);
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    // 阴影
    ui->widget->setGraphicsEffect(GraphicsEffectUtil::newShadow(this, 64));
    ui->treeWidget->setGraphicsEffect(GraphicsEffectUtil::newShadow(this, 64));
    ui->frame->setGraphicsEffect(GraphicsEffectUtil::newShadow(this, 64));

    ui->Maximize->hide();
    ui->Minimize->hide();
    // 表头
    QStringList list;
    list << QString::fromLocal8Bit("文件名") << QString::fromLocal8Bit("更改类型") << QString::fromLocal8Bit("行号")
         << QString::fromLocal8Bit("列号");
    ui->treeWidget->setHeaderLabels(list);
    // 表头自适应宽度
    ui->treeWidget->header()->setDefaultAlignment(Qt::AlignLeft);
    ui->treeWidget->header()->setMinimumSectionSize(170);
    ui->treeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // TextEdit
    ui->textEdit->setLineWrapMode(QTextEdit::NoWrap); // 关闭自动文字折行//
    ui->textEdit->setFont(QFont("Source Code Pro, Microsoft Yahei", 15));
    setTextEditLineSpacing();
    ui->textEdit->setTabStopWidth(4 * ui->textEdit->fontMetrics().horizontalAdvance(' '));
}

void CloneExtractPreview::setLineCount(int n) {
    ui->label->setAlignment(Qt::AlignRight);
    ui->label->setFont(QFont("Source Code Pro, Microsoft Yahei", 15));

    QString s = "";
    for (int i = 1; i <= n; ++i) {
        s += QString::number(i);
        if (i != n)
            s += "\n";
    }

    ui->label->setFixedHeight(
        (std::max)(ui->textEdit->height(), ui->textEdit->fontMetrics().lineSpacing() * (n + 2) + 4));
    ui->label->setText(s);
}

void CloneExtractPreview::connectEvents() {
    connect(ui->textEdit->verticalScrollBar(), &QScrollBar::valueChanged, this, [=](int val) {
        ui->label->move(0, -val);
    });

    connect(ui->treeWidget, &QTreeWidget::itemClicked, this, [=](QTreeWidgetItem *item, int column) {
        if (item->data(1, 0).isValid()) {
            QString file_path = item->parent()->data(0, 0).toString();
            int row = item->data(2, 0).toInt();
            int col = item->data(3, 0).toInt();
            readAndScroll(file_path, row, col);
        }
    });
}

void CloneExtractPreview::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && event->pos().y() < ui->TopBar->height()) {
        isDragging = true;
        mousePressPos = event->globalPos() - this->pos();
        event->accept();
    } else {
        QWidget::mousePressEvent(event);
    }
}

void CloneExtractPreview::mouseMoveEvent(QMouseEvent *event) {
    if (isDragging) {
        move(event->globalPos() - mousePressPos);
        event->accept();
    } else {
        QWidget::mouseMoveEvent(event);
    }
}

void CloneExtractPreview::mouseReleaseEvent(QMouseEvent *event) {
    isDragging = false;
    QWidget::mouseReleaseEvent(event);
}

void CloneExtractPreview::readAndScroll(const QString &file_path, int row, int col) {
    QString text = FileManager::read(file_path, true);
    ui->textEdit->clear();
    setTextEditLineSpacing();
    ui->textEdit->insertPlainText(text);

    QTextCursor c = ui->textEdit->textCursor();

    /// 经过initTreeWidget(),此时opList中的行号都是经过修正后的行号,即最终改好的代码的行号
    for (CodeGenerateInfo &cg : cgList) {
        if (filePath == file_path) {
            int begPos;
            QBrush brush;
            brush.setStyle(Qt::SolidPattern);
            //            qDebug() << QString::fromStdString(cg.m_modifiedCode)  << cg.m_beginColum << cg.m_endColum;
            if (cg.m_gcType == CG_Add) {
                QTextBlock b = ui->textEdit->document()->findBlockByNumber(cg.m_beginLine - 1);
                begPos = b.position();
                c.setPosition(begPos);
                ui->textEdit->setTextCursor(c);
                c.insertText(cg.m_modifiedCode);
                brush.setColor(QColor(177, 242, 161));
            } else if (cg.m_gcType == CG_Modify) {
                begPos = ui->textEdit->document()->findBlockByNumber(cg.m_beginLine - 1).position() + cg.m_beginColum;
                c.setPosition(begPos, QTextCursor::MoveAnchor);
                c.setPosition(ui->textEdit->document()->findBlockByNumber(cg.m_endLine - 1).position() + cg.m_endColum,
                              QTextCursor::KeepAnchor);
                ui->textEdit->setTextCursor(c);
                c.insertText(cg.m_modifiedCode);
                brush.setColor(QColor(184, 238, 255));
            } else {
            }

            c = ui->textEdit->textCursor();
            c.setPosition(begPos, QTextCursor::MoveAnchor);
            c.setPosition(begPos + cg.m_modifiedCode.length(), QTextCursor::KeepAnchor);
            ui->textEdit->setTextCursor(c);

            QTextCharFormat format;
            format.setBackground(brush);
            c.setCharFormat(format);
            ui->textEdit->setTextCursor(c);
        }
    }

    /// 先构造好行号列表
    setLineCount(ui->textEdit->document()->blockCount());
    /// 再滚动
    QTextBlock b = ui->textEdit->document()->findBlockByNumber(row - 1);
    c.setPosition(b.position() + col);
    ui->textEdit->setTextCursor(c);

    ui->frame->layout()->setEnabled(false);
    ui->textEdit->ensureCursorVisible(); // 否则第一次打开时行号不会更新位置
}

void CloneExtractPreview::initTreeWidget() {
    ui->treeWidget->clear();

    std::sort(cgList.begin(), cgList.end(), &compare_func);

    std::map<QString, QTreeWidgetItem *> Tree;
    std::map<QString, int> deltaLines;

    for (CodeGenerateInfo &cg : cgList) {
        if (deltaLines.find(filePath) == deltaLines.end()) {
            deltaLines[filePath] = 0;
        }

        /// 修正行号,因为插入和删除等会影响其他操作的行号位置
        cg.m_beginLine += deltaLines[filePath];
        cg.m_endLine += deltaLines[filePath];

        QString newCode = cg.m_modifiedCode;
        int newCodeLines = 1;
        for (const QChar &c : newCode) {
            if (c == '\n')
                ++newCodeLines;
        }

        QString type;
        if (cg.m_gcType == CG_Add) {
            deltaLines[filePath] += newCodeLines - 1;
            type = QString::fromLocal8Bit("插入");
        } else if (cg.m_gcType == CG_Modify) {
            deltaLines[filePath] += newCodeLines - (cg.m_endLine - cg.m_beginLine) - 1;
            type = QString::fromLocal8Bit("修改");
        } else {
            type = QString::fromLocal8Bit("删除");
        }
        QTreeWidgetItem *child = new QTreeWidgetItem(QStringList() << "" << type << QString::number(cg.m_beginLine)
                                                                   << QString::number(cg.m_beginColum));

        if (Tree.find(filePath) == Tree.end()) {
            QTreeWidgetItem *parent = new QTreeWidgetItem(QStringList() << filePath);
            ui->treeWidget->addTopLevelItem(parent);

            parent->addChild(child);
            Tree[filePath] = parent;
        } else {
            Tree[filePath]->addChild(child);
        }
    }
}

void CloneExtractPreview::setTextEditLineSpacing() {
    QTextCursor c = ui->textEdit->textCursor();
    QTextBlockFormat blockFormat = c.blockFormat();
    blockFormat.setLineHeight(ui->textEdit->fontMetrics().lineSpacing(), QTextBlockFormat::FixedHeight);

    c.select(QTextCursor::Document);
    c.mergeBlockFormat(blockFormat);
    c.clearSelection();
    ui->textEdit->setTextCursor(c);
}

void CloneExtractPreview::on_Cancel_clicked() {
    delete this;
}

void CloneExtractPreview::on_Finish_clicked() {
    qDebug() << ui->textEdit->toPlainText();
    FileManager::write(filePath, FileManager::read(filePath).first, ui->textEdit->toPlainText());
    QCoreApplication::processEvents();
    codeBox->updateCode(filePath);
    delete this;
}
