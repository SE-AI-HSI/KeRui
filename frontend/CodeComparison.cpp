#include "CodeComparison.h"
#include "AnalyzerClient.h"
#include "FileManager.h"
#include "GraphicsEffectUtil.h"
#include "ui_CodeComparison.h"
#include <QDebug>
#include <QGuiApplication>
#include <QLabel>
#include <QPushButton>
#include <QScreen>
#include <QScrollBar>
#include <QTextBlock>
#include <QVBoxLayout>

#define LINE_NUMBER_WIDTH 120

CodeComparison::CodeComparison() : QWidget(nullptr), ui(new Ui::CodeComparison) {
    ui->setupUi(this);
    setWindowModality(Qt::ApplicationModal);
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    ui->widget->setGraphicsEffect(GraphicsEffectUtil::newShadow(this, 64));
    ui->widget_4->setGraphicsEffect(GraphicsEffectUtil::newShadow(this, 64));

    QRect window_rect = AnalyzerClient::windowRect;
    setFixedSize(window_rect.width() * 0.85, window_rect.height() * 0.95);
    show();
    move(window_rect.x() + (window_rect.width() - width()) / 2,
         window_rect.y() + (window_rect.height() - height()) / 2);

    setupLayout();
    connectEvents();

    ui->Minimize->hide();
    ui->Maximize->hide();
}

CodeComparison::~CodeComparison() {
    delete ui;
}

void CodeComparison::setText() {
    Comparison a(Modify, 8, 8, 8, 8);
    Comparison b(Modify, 25, 25, 25, 25);
    Comparison c(Modify, 45, 45, 45, 45);
    Comparison d(Delete, 129, 133, 128, 128);
    Comparison e(Insert, 180, 180, 176, 179);
    Comparison g(Modify, 321, 321, 320, 320);
    Comparison h(Delete, 338, 338, 336, 336);
    Comparison i(Insert, 340, 340, 339, 342);
    comps.emplace_back(a);
    comps.emplace_back(b);
    comps.emplace_back(c);
    comps.emplace_back(d);
    comps.emplace_back(e);
    comps.emplace_back(g);
    comps.emplace_back(h);
    comps.emplace_back(i);

    QString ls = FileManager::read("d:/l.cpp").second;
    QString rs = FileManager::read("d:/r.cpp").second;

    LTextEdit->setText(ls);
    RTextEdit->setText(rs);

    /// 文本对齐处理
    int LExtraLines = 0, RExtraLines = 0;
    for (Comparison &c : comps) {
        int delta = c.lRowEnd - c.lRowBeg - (c.rRowEnd - c.rRowBeg);

        c.lRowBeg += LExtraLines, c.lRowEnd += LExtraLines;
        c.rRowBeg += RExtraLines, c.rRowEnd += RExtraLines;

        if (c.lRowBeg == c.lRowEnd && c.type == Insert) {
            LTextEdit->insertNewLines(c.lRowEnd);
            c.lRowEnd += 1;
            c.lRowBeg = c.lRowEnd;
            LExtraLines += 1;
        } else if (c.rRowBeg == c.rRowEnd && c.type == Delete) {
            RTextEdit->insertNewLines(c.rRowEnd);
            c.rRowEnd += 1;
            c.rRowBeg = c.rRowEnd;
            RExtraLines += 1;
        }

        if (delta > 0) {
            RTextEdit->insertNewLines(c.rRowEnd, delta);
            c.rRowEnd += delta;
            RExtraLines += delta;
        } else if (delta < 0) {
            LTextEdit->insertNewLines(c.lRowEnd, -delta);
            c.lRowEnd -= delta;
            LExtraLines -= delta;
        }
    }
}

void CodeComparison::setupLayout() {
    int TEWidth = (ui->Container->width() - LINE_NUMBER_WIDTH) / 2;
    /// 代码文本框
    LTextEdit = new CCTextEdit(ui->Container);
    RTextEdit = new CCTextEdit(ui->Container);

    ///!!!先setText
    setText();

    LTextEdit->setFixedWidth(TEWidth);
    RTextEdit->setFixedWidth(TEWidth);

    LTextEdit->move(0, 0);
    RTextEdit->move(TEWidth + LINE_NUMBER_WIDTH, 0);

    LTextEdit->setFixedHeight(ui->Container->height());
    RTextEdit->setFixedHeight(ui->Container->height());

    LTextEdit->show();
    RTextEdit->show();

    LTextEdit->verticalScrollBar()->setValue(0);
    RTextEdit->verticalScrollBar()->setValue(0);

    /// 代码高亮器
    Highlighter = new CCHighlighter(ui->Container, comps, TEWidth);

    Highlighter->move(LTextEdit->pos());

    Highlighter->show();

    LTextEdit->raise();
    RTextEdit->raise();

    /// 代码行号
    lineNumberList = LineNumberList(LTextEdit->lineCount());

    lineNumberList->move(TEWidth, 0);

    lineNumberList->show();

    ui->Container->setFixedHeight(lineNumberList->height());
    Highlighter->setFixedSize(ui->Container->size());

    setButtons();
}

