/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Tue Mar 24 11:14:44 2015
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
#include <QtGui/QComboBox>
#include <QtGui/QFrame>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QStatusBar>
#include <QtGui/QTextEdit>
#include <QtGui/QTreeWidget>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QPushButton *pushButton_unshare;
    QListWidget *listWidget_list;
    QPushButton *pushButton_list;
    QPushButton *pushButton_share;
    QPushButton *pushButton_reboot;
    QPushButton *pushButton_download;
    QPushButton *pushButton_power_on;
    QPushButton *pushButton_power_off;
    QPushButton *pushButton_power_off_soft;
    QPushButton *pushButton_onkey;
    QFrame *line;
    QLabel *label_2;
    QFrame *line_2;
    QFrame *line_3;
    QLabel *label;
    QPushButton *pushButton_fw_update;
    QFrame *line_4;
    QPushButton *pushButton_emmd_dump;
    QWidget *widget_burn_image;
    QPushButton *pushButton_burning_image;
    QComboBox *comboBox;
    QTreeWidget *ltk_image_tree;
    QTextEdit *textEdit_burn_detail;
    QLineEdit *lineEdit_burn_result;
    QPushButton *pushButton_update_list;
    QPushButton *pushButton_clear_log;
    QPushButton *pushButton_burn_switch;
    QTextEdit *textEdit_detail;
    QComboBox *comboBox_branch_list;
    QLabel *label_6;
    QComboBox *comboBox_register_level;
    QMenuBar *menuBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(407, 712);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        pushButton_unshare = new QPushButton(centralWidget);
        pushButton_unshare->setObjectName(QString::fromUtf8("pushButton_unshare"));
        pushButton_unshare->setGeometry(QRect(310, 55, 91, 35));
        listWidget_list = new QListWidget(centralWidget);
        listWidget_list->setObjectName(QString::fromUtf8("listWidget_list"));
        listWidget_list->setGeometry(QRect(10, 5, 291, 87));
        QFont font;
        font.setPointSize(9);
        listWidget_list->setFont(font);
        pushButton_list = new QPushButton(centralWidget);
        pushButton_list->setObjectName(QString::fromUtf8("pushButton_list"));
        pushButton_list->setGeometry(QRect(210, 37, 91, 51));
        QFont font1;
        font1.setPointSize(12);
        pushButton_list->setFont(font1);
        pushButton_share = new QPushButton(centralWidget);
        pushButton_share->setObjectName(QString::fromUtf8("pushButton_share"));
        pushButton_share->setGeometry(QRect(310, 10, 91, 35));
        pushButton_reboot = new QPushButton(centralWidget);
        pushButton_reboot->setObjectName(QString::fromUtf8("pushButton_reboot"));
        pushButton_reboot->setGeometry(QRect(310, 150, 91, 40));
        pushButton_reboot->setFont(font);
        pushButton_download = new QPushButton(centralWidget);
        pushButton_download->setObjectName(QString::fromUtf8("pushButton_download"));
        pushButton_download->setGeometry(QRect(310, 260, 91, 40));
        pushButton_download->setFont(font);
        pushButton_power_on = new QPushButton(centralWidget);
        pushButton_power_on->setObjectName(QString::fromUtf8("pushButton_power_on"));
        pushButton_power_on->setGeometry(QRect(310, 315, 91, 40));
        pushButton_power_on->setFont(font);
        pushButton_power_off = new QPushButton(centralWidget);
        pushButton_power_off->setObjectName(QString::fromUtf8("pushButton_power_off"));
        pushButton_power_off->setGeometry(QRect(310, 425, 91, 40));
        pushButton_power_off->setFont(font);
        pushButton_power_off_soft = new QPushButton(centralWidget);
        pushButton_power_off_soft->setObjectName(QString::fromUtf8("pushButton_power_off_soft"));
        pushButton_power_off_soft->setGeometry(QRect(310, 480, 91, 40));
        pushButton_power_off_soft->setFont(font);
        pushButton_onkey = new QPushButton(centralWidget);
        pushButton_onkey->setObjectName(QString::fromUtf8("pushButton_onkey"));
        pushButton_onkey->setGeometry(QRect(310, 205, 91, 40));
        pushButton_onkey->setFont(font);
        line = new QFrame(centralWidget);
        line->setObjectName(QString::fromUtf8("line"));
        line->setGeometry(QRect(10, 90, 401, 20));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        label_2 = new QLabel(centralWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(305, 110, 101, 17));
        QFont font2;
        font2.setPointSize(8);
        font2.setBold(true);
        font2.setWeight(75);
        label_2->setFont(font2);
        line_2 = new QFrame(centralWidget);
        line_2->setObjectName(QString::fromUtf8("line_2"));
        line_2->setGeometry(QRect(295, 0, 20, 691));
        line_2->setFrameShape(QFrame::VLine);
        line_2->setFrameShadow(QFrame::Sunken);
        line_3 = new QFrame(centralWidget);
        line_3->setObjectName(QString::fromUtf8("line_3"));
        line_3->setGeometry(QRect(20, -10, 381, 20));
        line_3->setFrameShape(QFrame::HLine);
        line_3->setFrameShadow(QFrame::Sunken);
        label = new QLabel(centralWidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(330, 130, 62, 17));
        QFont font3;
        font3.setBold(true);
        font3.setWeight(75);
        label->setFont(font3);
        pushButton_fw_update = new QPushButton(centralWidget);
        pushButton_fw_update->setObjectName(QString::fromUtf8("pushButton_fw_update"));
        pushButton_fw_update->setGeometry(QRect(310, 535, 91, 41));
        pushButton_fw_update->setFont(font);
        line_4 = new QFrame(centralWidget);
        line_4->setObjectName(QString::fromUtf8("line_4"));
        line_4->setGeometry(QRect(307, 580, 111, 20));
        line_4->setFrameShape(QFrame::HLine);
        line_4->setFrameShadow(QFrame::Sunken);
        pushButton_emmd_dump = new QPushButton(centralWidget);
        pushButton_emmd_dump->setObjectName(QString::fromUtf8("pushButton_emmd_dump"));
        pushButton_emmd_dump->setGeometry(QRect(310, 370, 91, 40));
        pushButton_emmd_dump->setFont(font);
        widget_burn_image = new QWidget(centralWidget);
        widget_burn_image->setObjectName(QString::fromUtf8("widget_burn_image"));
        widget_burn_image->setGeometry(QRect(0, 105, 301, 571));
        widget_burn_image->setStyleSheet(QString::fromUtf8("border-top-color: qconicalgradient(cx:0.5, cy:0.5, angle:0, stop:0 rgba(255, 255, 255, 255), stop:0.373979 rgba(255, 255, 255, 255), stop:0.373991 rgba(33, 30, 255, 255), stop:0.624018 rgba(33, 30, 255, 255), stop:0.624043 rgba(255, 0, 0, 255), stop:1 rgba(255, 0, 0, 255));"));
        pushButton_burning_image = new QPushButton(widget_burn_image);
        pushButton_burning_image->setObjectName(QString::fromUtf8("pushButton_burning_image"));
        pushButton_burning_image->setGeometry(QRect(198, 30, 91, 27));
        comboBox = new QComboBox(widget_burn_image);
        comboBox->setObjectName(QString::fromUtf8("comboBox"));
        comboBox->setGeometry(QRect(49, 10, 141, 27));
        ltk_image_tree = new QTreeWidget(widget_burn_image);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QString::fromUtf8("1"));
        ltk_image_tree->setHeaderItem(__qtreewidgetitem);
        ltk_image_tree->setObjectName(QString::fromUtf8("ltk_image_tree"));
        ltk_image_tree->setGeometry(QRect(9, 125, 281, 451));
        textEdit_burn_detail = new QTextEdit(widget_burn_image);
        textEdit_burn_detail->setObjectName(QString::fromUtf8("textEdit_burn_detail"));
        textEdit_burn_detail->setGeometry(QRect(51, 30, 141, 93));
        lineEdit_burn_result = new QLineEdit(widget_burn_image);
        lineEdit_burn_result->setObjectName(QString::fromUtf8("lineEdit_burn_result"));
        lineEdit_burn_result->setGeometry(QRect(198, 90, 91, 27));
        QFont font4;
        font4.setPointSize(8);
        lineEdit_burn_result->setFont(font4);
        lineEdit_burn_result->setStyleSheet(QString::fromUtf8("color: rgb(85, 255, 0);"));
        pushButton_update_list = new QPushButton(widget_burn_image);
        pushButton_update_list->setObjectName(QString::fromUtf8("pushButton_update_list"));
        pushButton_update_list->setGeometry(QRect(198, 10, 91, 27));
        pushButton_clear_log = new QPushButton(widget_burn_image);
        pushButton_clear_log->setObjectName(QString::fromUtf8("pushButton_clear_log"));
        pushButton_clear_log->setGeometry(QRect(198, 60, 91, 27));
        pushButton_burn_switch = new QPushButton(widget_burn_image);
        pushButton_burn_switch->setObjectName(QString::fromUtf8("pushButton_burn_switch"));
        pushButton_burn_switch->setGeometry(QRect(130, 230, 93, 41));
        textEdit_detail = new QTextEdit(widget_burn_image);
        textEdit_detail->setObjectName(QString::fromUtf8("textEdit_detail"));
        textEdit_detail->setGeometry(QRect(10, 0, 291, 581));
        textEdit_detail->setTextInteractionFlags(Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);
        comboBox_branch_list = new QComboBox(centralWidget);
        comboBox_branch_list->setObjectName(QString::fromUtf8("comboBox_branch_list"));
        comboBox_branch_list->setGeometry(QRect(190, 610, 91, 31));
        label_6 = new QLabel(centralWidget);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(315, 600, 81, 20));
        QFont font5;
        font5.setPointSize(9);
        font5.setBold(true);
        font5.setWeight(75);
        label_6->setFont(font5);
        comboBox_register_level = new QComboBox(centralWidget);
        comboBox_register_level->setObjectName(QString::fromUtf8("comboBox_register_level"));
        comboBox_register_level->setGeometry(QRect(310, 630, 89, 31));
        MainWindow->setCentralWidget(centralWidget);
        comboBox_branch_list->raise();
        pushButton_list->raise();
        pushButton_unshare->raise();
        listWidget_list->raise();
        pushButton_share->raise();
        pushButton_reboot->raise();
        pushButton_download->raise();
        pushButton_power_on->raise();
        pushButton_power_off->raise();
        pushButton_power_off_soft->raise();
        pushButton_onkey->raise();
        line->raise();
        label_2->raise();
        line_2->raise();
        line_3->raise();
        label->raise();
        pushButton_fw_update->raise();
        line_4->raise();
        pushButton_emmd_dump->raise();
        widget_burn_image->raise();
        label_6->raise();
        comboBox_register_level->raise();
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 407, 23));
        MainWindow->setMenuBar(menuBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);
        QObject::connect(pushButton_list, SIGNAL(clicked()), MainWindow, SLOT(list_board()));
        QObject::connect(listWidget_list, SIGNAL(currentTextChanged(QString)), MainWindow, SLOT(show_board_detail(QString)));
        QObject::connect(pushButton_share, SIGNAL(clicked()), MainWindow, SLOT(share_board()));
        QObject::connect(pushButton_unshare, SIGNAL(clicked()), MainWindow, SLOT(unshare_board()));
        QObject::connect(pushButton_reboot, SIGNAL(clicked()), MainWindow, SLOT(reboot()));
        QObject::connect(pushButton_onkey, SIGNAL(clicked()), MainWindow, SLOT(onkey()));
        QObject::connect(pushButton_download, SIGNAL(clicked()), MainWindow, SLOT(download()));
        QObject::connect(pushButton_power_on, SIGNAL(clicked()), MainWindow, SLOT(power_on()));
        QObject::connect(pushButton_power_off, SIGNAL(clicked()), MainWindow, SLOT(power_off()));
        QObject::connect(pushButton_power_off_soft, SIGNAL(clicked()), MainWindow, SLOT(power_off_soft()));
        QObject::connect(pushButton_fw_update, SIGNAL(clicked()), MainWindow, SLOT(fw_update()));
        QObject::connect(pushButton_emmd_dump, SIGNAL(clicked()), MainWindow, SLOT(emmd_dump()));
        QObject::connect(pushButton_burn_switch, SIGNAL(clicked()), MainWindow, SLOT(show_burning_setting()));
        QObject::connect(pushButton_update_list, SIGNAL(clicked()), MainWindow, SLOT(load_blf_map()));

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "LTK Cloud Client", 0, QApplication::UnicodeUTF8));
        pushButton_unshare->setText(QApplication::translate("MainWindow", "Unshare", 0, QApplication::UnicodeUTF8));
        pushButton_list->setText(QApplication::translate("MainWindow", "List", 0, QApplication::UnicodeUTF8));
        pushButton_share->setText(QApplication::translate("MainWindow", "Share", 0, QApplication::UnicodeUTF8));
        pushButton_reboot->setText(QApplication::translate("MainWindow", "Reboot", 0, QApplication::UnicodeUTF8));
        pushButton_download->setText(QApplication::translate("MainWindow", "Download", 0, QApplication::UnicodeUTF8));
        pushButton_power_on->setText(QApplication::translate("MainWindow", "Power_On", 0, QApplication::UnicodeUTF8));
        pushButton_power_off->setText(QApplication::translate("MainWindow", "Power_Off_HW", 0, QApplication::UnicodeUTF8));
        pushButton_power_off_soft->setText(QApplication::translate("MainWindow", "Power_Off_SW", 0, QApplication::UnicodeUTF8));
        pushButton_onkey->setText(QApplication::translate("MainWindow", "On_Key", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("MainWindow", "Remote Control", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("MainWindow", "Center", 0, QApplication::UnicodeUTF8));
        pushButton_fw_update->setText(QApplication::translate("MainWindow", "FW Update", 0, QApplication::UnicodeUTF8));
        pushButton_emmd_dump->setText(QApplication::translate("MainWindow", "EMMD_Dump", 0, QApplication::UnicodeUTF8));
        pushButton_burning_image->setText(QApplication::translate("MainWindow", "Burn Image", 0, QApplication::UnicodeUTF8));
        textEdit_burn_detail->setHtml(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Sans'; font-size:10pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">buring status</p></body></html>", 0, QApplication::UnicodeUTF8));
        lineEdit_burn_result->setText(QApplication::translate("MainWindow", "Result: Success", 0, QApplication::UnicodeUTF8));
        pushButton_update_list->setText(QApplication::translate("MainWindow", "Update List", 0, QApplication::UnicodeUTF8));
        pushButton_clear_log->setText(QApplication::translate("MainWindow", "Clear Log", 0, QApplication::UnicodeUTF8));
        pushButton_burn_switch->setText(QApplication::translate("MainWindow", "Burn Image", 0, QApplication::UnicodeUTF8));
        textEdit_detail->setHtml(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Sans'; font-size:10pt; font-weight:400; font-style:normal;\">\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"></p></body></html>", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        comboBox_branch_list->setToolTip(QApplication::translate("MainWindow", "If you register branch info LTK cloud,After LTK cloud test finished,\n"
"LTK cloud will revert the board to the latest branch you sellected,\n"
"If no Hung/fail happened on board to keep environment,\n"
"\n"
"", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_6->setText(QApplication::translate("MainWindow", "Share Level", 0, QApplication::UnicodeUTF8));
        comboBox_register_level->clear();
        comboBox_register_level->insertItems(0, QStringList()
         << QApplication::translate("MainWindow", "Global", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MainWindow", "Team", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MainWindow", "Local", 0, QApplication::UnicodeUTF8)
        );
#ifndef QT_NO_TOOLTIP
        comboBox_register_level->setToolTip(QApplication::translate("MainWindow", "Global :  Board can be assigned all task \n"
"Team   :  Board can be assigned the task form your team\n"
"Local   :  Board can be assigned the task form yourself", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
