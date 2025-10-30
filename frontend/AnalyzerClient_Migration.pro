QT += core gui widgets network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    AnalysisLabel.cpp \
    AnalyzeConfig.cpp \
    AnalyzeResultHandler.cpp \
    AnalyzeThread.cpp \
    AnalyzerClient.cpp \
    AnalyzingPopPage.cpp \
    AutoComplete.cpp \
    CCTextEdit.cpp \
    CloneExtractPreview.cpp \
    CodeBox.cpp \
    CodeComparison.cpp \
    CodeEditor.cpp \
    CreateProjectWidget.cpp \
    FileLoadingPopPage.cpp \
    FileManager.cpp \
    FileMenu.cpp \
    FileTabBar.cpp \
    FindAndReplaceDlg.cpp \
    GraphicsEffectUtil.cpp \
    HighlighterCode.cpp \
    HighlighterLabel.cpp \
    InfoStruct.cpp \
    LabelManager.cpp \
    LineNumberList.cpp \
    LocalDB.cpp \
    LoginWidget.cpp \
    MainWidget.cpp \
    PieChart.cpp \
    RemoteDB.cpp \
    ScoreWidget.cpp \
    SettingWidget.cpp \
    SummaryWidget.cpp \
    TextEdit.cpp \
    Toast.cpp \
    UrlInterpreter.cpp \
    Utils.cpp \
    WelcomePage.cpp \
    jsonutil.cpp \
    main.cpp

HEADERS += \
    AnalysisLabel.h \
    AnalyzeConfig.h \
    AnalyzeResultHandler.h \
    AnalyzeThread.h \
    AnalyzerClient.h \
    AnalyzingPopPage.h \
    AutoComplete.h \
    CCTextEdit.h \
    CloneExtractPreview.h \
    CodeBox.h \
    CodeBoxFile.h \
    CodeComparison.h \
    CodeEditor.h \
    CreateProjectWidget.h \
    Enums.h \
    FileLoadingPopPage.h \
    FileManager.h \
    FileMenu.h \
    FileTabBar.h \
    FindAndReplaceDlg.h \
    GraphicsEffectUtil.h \
    HighlighterCode.h \
    HighlighterLabel.h \
    InfoStruct.h \
    LabelManager.h \
    LineNumberList.h \
    LocalDB.h \
    LoginWidget.h \
    MainWidget.h \
    PieChart.h \
    RemoteDB.h \
    ScoreWidget.h \
    Serverex.h \
    SettingWidget.h \
    SummaryWidget.h \
    TextEdit.h \
    Toast.h \
    UrlDef.h \
    UrlInterpreter.h \
    Utils.h \
    WelcomePage.h \
    jsonutil.h

FORMS += \
    AnalyzingPopPage.ui \
    CloneExtractPreview.ui \
    CodeBox.ui \
    CodeComparison.ui \
    CreateProjectWidget.ui \
    FileLoadingPopPage.ui \
    FileManager.ui \
    FindAndReplaceDlg.ui \
    ScoreWidget.ui \
    SettingWidget.ui \
    SummaryWidget.ui \
    WelcomePage.ui \
    analyzerclient.ui \
    loginwidget.ui \
    mainwidget.ui


INCLUDEPATH += $$PWD/boost
CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/lib -llibboost_json-vc143-mt-gd-x64-1_81 -L$$PWD -llibboost_container-vc143-mt-gd-x64-1_81
} else {
    LIBS += -L$$PWD/lib -llibboost_json-vc143-mt-x64-1_81 -L$$PWD -llibboost_container-vc143-mt-x64-1_81
}

RESOURCES += \
    images.qrc