void CodeComparison::setButtons() {
    for (int i = 0; i < comps.size(); ++i) {
        QPushButton *btn = new QPushButton(ui->Container);
        btn->setObjectName(QString::number(i));
        btn->show();
        btn->setFixedSize(36, 36);
        btn->move(LTextEdit->width(),
                  5 - 18 + (comps[i].lRowEnd + comps[i].lRowBeg - 1) * 0.5 * CCTextEdit::lineHeight());
        btn->setCursor(Qt::PointingHandCursor);

        Comparison &c = comps[btn->objectName().toInt()];
        if (c.type == Modify) {
            btn->setStyleSheet("border-image: url(:/images/CodeComparison/cc_modify.svg);");
        } else if (c.type == Delete) {
            btn->setStyleSheet("border-image: url(:/images/CodeComparison/cc_delete.svg);");
        } else {
            btn->setStyleSheet("border-image: url(:/images/CodeComparison/cc_insert.svg);");
        }
        connect(btn, &QPushButton::pressed, this, [=] {
            Comparison &c = comps[btn->objectName().toInt()];
            if (!c.isApplied) {
                c.isApplied = true;
                c.oldText = LTextEdit->getText(c.lRowBeg, c.lRowEnd);
                LTextEdit->replaceText(RTextEdit->getText(c.rRowBeg, c.rRowEnd), c.lRowBeg, c.lRowEnd);
                btn->setStyleSheet("border-image: url(:/images/CodeComparison/cc_redo.svg);");
            } else {
                c.isApplied = false;
                LTextEdit->replaceText(c.oldText, c.lRowBeg, c.lRowEnd);
                c.oldText.clear();
                if (c.type == Modify) {
                    btn->setStyleSheet("border-image: url(:/images/CodeComparison/cc_modify.svg);");
                } else if (c.type == Delete) {
                    btn->setStyleSheet("border-image: url(:/images/CodeComparison/cc_delete.svg);");
                } else {
                    btn->setStyleSheet("border-image: url(:/images/CodeComparison/cc_insert.svg);");
                }
            }
            Highlighter->update();
            LTextEdit->verticalScrollBar()->setValue(ui->MainScrollArea->verticalScrollBar()->value());
            RTextEdit->verticalScrollBar()->setValue(ui->MainScrollArea->verticalScrollBar()->value());
        });
    }
}

QWidget *CodeComparison::LineNumberList(int num_lines) {
    QLabel *List = new QLabel(ui->Container);
    List->setFixedWidth(LINE_NUMBER_WIDTH);
    List->setFont(QFont("Source Code Pro, Microsoft Yahei", 15));
    List->setStyleSheet("padding-top: 4px;");

    List->setAlignment(Qt::AlignCenter);

    QString s = "";
    for (int i = 1; i <= num_lines; ++i) {
        s += QString::number(i);
        if (i != num_lines)
            s += "\n";
    }
    List->setText(s);
    List->adjustSize();

    return List;
}

void CodeComparison::connectEvents() {
    connect(ui->MainScrollArea->verticalScrollBar(), &QScrollBar::valueChanged, this, [=](int val) {
        /// 保持文本框位置不变
        LTextEdit->move(0, val);
        RTextEdit->move(RTextEdit->x(), val);
        /// 滚动其内容
        LTextEdit->verticalScrollBar()->setValue(val);
        RTextEdit->verticalScrollBar()->setValue(val);
    });

    /// 水平联动滚动
    connect(LTextEdit->horizontalScrollBar(), &QScrollBar::valueChanged, this, [=](int val) {
        RTextEdit->horizontalScrollBar()->setValue(val);
    });
    connect(RTextEdit->horizontalScrollBar(), &QScrollBar::valueChanged, this, [=](int val) {
        LTextEdit->horizontalScrollBar()->setValue(val);
    });

    connect(ui->Cancel, &QPushButton::pressed, this, [=] {
        delete this;
    });
    connect(ui->Finish, &QPushButton::pressed, this, [=] {
        for (int i = comps.size() - 1; i >= 0; --i) {
            for (int line = comps[i].lRowEnd; line >= comps[i].lRowBeg; --line) {
                QTextBlock b = LTextEdit->document()->findBlockByNumber(line - 1);
                if (b.text().isEmpty()) {
                    LTextEdit->deleteLine(line);
                } else
                    break;
            }
        }
        FileManager::write("d:/res.cpp", UTF8, LTextEdit->toPlainText());
        delete this;
    });
}

