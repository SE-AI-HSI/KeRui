#include "AnalysisLabel.h"
#include "QRegExp"
#include "TextEdit.h"
#include "UrlInterpreter.h"
#include <QDebug>
#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollArea>

#define FIXED_WIDTH 300
#define DOC_PADDING 10
#define EXTRA_HEIGHT 20 // 等于DOC_PADDING * 2//

int AnalysisLabel::lineHeight;
AnalysisLabelEventProxy AnalysisLabel::eventProxy;

AnalysisLabel::AnalysisLabel(QWidget *parent, QWidget *painter, LabelData *_data)
    // parent实际传过来的是PlainTextEdit,即AnalysisLabel是显示在PlainTextEdit上层的//
    // 而不是显示在LabelManager(参数painter)上,因为LabelManager位于PlainTextEdit下层//
    // LabelManager负责绘制连线和管理显示哪些AnalysisLabel应该显示在PlainTextEdit//
    : QTextBrowser(parent), linePainter(painter), data(_data) {
    lineHeight = this->fontMetrics().lineSpacing();
    /** 标签文本高亮 **/
    highlighter = new LabelHighlighter(document());

    setupStyle(data->ltype);

    setText();
    connectEvents();
}

AnalysisLabel::~AnalysisLabel() {
    delete data;
}

void AnalysisLabel::setupBottomBar(int page, int max_page) {
    QWidget *Container = new QWidget(this);
    Container->setAttribute(Qt::WA_StyledBackground);
    Container->setFixedHeight(26);
    Container->setStyleSheet("border-image: url(:/images/AnalysisLabel/bg.svg);");

    QHBoxLayout *layout = new QHBoxLayout(Container);
    layout->setContentsMargins(10, 0, 10, 0);
    layout->addStretch();

    QPushButton *first = new QPushButton();
    QPushButton *prev = new QPushButton();
    QPushButton *next = new QPushButton();
    QPushButton *last = new QPushButton();
    first->setFixedSize(20, 20);
    prev->setFixedSize(20, 20);
    next->setFixedSize(20, 20);
    last->setFixedSize(20, 20);
    first->setCursor(Qt::PointingHandCursor);
    prev->setCursor(Qt::PointingHandCursor);
    next->setCursor(Qt::PointingHandCursor);
    last->setCursor(Qt::PointingHandCursor);

    first->setStyleSheet("QPushButton{"
                         "border-image: url(:/images/AnalysisLabel/first.svg);"
                         "}"
                         "QPushButton:hover{"
                         "border-image: url(:/images/AnalysisLabel/first-hover.svg);"
                         "}");
    prev->setStyleSheet("QPushButton{"
                        "border-image: url(:/images/AnalysisLabel/prev.svg);"
                        "}"
                        "QPushButton:hover{"
                        "border-image: url(:/images/AnalysisLabel/prev-hover.svg);"
                        "}");
    next->setStyleSheet("QPushButton{"
                        "border-image: url(:/images/AnalysisLabel/next.svg);"
                        "}"
                        "QPushButton:hover{"
                        "border-image: url(:/images/AnalysisLabel/next-hover.svg);"
                        "}");
    last->setStyleSheet("QPushButton{"
                        "border-image: url(:/images/AnalysisLabel/last.svg);"
                        "}"
                        "QPushButton:hover{"
                        "border-image: url(:/images/AnalysisLabel/last-hover.svg);"
                        "}");

    spinBox = new QSpinBox;
    spinBox->setFixedHeight(20);
    spinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    spinBox->setAlignment(Qt::AlignCenter);
    spinBox->setMinimum(1);
    spinBox->setMaximum(max_page);
    spinBox->setValue(page);
    spinBox->setSuffix("/" + QString::number(max_page));

    connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int val) {
        emit UrlInterpreter::interpret(this, parent(), QUrl(UrlInterpreter::urlOfToSpecificLabel(val)));
    });

    connect(first, &QPushButton::clicked, this, [=]() {
        UrlInterpreter::interpret(this, parent(), QUrl(UrlInterpreter::urlOfToSpecificLabel(1)));
    });
    connect(prev, &QPushButton::clicked, this, [=]() {
        UrlInterpreter::interpret(this, parent(), QUrl(UrlInterpreter::urlOfToPrevLabel()));
    });
    connect(next, &QPushButton::clicked, this, [=]() {
        UrlInterpreter::interpret(this, parent(), QUrl(UrlInterpreter::urlOfToNextLabel()));
    });
    connect(last, &QPushButton::clicked, this, [=]() {
        UrlInterpreter::interpret(this, parent(), QUrl(UrlInterpreter::urlOfToSpecificLabel(spinBox->maximum())));
    });

    layout->addWidget(first);
    layout->addWidget(prev);
    layout->addWidget(spinBox);
    layout->addWidget(next);
    layout->addWidget(last);
    layout->addStretch();

    Container->show();
    Container->move((width() - Container->width()) / 2, height() - Container->height() - 10);
}

void AnalysisLabel::setupStyle(LabelType &t) {
    setFixedWidth(FIXED_WIDTH);
    document()->setDocumentMargin(DOC_PADDING);

    setObjectName("Label");
    QString qss = "#Label{"
                  "color: rgb(66,66,66);"
                  "border-radius: 20px;"
                  "font: 13px;";
    if (t == LabelType::Warning) {
        qss += "background-color: #ffefb5";
    } else if (t == LabelType::Clone) {
        qss += "background-color: #b8eeff";
    } else if (t == LabelType::Error) {
        qss += "background-color: #ffcac8";
    }
    qss += "}";
    setStyleSheet(qss);
}

void AnalysisLabel::setText() {
    append(*(data->text));
    delete data->text; // 用完就可以删掉了//
    data->text = NULL;

    // 根据文本<预>调整大小,后面还是需要LabelManager对其高度进行调整//
    setFixedHeight(this->lineHeight * document()->lineCount() + EXTRA_HEIGHT);
}

void AnalysisLabel::connectEvents() {
    /** 实现点击超链接,没有一个字母是多余的:) **/
    setFocusPolicy(Qt::NoFocus);
    setOpenLinks(false);
    setOpenExternalLinks(false);

    connect(this, &QTextBrowser::anchorClicked, this, [&](const QUrl &url) {
        // 让UrlInterpreter负责解释//
        UrlInterpreter::interpret(this, parent(), url);
    });
}

void AnalysisLabel::mousePressEvent(QMouseEvent *event) {
    QTextBrowser::mousePressEvent(event);
    if (event->button() == Qt::LeftButton) {
        raise(); // 点击的时候升到顶层//
        mousePressPos = event->pos();
        isDragging = true;
    }
}

void AnalysisLabel::mouseMoveEvent(QMouseEvent *event) {
    if (isDragging == true) {
        int x = this->x() + event->pos().x() - mousePressPos.x();
        int y = this->y() + event->pos().y() - mousePressPos.y();
        QRect label(x - 5, y - 5, width() + 5, height() + 5);
        /** 限制移动 **/
        if (((QWidget *)parent())->geometry().contains(label)) {
            move(x, y);
            linePainter->update(); // 重新绘制连线, 效率(也许)比调用repaint高//
        }
    } else {
        // 不调用没法点击超链接:)//
        QTextBrowser::mouseMoveEvent(event);
    }
}
void AnalysisLabel::mouseReleaseEvent(QMouseEvent *event) {
    isDragging = false;
    QTextBrowser::mouseReleaseEvent(event);
}

void AnalysisLabel::keyPressEvent(QKeyEvent *e) {
    if (e->key() == 16777220 || 16777221) {
        e->accept();
    }
}
