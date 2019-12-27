/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Mon Apr 20 10:17:25 2015
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDockWidget>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QStatusBar>
#include <QtGui/QTabWidget>
#include <QtGui/QTableWidget>
#include <QtGui/QToolBar>
#include <QtGui/QTreeWidget>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QGridLayout *gridLayout_2;
    QTabWidget *tabWidget;
    QWidget *tabWidgetPage1;
    QGridLayout *gridLayout_5;
    QTableWidget *tableWidget_board;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;
    QDockWidget *dockWidget;
    QWidget *dockWidgetContents;
    QGridLayout *gridLayout_3;
    QTreeWidget *treeWidget;
    QGroupBox *groupBox_config;
    QGridLayout *gridLayout;
    QLabel *label;
    QComboBox *comboBox_platform;
    QCheckBox *checkBox_last_image;
    QPushButton *pushButton_update;
    QLabel *label_4;
    QLineEdit *lineEdit_task_name;
    QPushButton *pushButton_confirm;
    QPushButton *pushButton_start;
    QCheckBox *checkBox_auto_emmd;
    QCheckBox *checkBox_last_package;
    QCheckBox *checkBox_console_log;
    QCheckBox *checkBox_add_time_flag;
    QFrame *line_2;
    QLabel *label_2;
    QLabel *label_log_setting;
    QCheckBox *checkBox_ex_setting;
    QCheckBox *checkBox_dealy_task;
    QComboBox *comboBox_runcase_mode;
    QCheckBox *checkBox_runcase_mode;
    QCheckBox *checkBox_lock_board;
    QCheckBox *checkBox_ignore_offline;
    QCheckBox *checkBox_auto_reboot;
    QDockWidget *dockWidget_4;
    QWidget *dockWidgetContents_4;
    QGridLayout *gridLayout_6;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1200, 1000);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(2);
        sizePolicy.setVerticalStretch(2);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        gridLayout_2 = new QGridLayout(centralWidget);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        tabWidget = new QTabWidget(centralWidget);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(2);
        sizePolicy1.setVerticalStretch(2);
        sizePolicy1.setHeightForWidth(tabWidget->sizePolicy().hasHeightForWidth());
        tabWidget->setSizePolicy(sizePolicy1);
        tabWidget->setTabsClosable(true);
        tabWidget->setMovable(true);
        tabWidgetPage1 = new QWidget();
        tabWidgetPage1->setObjectName(QString::fromUtf8("tabWidgetPage1"));
        gridLayout_5 = new QGridLayout(tabWidgetPage1);
        gridLayout_5->setSpacing(6);
        gridLayout_5->setContentsMargins(11, 11, 11, 11);
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        tableWidget_board = new QTableWidget(tabWidgetPage1);
        tableWidget_board->setObjectName(QString::fromUtf8("tableWidget_board"));
        QFont font;
        font.setPointSize(9);
        tableWidget_board->setFont(font);
        tableWidget_board->horizontalHeader()->setMinimumSectionSize(20);

        gridLayout_5->addWidget(tableWidget_board, 0, 0, 1, 1);

        tabWidget->addTab(tabWidgetPage1, QString());

        gridLayout_2->addWidget(tabWidget, 0, 0, 1, 1);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1200, 23));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);
        dockWidget = new QDockWidget(MainWindow);
        dockWidget->setObjectName(QString::fromUtf8("dockWidget"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(dockWidget->sizePolicy().hasHeightForWidth());
        dockWidget->setSizePolicy(sizePolicy2);
        dockWidget->setMinimumSize(QSize(812, 532));
        QFont font1;
        font1.setPointSize(10);
        dockWidget->setFont(font1);
#ifndef QT_NO_STATUSTIP
        dockWidget->setStatusTip(QString::fromUtf8(""));
#endif // QT_NO_STATUSTIP
        dockWidget->setAutoFillBackground(true);
        dockWidget->setFeatures(QDockWidget::DockWidgetFeatureMask);
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea|Qt::TopDockWidgetArea);
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QString::fromUtf8("dockWidgetContents"));
        gridLayout_3 = new QGridLayout(dockWidgetContents);
        gridLayout_3->setSpacing(6);
        gridLayout_3->setContentsMargins(11, 11, 11, 11);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        treeWidget = new QTreeWidget(dockWidgetContents);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QString::fromUtf8("1"));
        treeWidget->setHeaderItem(__qtreewidgetitem);
        treeWidget->setObjectName(QString::fromUtf8("treeWidget"));
        sizePolicy2.setHeightForWidth(treeWidget->sizePolicy().hasHeightForWidth());
        treeWidget->setSizePolicy(sizePolicy2);
        treeWidget->setFont(font);
        treeWidget->setSortingEnabled(true);
        treeWidget->setWordWrap(false);

        gridLayout_3->addWidget(treeWidget, 0, 0, 1, 1);

        groupBox_config = new QGroupBox(dockWidgetContents);
        groupBox_config->setObjectName(QString::fromUtf8("groupBox_config"));
        sizePolicy2.setHeightForWidth(groupBox_config->sizePolicy().hasHeightForWidth());
        groupBox_config->setSizePolicy(sizePolicy2);
        groupBox_config->setStyleSheet(QString::fromUtf8("QGroupBox {\n"
"    border-width:1px;   //\347\272\277\347\232\204\347\262\227\347\273\206\n"
"   border-style:solid;\n"
"  border-color:lightGray;   //\351\242\234\350\211\262\357\274\214\n"
"  margin-top: 0.5ex;  //\346\226\207\345\255\227\345\234\250\346\226\271\346\241\206\344\270\255\344\275\215\347\275\256\347\232\204\345\201\217\347\246\273\345\272\246\n"
"}\n"
"QGroupBox::title {\n"
"     subcontrol-origin: margin;\n"
"     subcontrol-position: top left;\n"
"   left:25px;       //\347\272\277\347\232\204\345\201\217\347\246\273\345\272\246\n"
"     margin-left: 0px;\n"
"     padding:0 1px;   //\346\226\207\345\255\227\345\234\250\346\226\271\346\241\206\344\270\255\344\275\215\347\275\256\347\232\204\345\201\217\347\246\273\345\272\246\n"
"}"));
        gridLayout = new QGridLayout(groupBox_config);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label = new QLabel(groupBox_config);
        label->setObjectName(QString::fromUtf8("label"));
        QFont font2;
        font2.setPointSize(11);
        font2.setBold(true);
        font2.setWeight(75);
        label->setFont(font2);

        gridLayout->addWidget(label, 0, 0, 1, 1);

        comboBox_platform = new QComboBox(groupBox_config);
        comboBox_platform->setObjectName(QString::fromUtf8("comboBox_platform"));
        comboBox_platform->setEnabled(true);

        gridLayout->addWidget(comboBox_platform, 0, 2, 1, 1);

        checkBox_last_image = new QCheckBox(groupBox_config);
        checkBox_last_image->setObjectName(QString::fromUtf8("checkBox_last_image"));
        checkBox_last_image->setChecked(false);

        gridLayout->addWidget(checkBox_last_image, 5, 0, 1, 3);

        pushButton_update = new QPushButton(groupBox_config);
        pushButton_update->setObjectName(QString::fromUtf8("pushButton_update"));

        gridLayout->addWidget(pushButton_update, 17, 0, 1, 2);

        label_4 = new QLabel(groupBox_config);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        QFont font3;
        font3.setBold(true);
        font3.setWeight(75);
        label_4->setFont(font3);

        gridLayout->addWidget(label_4, 18, 0, 1, 2);

        lineEdit_task_name = new QLineEdit(groupBox_config);
        lineEdit_task_name->setObjectName(QString::fromUtf8("lineEdit_task_name"));

        gridLayout->addWidget(lineEdit_task_name, 18, 2, 1, 1);

        pushButton_confirm = new QPushButton(groupBox_config);
        pushButton_confirm->setObjectName(QString::fromUtf8("pushButton_confirm"));

        gridLayout->addWidget(pushButton_confirm, 19, 0, 1, 2);

        pushButton_start = new QPushButton(groupBox_config);
        pushButton_start->setObjectName(QString::fromUtf8("pushButton_start"));
        pushButton_start->setEnabled(true);
        pushButton_start->setCheckable(false);
        pushButton_start->setChecked(false);

        gridLayout->addWidget(pushButton_start, 19, 2, 1, 1);

        checkBox_auto_emmd = new QCheckBox(groupBox_config);
        checkBox_auto_emmd->setObjectName(QString::fromUtf8("checkBox_auto_emmd"));
        checkBox_auto_emmd->setChecked(true);

        gridLayout->addWidget(checkBox_auto_emmd, 14, 0, 1, 3);

        checkBox_last_package = new QCheckBox(groupBox_config);
        checkBox_last_package->setObjectName(QString::fromUtf8("checkBox_last_package"));
        checkBox_last_package->setChecked(true);

        gridLayout->addWidget(checkBox_last_package, 4, 0, 1, 3);

        checkBox_console_log = new QCheckBox(groupBox_config);
        checkBox_console_log->setObjectName(QString::fromUtf8("checkBox_console_log"));
        checkBox_console_log->setChecked(true);

        gridLayout->addWidget(checkBox_console_log, 15, 0, 1, 3);

        checkBox_add_time_flag = new QCheckBox(groupBox_config);
        checkBox_add_time_flag->setObjectName(QString::fromUtf8("checkBox_add_time_flag"));
        checkBox_add_time_flag->setChecked(false);

        gridLayout->addWidget(checkBox_add_time_flag, 9, 0, 1, 3);

        line_2 = new QFrame(groupBox_config);
        line_2->setObjectName(QString::fromUtf8("line_2"));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);

        gridLayout->addWidget(line_2, 16, 0, 1, 3);

        label_2 = new QLabel(groupBox_config);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 7, 0, 1, 3);

        label_log_setting = new QLabel(groupBox_config);
        label_log_setting->setObjectName(QString::fromUtf8("label_log_setting"));

        gridLayout->addWidget(label_log_setting, 12, 0, 1, 3);

        checkBox_ex_setting = new QCheckBox(groupBox_config);
        checkBox_ex_setting->setObjectName(QString::fromUtf8("checkBox_ex_setting"));

        gridLayout->addWidget(checkBox_ex_setting, 6, 0, 1, 3);

        checkBox_dealy_task = new QCheckBox(groupBox_config);
        checkBox_dealy_task->setObjectName(QString::fromUtf8("checkBox_dealy_task"));

        gridLayout->addWidget(checkBox_dealy_task, 10, 0, 1, 3);

        comboBox_runcase_mode = new QComboBox(groupBox_config);
        comboBox_runcase_mode->setObjectName(QString::fromUtf8("comboBox_runcase_mode"));

        gridLayout->addWidget(comboBox_runcase_mode, 8, 2, 1, 1);

        checkBox_runcase_mode = new QCheckBox(groupBox_config);
        checkBox_runcase_mode->setObjectName(QString::fromUtf8("checkBox_runcase_mode"));

        gridLayout->addWidget(checkBox_runcase_mode, 8, 0, 1, 1);

        checkBox_lock_board = new QCheckBox(groupBox_config);
        checkBox_lock_board->setObjectName(QString::fromUtf8("checkBox_lock_board"));

        gridLayout->addWidget(checkBox_lock_board, 11, 0, 1, 3);

        checkBox_ignore_offline = new QCheckBox(groupBox_config);
        checkBox_ignore_offline->setObjectName(QString::fromUtf8("checkBox_ignore_offline"));

        gridLayout->addWidget(checkBox_ignore_offline, 17, 2, 1, 1);

        checkBox_auto_reboot = new QCheckBox(groupBox_config);
        checkBox_auto_reboot->setObjectName(QString::fromUtf8("checkBox_auto_reboot"));

        gridLayout->addWidget(checkBox_auto_reboot, 13, 0, 1, 3);


        gridLayout_3->addWidget(groupBox_config, 0, 1, 1, 1);

        dockWidget->setWidget(dockWidgetContents);
        MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(4), dockWidget);
        dockWidget_4 = new QDockWidget(MainWindow);
        dockWidget_4->setObjectName(QString::fromUtf8("dockWidget_4"));
        dockWidget_4->setFeatures(QDockWidget::AllDockWidgetFeatures);
        dockWidgetContents_4 = new QWidget();
        dockWidgetContents_4->setObjectName(QString::fromUtf8("dockWidgetContents_4"));
        gridLayout_6 = new QGridLayout(dockWidgetContents_4);
        gridLayout_6->setSpacing(6);
        gridLayout_6->setContentsMargins(11, 11, 11, 11);
        gridLayout_6->setObjectName(QString::fromUtf8("gridLayout_6"));
        dockWidget_4->setWidget(dockWidgetContents_4);
        MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(2), dockWidget_4);

        retranslateUi(MainWindow);
        QObject::connect(comboBox_platform, SIGNAL(currentIndexChanged(QString)), MainWindow, SLOT(change_list()));
        QObject::connect(pushButton_start, SIGNAL(clicked()), MainWindow, SLOT(start_test()));
        QObject::connect(pushButton_confirm, SIGNAL(clicked()), MainWindow, SLOT(clear_config()));
        QObject::connect(pushButton_update, SIGNAL(clicked()), MainWindow, SLOT(update_status()));
        QObject::connect(checkBox_last_package, SIGNAL(clicked()), MainWindow, SLOT(show_package()));
        QObject::connect(checkBox_last_image, SIGNAL(clicked()), MainWindow, SLOT(show_image_config()));
        QObject::connect(checkBox_ex_setting, SIGNAL(clicked()), MainWindow, SLOT(add_ex_setting()));
        QObject::connect(checkBox_dealy_task, SIGNAL(clicked(bool)), MainWindow, SLOT(choose_delay_tab(bool)));
        QObject::connect(checkBox_runcase_mode, SIGNAL(clicked(bool)), MainWindow, SLOT(check_runcase_mode(bool)));
        QObject::connect(tableWidget_board, SIGNAL(cellClicked(int,int)), MainWindow, SLOT(show_board_detail(int,int)));

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "LTK Cloud Server", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tabWidgetPage1), QString());
        dockWidget->setWindowTitle(QString());
        groupBox_config->setTitle(QString());
        label->setText(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Sans'; font-size:11pt; font-weight:600; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Platform</p></body></html>", 0, QApplication::UnicodeUTF8));
        comboBox_platform->clear();
        comboBox_platform->insertItems(0, QStringList()
         << QApplication::translate("MainWindow", "EDEN", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MainWindow", "ULC1", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MainWindow", "HELAN3", 0, QApplication::UnicodeUTF8)
        );
#ifndef QT_NO_TOOLTIP
        comboBox_platform->setToolTip(QApplication::translate("MainWindow", "Platform", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        checkBox_last_image->setText(QApplication::translate("MainWindow", "Burning New Image", 0, QApplication::UnicodeUTF8));
        pushButton_update->setText(QApplication::translate("MainWindow", "update", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("MainWindow", "Task Name:", 0, QApplication::UnicodeUTF8));
        pushButton_confirm->setText(QApplication::translate("MainWindow", "Refresh", 0, QApplication::UnicodeUTF8));
        pushButton_start->setText(QApplication::translate("MainWindow", "Start", 0, QApplication::UnicodeUTF8));
        checkBox_auto_emmd->setText(QApplication::translate("MainWindow", "Auto EMMD For Panic/Hung", 0, QApplication::UnicodeUTF8));
        checkBox_last_package->setText(QApplication::translate("MainWindow", "Use Latest Test Package", 0, QApplication::UnicodeUTF8));
        checkBox_console_log->setText(QApplication::translate("MainWindow", "Enable Console Capture and Logcat", 0, QApplication::UnicodeUTF8));
        checkBox_add_time_flag->setText(QApplication::translate("MainWindow", "Add Time Flag Into Task", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Sans'; font-size:10pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">Task Setting</span> <span style=\" color:#c8c8c8;\">-------------------------------------------------------------------------------------------</span></p></body></html>", 0, QApplication::UnicodeUTF8));
        label_log_setting->setText(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Sans'; font-size:10pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">Log Setting</span><span style=\" color:#c8c8c8;\">----------------------------------------------------------------------------------------------</span></p></body></html>", 0, QApplication::UnicodeUTF8));
        checkBox_ex_setting->setText(QApplication::translate("MainWindow", "Add Extra Setting Before Test", 0, QApplication::UnicodeUTF8));
        checkBox_dealy_task->setText(QApplication::translate("MainWindow", "Use Delay Deployment", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        comboBox_runcase_mode->setToolTip(QApplication::translate("MainWindow", "serialonly -> For Suspend/Resume test\n"
"serial        -> For USB Test\n"
"adb           -> For Uart Test", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        checkBox_runcase_mode->setToolTip(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Sans'; font-size:10pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:9pt; font-weight:600;\">Checked:</span><span style=\" font-size:9pt;\"> Run Case Mode will auto selected</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:9pt; font-weight:600;\">Un-checked:</span><span style=\" font-size:9pt;\"> Run case with selected mode</span></p></body></html>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        checkBox_runcase_mode->setText(QApplication::translate("MainWindow", "Run Case Mode", 0, QApplication::UnicodeUTF8));
        checkBox_lock_board->setText(QApplication::translate("MainWindow", "Lock Board After Test", 0, QApplication::UnicodeUTF8));
        checkBox_ignore_offline->setText(QApplication::translate("MainWindow", "Ignore Offline", 0, QApplication::UnicodeUTF8));
        checkBox_auto_reboot->setText(QApplication::translate("MainWindow", "Atuo Reboot For Panic/Hung", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        dockWidget_4->setToolTip(QApplication::translate("MainWindow", "Config list", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