void CodeComparison::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && event->pos().y() < ui->TopBar->height()) {
        isDragging = true;
        mousePressPos = event->globalPos() - this->pos();
        event->accept();
    } else {
        QWidget::mousePressEvent(event);
    }
}

void CodeComparison::mouseMoveEvent(QMouseEvent *event) {
    if (isDragging) {
        move(event->globalPos() - mousePressPos);
        event->accept();
    } else {
        QWidget::mouseMoveEvent(event);
    }
}

void CodeComparison::mouseReleaseEvent(QMouseEvent *event) {
    isDragging = false;
    QWidget::mouseReleaseEvent(event);
}

//////////    /////////  //    ///  /////////  //////     /////////  ///////
//////   //     ///      //   ///     ///      ///   //   ///        ///    //
/////   //     ///       //  ///     ///      ///   //   /////////  ///////
////   //     ///         /////     ///      ///   //   ///        /////
///////   /////////       ///   /////////  ///////    /////////  ///  ////

CCHighlighter::CCHighlighter(QWidget *parent, std::vector<Comparison> &comps_, int width)
    : QFrame(parent), comps(comps_), TEXT_EDIT_WIDTH(width) {
    brush.setStyle(Qt::SolidPattern);
    pen.setStyle(Qt::DashLine);
    pen.setWidth(2);
}

void CCHighlighter::paintEvent(QPaintEvent *e) {
    QFrame::paintEvent(e);
    QPainter painter(this);

    brush.setColor(QColor(255, 172, 122));
    painter.setPen(Qt::NoPen);
    painter.setBrush(brush);
    painter.drawRect(TEXT_EDIT_WIDTH, 0, LINE_NUMBER_WIDTH, height());

    for (Comparison &c : comps) {
        int y1 = (c.rRowBeg - 1) * CCTextEdit::lineHeight() + 5;
        int y2 = (c.rRowEnd) * CCTextEdit::lineHeight() + 5;

        /// Modify
        if (c.type == Modify) {
            brush.setColor(QColor(184, 238, 255));
            pen.setColor(QColor(184, 238, 255));
            if (!c.isApplied) {
                painter.setPen(Qt::NoPen);
                painter.setBrush(brush);
                painter.drawRect(0, y1, width(), y2 - y1);
            } else {
                painter.setBrush(Qt::NoBrush);
                painter.setPen(pen);
                painter.drawLine(0, y1 + 1, width(), y1 + 1);
                painter.drawLine(0, y2 - 1, width(), y2 - 1);
            }
        } /// Delete
        else if (c.type == Delete) {
            brush.setColor(QColor(255, 202, 200));
            pen.setColor(QColor(255, 202, 200));

            painter.setBrush(Qt::NoBrush);
            painter.setPen(pen);
            painter.drawLine(TEXT_EDIT_WIDTH, y1 + 1, width(), y1 + 1);
            painter.drawLine(TEXT_EDIT_WIDTH, y2 - 1, width(), y2 - 1);

            if (!c.isApplied) {
                painter.setPen(Qt::NoPen);
                painter.setBrush(brush);
                painter.drawRect(0, y1, TEXT_EDIT_WIDTH, y2 - y1);
            } else {
                painter.drawLine(0, y1 + 1, TEXT_EDIT_WIDTH, y1 + 1);
                painter.drawLine(0, y2 - 1, TEXT_EDIT_WIDTH, y2 - 1);
            }
        } /// Insert
        else {
            static const int pos = TEXT_EDIT_WIDTH + LINE_NUMBER_WIDTH;
            brush.setColor(QColor(177, 242, 161));
            pen.setColor(QColor(177, 242, 161));

            painter.setBrush(Qt::NoBrush);
            painter.setPen(pen);
            painter.drawLine(0, y1 + 1, pos, y1 + 1);
            painter.drawLine(0, y2 - 1, pos, y2 - 1);

            if (!c.isApplied) {
                painter.setPen(Qt::NoPen);
                painter.setBrush(brush);
                painter.drawRect(pos, y1, TEXT_EDIT_WIDTH + LINE_NUMBER_WIDTH / 2, y2 - y1);
            } else {
                painter.drawLine(pos, y1 + 1, width(), y1 + 1);
                painter.drawLine(pos, y2 - 1, width(), y2 - 1);
            }
        }
    }
}

void CodeComparison::on_Close_clicked() {
    delete this;
}

void CodeComparison::on_Minimize_clicked() {
    showMinimized();
}

void CodeComparison::on_Maximize_clicked() {
    if (this->geometry().width() < QGuiApplication::screens().at(0)->geometry().width()) {
        showMinimized();
        showMaximized();
    } else {
        showMaximized();
        showNormal();
    }
}
