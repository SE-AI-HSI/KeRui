#ifndef SUMMARY_H
#define SUMMARY_H

#include <QHBoxLayout>
#include <QWidget>
#include <vector>

#define SUMMARY_FILE_NAME "AnalysisSummary.kr"

class SummaryRowData;
class SummaryWidgetEventProxy;

/**
 * @class SummaryWidget
 * @brief 总览窗口
 */

namespace Ui {
class SummaryWidget;
}

class SummaryWidget : public QWidget {
    Q_OBJECT

public:
    explicit SummaryWidget(std::vector<SummaryRowData> &_datas, QWidget *parent = nullptr);
    ~SummaryWidget();
    static bool saveSummary(const std::vector<SummaryRowData> &datas);
    static bool readSummary(std::vector<SummaryRowData> &datas);
private slots:

    void on_pushButton_pressed();

    void on_pushButton_2_pressed();

    void on_pushButton_3_pressed();

    void on_pushButton_4_pressed();

    void on_pushButton_5_pressed();

    void on_pushButton_6_pressed();

    void on_pushButton_7_pressed();

    void on_pushButton_8_pressed();

    void on_pushButton_11_pressed();

    void on_pushButton_12_pressed();

    void on_pushButton_9_pressed();

    void on_pushButton_10_pressed();

private:
    void loadRows();
    void clearRows();
    void resetSortButtons();

private:
    Ui::SummaryWidget *ui;
    std::vector<SummaryRowData> &datas;
    std::vector<QWidget *> rows;

public:
    static SummaryWidgetEventProxy eventProxy;
};

//////////    /////////  //    ///  /////////  //////     /////////  ///////
//////   //     ///      //   ///     ///      ///   //   ///        ///    //
/////   //     ///       //  ///     ///      ///   //   /////////  ///////
////   //     ///         /////     ///      ///   //   ///        /////
///////   /////////       ///   /////////  ///////    /////////  ///  ////

class SummaryRowData {
public:
    SummaryRowData(const QString &file_name, int num_errors, int num_norm, int num_effi, int num_main, int num_secu)
        : filePath(file_name), numErrors(num_errors), numNorm(num_norm), numEffi(num_effi), numMain(num_main),
          numSecu(num_secu) {
    }
    SummaryRowData() {
    }
    inline QString fileName() const {
        return filePath.mid(filePath.lastIndexOf("/") + 1);
    }
    /// data members
    QString filePath = "";
    int numErrors = 0;
    int numNorm = 0;
    int numEffi = 0;
    int numMain = 0;
    int numSecu = 0;
};

//////////    /////////  //    ///  /////////  //////     /////////  ///////
//////   //     ///      //   ///     ///      ///   //   ///        ///    //
/////   //     ///       //  ///     ///      ///   //   /////////  ///////
////   //     ///         /////     ///      ///   //   ///        /////
///////   /////////       ///   /////////  ///////    /////////  ///  ////

class SummaryRow : public QWidget {
public:
    SummaryRow(const SummaryRowData &data);
};

//////////    /////////  //    ///  /////////  //////     /////////  ///////
//////   //     ///      //   ///     ///      ///   //   ///        ///    //
/////   //     ///       //  ///     ///      ///   //   /////////  ///////
////   //     ///         /////     ///      ///   //   ///        /////
///////   /////////       ///   /////////  ///////    /////////  ///  ////

class SummaryWidgetEventProxy : public QObject {
    Q_OBJECT
signals:
    void openFile(const QString &file_path);
};

#endif // SUMMARY_H
