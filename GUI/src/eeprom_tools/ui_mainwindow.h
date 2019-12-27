/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Fri Aug 22 10:59:29 2014
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
#include <QtGui/QFrame>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QStatusBar>
#include <QtGui/QTextEdit>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QCheckBox *checkBox_board_type;
    QLineEdit *lineEdit_board_type;
    QCheckBox *checkBox_board_id;
    QCheckBox *checkBox_chip_name;
    QCheckBox *checkBox_board_register_date;
    QCheckBox *checkBox_chip_stepping;
    QCheckBox *checkBox_board_eco;
    QCheckBox *checkBox_current_user;
    QCheckBox *checkBox_user_team;
    QCheckBox *checkBox_board_status;
    QCheckBox *checkBox_lcd_resolution;
    QCheckBox *checkBox_lcd_screensize;
    QCheckBox *checkBox_ddr_type;
    QCheckBox *checkBox_ddr_size;
    QCheckBox *checkBox_emmc_type;
    QCheckBox *checkBox_emmc_size;
    QCheckBox *checkBox_rf_type;
    QCheckBox *checkBox_rf_name;
    QLineEdit *lineEdit_board_id;
    QLineEdit *lineEdit_chip_name;
    QLineEdit *lineEdit_chip_stepping;
    QLineEdit *lineEdit_board_register_date;
    QLineEdit *lineEdit_board_status;
    QLineEdit *lineEdit_user_team;
    QLineEdit *lineEdit_board_eco;
    QLineEdit *lineEdit_current_user;
    QLineEdit *lineEdit_lcd_resolution;
    QLineEdit *lineEdit_lcd_screensize;
    QLineEdit *lineEdit_ddr_type;
    QLineEdit *lineEdit_ddr_size;
    QLineEdit *lineEdit_emmc_size;
    QLineEdit *lineEdit_emmc_type;
    QLineEdit *lineEdit_rf_name;
    QLineEdit *lineEdit_rf_type;
    QPushButton *pushButton_read;
    QComboBox *comboBox_config;
    QPushButton *pushButton_save;
    QPushButton *pushButton_load;
    QCheckBox *checkBox_select_all;
    QFrame *line;
    QFrame *line_2;
    QFrame *line_3;
    QComboBox *comboBox_lcd_resolution;
    QPushButton *pushButton_write;
    QComboBox *comboBox_lcd_screensize;
    QLabel *label;
    QLabel *label_2;
    QFrame *line_4;
    QFrame *line_5;
    QComboBox *comboBox_chip_name;
    QComboBox *comboBox_chip_stepping;
    QComboBox *comboBox_ddr_type;
    QComboBox *comboBox_ddr_size;
    QComboBox *comboBox_emmc_type;
    QComboBox *comboBox_emmc_size;
    QPushButton *pushButton_clear;
    QFrame *line_6;
    QFrame *line_7;
    QFrame *line_8;
    QFrame *line_9;
    QFrame *line_10;
    QFrame *line_11;
    QFrame *line_12;
    QPushButton *pushButton_read_all;
    QTextEdit *textEdit;
    QTextEdit *textEdit_log;
    QComboBox *comboBox_ddr_freq;
    QComboBox *comboBox_ddr_lp_type;
    QMenuBar *menuBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(700, 673);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        checkBox_board_type = new QCheckBox(centralWidget);
        checkBox_board_type->setObjectName(QString::fromUtf8("checkBox_board_type"));
        checkBox_board_type->setGeometry(QRect(10, 52, 121, 20));
        QFont font;
        font.setPointSize(12);
        checkBox_board_type->setFont(font);
        checkBox_board_type->setMouseTracking(true);
        lineEdit_board_type = new QLineEdit(centralWidget);
        lineEdit_board_type->setObjectName(QString::fromUtf8("lineEdit_board_type"));
        lineEdit_board_type->setGeometry(QRect(210, 52, 251, 25));
        lineEdit_board_type->setFont(font);
        checkBox_board_id = new QCheckBox(centralWidget);
        checkBox_board_id->setObjectName(QString::fromUtf8("checkBox_board_id"));
        checkBox_board_id->setGeometry(QRect(10, 77, 121, 20));
        checkBox_board_id->setFont(font);
        checkBox_chip_name = new QCheckBox(centralWidget);
        checkBox_chip_name->setObjectName(QString::fromUtf8("checkBox_chip_name"));
        checkBox_chip_name->setGeometry(QRect(10, 340, 121, 20));
        checkBox_chip_name->setFont(font);
        checkBox_board_register_date = new QCheckBox(centralWidget);
        checkBox_board_register_date->setObjectName(QString::fromUtf8("checkBox_board_register_date"));
        checkBox_board_register_date->setGeometry(QRect(10, 102, 191, 20));
        checkBox_board_register_date->setFont(font);
        checkBox_chip_stepping = new QCheckBox(centralWidget);
        checkBox_chip_stepping->setObjectName(QString::fromUtf8("checkBox_chip_stepping"));
        checkBox_chip_stepping->setGeometry(QRect(10, 364, 141, 20));
        checkBox_chip_stepping->setFont(font);
        checkBox_board_eco = new QCheckBox(centralWidget);
        checkBox_board_eco->setObjectName(QString::fromUtf8("checkBox_board_eco"));
        checkBox_board_eco->setGeometry(QRect(10, 223, 121, 20));
        checkBox_board_eco->setFont(font);
        checkBox_current_user = new QCheckBox(centralWidget);
        checkBox_current_user->setObjectName(QString::fromUtf8("checkBox_current_user"));
        checkBox_current_user->setGeometry(QRect(10, 200, 141, 20));
        checkBox_current_user->setFont(font);
        checkBox_user_team = new QCheckBox(centralWidget);
        checkBox_user_team->setObjectName(QString::fromUtf8("checkBox_user_team"));
        checkBox_user_team->setGeometry(QRect(10, 174, 121, 20));
        checkBox_user_team->setFont(font);
        checkBox_board_status = new QCheckBox(centralWidget);
        checkBox_board_status->setObjectName(QString::fromUtf8("checkBox_board_status"));
        checkBox_board_status->setGeometry(QRect(10, 150, 141, 20));
        checkBox_board_status->setFont(font);
        checkBox_lcd_resolution = new QCheckBox(centralWidget);
        checkBox_lcd_resolution->setObjectName(QString::fromUtf8("checkBox_lcd_resolution"));
        checkBox_lcd_resolution->setGeometry(QRect(10, 264, 141, 20));
        checkBox_lcd_resolution->setFont(font);
        checkBox_lcd_screensize = new QCheckBox(centralWidget);
        checkBox_lcd_screensize->setObjectName(QString::fromUtf8("checkBox_lcd_screensize"));
        checkBox_lcd_screensize->setGeometry(QRect(10, 291, 151, 20));
        checkBox_lcd_screensize->setFont(font);
        checkBox_ddr_type = new QCheckBox(centralWidget);
        checkBox_ddr_type->setObjectName(QString::fromUtf8("checkBox_ddr_type"));
        checkBox_ddr_type->setGeometry(QRect(10, 388, 121, 20));
        checkBox_ddr_type->setFont(font);
        checkBox_ddr_size = new QCheckBox(centralWidget);
        checkBox_ddr_size->setObjectName(QString::fromUtf8("checkBox_ddr_size"));
        checkBox_ddr_size->setGeometry(QRect(10, 412, 151, 20));
        checkBox_ddr_size->setFont(font);
        checkBox_emmc_type = new QCheckBox(centralWidget);
        checkBox_emmc_type->setObjectName(QString::fromUtf8("checkBox_emmc_type"));
        checkBox_emmc_type->setGeometry(QRect(10, 436, 121, 20));
        checkBox_emmc_type->setFont(font);
        checkBox_emmc_size = new QCheckBox(centralWidget);
        checkBox_emmc_size->setObjectName(QString::fromUtf8("checkBox_emmc_size"));
        checkBox_emmc_size->setGeometry(QRect(10, 460, 121, 20));
        checkBox_emmc_size->setFont(font);
        checkBox_rf_type = new QCheckBox(centralWidget);
        checkBox_rf_type->setObjectName(QString::fromUtf8("checkBox_rf_type"));
        checkBox_rf_type->setGeometry(QRect(10, 524, 121, 20));
        checkBox_rf_type->setFont(font);
        checkBox_rf_name = new QCheckBox(centralWidget);
        checkBox_rf_name->setObjectName(QString::fromUtf8("checkBox_rf_name"));
        checkBox_rf_name->setGeometry(QRect(10, 500, 121, 20));
        checkBox_rf_name->setFont(font);
        lineEdit_board_id = new QLineEdit(centralWidget);
        lineEdit_board_id->setObjectName(QString::fromUtf8("lineEdit_board_id"));
        lineEdit_board_id->setGeometry(QRect(210, 76, 251, 25));
        lineEdit_board_id->setFont(font);
        lineEdit_chip_name = new QLineEdit(centralWidget);
        lineEdit_chip_name->setObjectName(QString::fromUtf8("lineEdit_chip_name"));
        lineEdit_chip_name->setGeometry(QRect(210, 340, 131, 25));
        lineEdit_chip_name->setFont(font);
        lineEdit_chip_stepping = new QLineEdit(centralWidget);
        lineEdit_chip_stepping->setObjectName(QString::fromUtf8("lineEdit_chip_stepping"));
        lineEdit_chip_stepping->setGeometry(QRect(210, 364, 131, 25));
        lineEdit_chip_stepping->setFont(font);
        lineEdit_board_register_date = new QLineEdit(centralWidget);
        lineEdit_board_register_date->setObjectName(QString::fromUtf8("lineEdit_board_register_date"));
        lineEdit_board_register_date->setGeometry(QRect(210, 100, 251, 25));
        lineEdit_board_register_date->setFont(font);
        lineEdit_board_status = new QLineEdit(centralWidget);
        lineEdit_board_status->setObjectName(QString::fromUtf8("lineEdit_board_status"));
        lineEdit_board_status->setGeometry(QRect(210, 150, 251, 25));
        lineEdit_board_status->setFont(font);
        lineEdit_user_team = new QLineEdit(centralWidget);
        lineEdit_user_team->setObjectName(QString::fromUtf8("lineEdit_user_team"));
        lineEdit_user_team->setGeometry(QRect(210, 174, 251, 25));
        lineEdit_user_team->setFont(font);
        lineEdit_board_eco = new QLineEdit(centralWidget);
        lineEdit_board_eco->setObjectName(QString::fromUtf8("lineEdit_board_eco"));
        lineEdit_board_eco->setGeometry(QRect(210, 222, 251, 25));
        lineEdit_board_eco->setFont(font);
        lineEdit_current_user = new QLineEdit(centralWidget);
        lineEdit_current_user->setObjectName(QString::fromUtf8("lineEdit_current_user"));
        lineEdit_current_user->setGeometry(QRect(210, 198, 251, 25));
        lineEdit_current_user->setFont(font);
        lineEdit_lcd_resolution = new QLineEdit(centralWidget);
        lineEdit_lcd_resolution->setObjectName(QString::fromUtf8("lineEdit_lcd_resolution"));
        lineEdit_lcd_resolution->setGeometry(QRect(210, 267, 131, 25));
        lineEdit_lcd_resolution->setFont(font);
        lineEdit_lcd_screensize = new QLineEdit(centralWidget);
        lineEdit_lcd_screensize->setObjectName(QString::fromUtf8("lineEdit_lcd_screensize"));
        lineEdit_lcd_screensize->setGeometry(QRect(210, 291, 131, 25));
        lineEdit_lcd_screensize->setFont(font);
        lineEdit_ddr_type = new QLineEdit(centralWidget);
        lineEdit_ddr_type->setObjectName(QString::fromUtf8("lineEdit_ddr_type"));
        lineEdit_ddr_type->setGeometry(QRect(210, 388, 131, 25));
        QFont font1;
        font1.setPointSize(10);
        lineEdit_ddr_type->setFont(font1);
        lineEdit_ddr_size = new QLineEdit(centralWidget);
        lineEdit_ddr_size->setObjectName(QString::fromUtf8("lineEdit_ddr_size"));
        lineEdit_ddr_size->setGeometry(QRect(210, 412, 131, 25));
        lineEdit_ddr_size->setFont(font);
        lineEdit_emmc_size = new QLineEdit(centralWidget);
        lineEdit_emmc_size->setObjectName(QString::fromUtf8("lineEdit_emmc_size"));
        lineEdit_emmc_size->setGeometry(QRect(210, 460, 131, 25));
        lineEdit_emmc_size->setFont(font);
        lineEdit_emmc_type = new QLineEdit(centralWidget);
        lineEdit_emmc_type->setObjectName(QString::fromUtf8("lineEdit_emmc_type"));
        lineEdit_emmc_type->setGeometry(QRect(210, 436, 131, 25));
        lineEdit_emmc_type->setFont(font);
        lineEdit_rf_name = new QLineEdit(centralWidget);
        lineEdit_rf_name->setObjectName(QString::fromUtf8("lineEdit_rf_name"));
        lineEdit_rf_name->setGeometry(QRect(210, 500, 251, 25));
        lineEdit_rf_name->setFont(font);
        lineEdit_rf_type = new QLineEdit(centralWidget);
        lineEdit_rf_type->setObjectName(QString::fromUtf8("lineEdit_rf_type"));
        lineEdit_rf_type->setGeometry(QRect(210, 524, 251, 25));
        lineEdit_rf_type->setFont(font);
        pushButton_read = new QPushButton(centralWidget);
        pushButton_read->setObjectName(QString::fromUtf8("pushButton_read"));
        pushButton_read->setGeometry(QRect(490, 160, 81, 61));
        QFont font2;
        font2.setPointSize(12);
        font2.setBold(false);
        font2.setWeight(50);
        pushButton_read->setFont(font2);
        comboBox_config = new QComboBox(centralWidget);
        comboBox_config->setObjectName(QString::fromUtf8("comboBox_config"));
        comboBox_config->setGeometry(QRect(490, 360, 181, 27));
        comboBox_config->setFont(font);
        comboBox_config->setEditable(true);
        pushButton_save = new QPushButton(centralWidget);
        pushButton_save->setObjectName(QString::fromUtf8("pushButton_save"));
        pushButton_save->setGeometry(QRect(590, 400, 81, 61));
        pushButton_save->setFont(font2);
        pushButton_load = new QPushButton(centralWidget);
        pushButton_load->setObjectName(QString::fromUtf8("pushButton_load"));
        pushButton_load->setGeometry(QRect(490, 400, 81, 61));
        pushButton_load->setFont(font2);
        checkBox_select_all = new QCheckBox(centralWidget);
        checkBox_select_all->setObjectName(QString::fromUtf8("checkBox_select_all"));
        checkBox_select_all->setGeometry(QRect(10, 10, 111, 22));
        QFont font3;
        font3.setPointSize(12);
        font3.setBold(true);
        font3.setWeight(75);
        checkBox_select_all->setFont(font3);
        line = new QFrame(centralWidget);
        line->setObjectName(QString::fromUtf8("line"));
        line->setGeometry(QRect(10, 490, 451, 16));
        QFont font4;
        font4.setPointSize(10);
        font4.setBold(true);
        font4.setWeight(75);
        line->setFont(font4);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        line_2 = new QFrame(centralWidget);
        line_2->setObjectName(QString::fromUtf8("line_2"));
        line_2->setGeometry(QRect(10, 30, 461, 16));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);
        line_3 = new QFrame(centralWidget);
        line_3->setObjectName(QString::fromUtf8("line_3"));
        line_3->setGeometry(QRect(10, 320, 451, 16));
        line_3->setFont(font4);
        line_3->setFrameShape(QFrame::HLine);
        line_3->setFrameShadow(QFrame::Sunken);
        comboBox_lcd_resolution = new QComboBox(centralWidget);
        comboBox_lcd_resolution->setObjectName(QString::fromUtf8("comboBox_lcd_resolution"));
        comboBox_lcd_resolution->setGeometry(QRect(340, 267, 121, 25));
        comboBox_lcd_resolution->setFont(font);
        comboBox_lcd_resolution->setEditable(true);
        pushButton_write = new QPushButton(centralWidget);
        pushButton_write->setObjectName(QString::fromUtf8("pushButton_write"));
        pushButton_write->setGeometry(QRect(590, 160, 81, 61));
        pushButton_write->setFont(font2);
        comboBox_lcd_screensize = new QComboBox(centralWidget);
        comboBox_lcd_screensize->setObjectName(QString::fromUtf8("comboBox_lcd_screensize"));
        comboBox_lcd_screensize->setGeometry(QRect(340, 291, 121, 25));
        comboBox_lcd_screensize->setFont(font);
        comboBox_lcd_screensize->setEditable(true);
        label = new QLabel(centralWidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(490, 327, 201, 17));
        label->setFont(font3);
        label_2 = new QLabel(centralWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(490, 120, 201, 20));
        label_2->setFont(font3);
        line_4 = new QFrame(centralWidget);
        line_4->setObjectName(QString::fromUtf8("line_4"));
        line_4->setGeometry(QRect(10, 130, 451, 16));
        line_4->setFont(font4);
        line_4->setFrameShape(QFrame::HLine);
        line_4->setFrameShadow(QFrame::Sunken);
        line_5 = new QFrame(centralWidget);
        line_5->setObjectName(QString::fromUtf8("line_5"));
        line_5->setGeometry(QRect(10, 250, 451, 16));
        line_5->setFont(font4);
        line_5->setFrameShape(QFrame::HLine);
        line_5->setFrameShadow(QFrame::Sunken);
        comboBox_chip_name = new QComboBox(centralWidget);
        comboBox_chip_name->setObjectName(QString::fromUtf8("comboBox_chip_name"));
        comboBox_chip_name->setGeometry(QRect(340, 340, 121, 25));
        comboBox_chip_name->setFont(font);
        comboBox_chip_name->setEditable(true);
        comboBox_chip_stepping = new QComboBox(centralWidget);
        comboBox_chip_stepping->setObjectName(QString::fromUtf8("comboBox_chip_stepping"));
        comboBox_chip_stepping->setGeometry(QRect(340, 364, 121, 25));
        comboBox_chip_stepping->setFont(font);
        comboBox_chip_stepping->setEditable(true);
        comboBox_ddr_type = new QComboBox(centralWidget);
        comboBox_ddr_type->setObjectName(QString::fromUtf8("comboBox_ddr_type"));
        comboBox_ddr_type->setGeometry(QRect(340, 388, 65, 25));
        QFont font5;
        font5.setPointSize(8);
        comboBox_ddr_type->setFont(font5);
        comboBox_ddr_type->setEditable(true);
        comboBox_ddr_size = new QComboBox(centralWidget);
        comboBox_ddr_size->setObjectName(QString::fromUtf8("comboBox_ddr_size"));
        comboBox_ddr_size->setGeometry(QRect(340, 412, 65, 25));
        QFont font6;
        font6.setFamily(QString::fromUtf8("Andale Mono"));
        font6.setPointSize(11);
        comboBox_ddr_size->setFont(font6);
        comboBox_ddr_size->setEditable(true);
        comboBox_emmc_type = new QComboBox(centralWidget);
        comboBox_emmc_type->setObjectName(QString::fromUtf8("comboBox_emmc_type"));
        comboBox_emmc_type->setGeometry(QRect(340, 436, 121, 25));
        QFont font7;
        font7.setPointSize(11);
        comboBox_emmc_type->setFont(font7);
        comboBox_emmc_type->setEditable(true);
        comboBox_emmc_size = new QComboBox(centralWidget);
        comboBox_emmc_size->setObjectName(QString::fromUtf8("comboBox_emmc_size"));
        comboBox_emmc_size->setGeometry(QRect(340, 460, 121, 25));
        comboBox_emmc_size->setFont(font);
        comboBox_emmc_size->setEditable(true);
        pushButton_clear = new QPushButton(centralWidget);
        pushButton_clear->setObjectName(QString::fromUtf8("pushButton_clear"));
        pushButton_clear->setGeometry(QRect(490, 490, 181, 51));
        pushButton_clear->setFont(font2);
        line_6 = new QFrame(centralWidget);
        line_6->setObjectName(QString::fromUtf8("line_6"));
        line_6->setGeometry(QRect(460, 40, 20, 511));
        line_6->setFrameShape(QFrame::VLine);
        line_6->setFrameShadow(QFrame::Sunken);
        line_7 = new QFrame(centralWidget);
        line_7->setObjectName(QString::fromUtf8("line_7"));
        line_7->setGeometry(QRect(470, 230, 211, 16));
        line_7->setFrameShape(QFrame::HLine);
        line_7->setFrameShadow(QFrame::Sunken);
        line_8 = new QFrame(centralWidget);
        line_8->setObjectName(QString::fromUtf8("line_8"));
        line_8->setGeometry(QRect(470, 310, 211, 16));
        line_8->setFrameShape(QFrame::HLine);
        line_8->setFrameShadow(QFrame::Sunken);
        line_9 = new QFrame(centralWidget);
        line_9->setObjectName(QString::fromUtf8("line_9"));
        line_9->setGeometry(QRect(470, 470, 211, 16));
        line_9->setFrameShape(QFrame::HLine);
        line_9->setFrameShadow(QFrame::Sunken);
        line_10 = new QFrame(centralWidget);
        line_10->setObjectName(QString::fromUtf8("line_10"));
        line_10->setGeometry(QRect(470, 100, 211, 16));
        line_10->setFrameShape(QFrame::HLine);
        line_10->setFrameShadow(QFrame::Sunken);
        line_11 = new QFrame(centralWidget);
        line_11->setObjectName(QString::fromUtf8("line_11"));
        line_11->setGeometry(QRect(670, 40, 20, 511));
        line_11->setFrameShape(QFrame::VLine);
        line_11->setFrameShadow(QFrame::Sunken);
        line_12 = new QFrame(centralWidget);
        line_12->setObjectName(QString::fromUtf8("line_12"));
        line_12->setGeometry(QRect(470, 30, 211, 16));
        line_12->setFrameShape(QFrame::HLine);
        line_12->setFrameShadow(QFrame::Sunken);
        pushButton_read_all = new QPushButton(centralWidget);
        pushButton_read_all->setObjectName(QString::fromUtf8("pushButton_read_all"));
        pushButton_read_all->setGeometry(QRect(490, 250, 181, 51));
        pushButton_read_all->setFont(font2);
        textEdit = new QTextEdit(centralWidget);
        textEdit->setObjectName(QString::fromUtf8("textEdit"));
        textEdit->setGeometry(QRect(480, 50, 191, 51));
        textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        textEdit_log = new QTextEdit(centralWidget);
        textEdit_log->setObjectName(QString::fromUtf8("textEdit_log"));
        textEdit_log->setGeometry(QRect(10, 560, 671, 71));
        comboBox_ddr_freq = new QComboBox(centralWidget);
        comboBox_ddr_freq->setObjectName(QString::fromUtf8("comboBox_ddr_freq"));
        comboBox_ddr_freq->setGeometry(QRect(404, 412, 57, 25));
        comboBox_ddr_freq->setFont(font7);
        comboBox_ddr_freq->setEditable(true);
        comboBox_ddr_lp_type = new QComboBox(centralWidget);
        comboBox_ddr_lp_type->setObjectName(QString::fromUtf8("comboBox_ddr_lp_type"));
        comboBox_ddr_lp_type->setGeometry(QRect(404, 388, 57, 25));
        QFont font8;
        font8.setPointSize(7);
        comboBox_ddr_lp_type->setFont(font8);
        comboBox_ddr_lp_type->setEditable(true);
        MainWindow->setCentralWidget(centralWidget);
        lineEdit_lcd_resolution->raise();
        comboBox_lcd_resolution->raise();
        checkBox_board_type->raise();
        lineEdit_board_type->raise();
        checkBox_board_id->raise();
        checkBox_chip_name->raise();
        checkBox_board_register_date->raise();
        checkBox_chip_stepping->raise();
        checkBox_board_eco->raise();
        checkBox_current_user->raise();
        checkBox_user_team->raise();
        checkBox_board_status->raise();
        checkBox_lcd_resolution->raise();
        checkBox_lcd_screensize->raise();
        checkBox_ddr_type->raise();
        checkBox_ddr_size->raise();
        checkBox_emmc_type->raise();
        checkBox_emmc_size->raise();
        checkBox_rf_type->raise();
        checkBox_rf_name->raise();
        lineEdit_board_id->raise();
        lineEdit_chip_name->raise();
        lineEdit_chip_stepping->raise();
        lineEdit_board_register_date->raise();
        lineEdit_board_status->raise();
        lineEdit_user_team->raise();
        lineEdit_board_eco->raise();
        lineEdit_current_user->raise();
        lineEdit_lcd_screensize->raise();
        lineEdit_ddr_type->raise();
        lineEdit_ddr_size->raise();
        lineEdit_emmc_size->raise();
        lineEdit_emmc_type->raise();
        lineEdit_rf_name->raise();
        lineEdit_rf_type->raise();
        pushButton_read->raise();
        comboBox_config->raise();
        pushButton_save->raise();
        pushButton_load->raise();
        checkBox_select_all->raise();
        line->raise();
        line_2->raise();
        line_3->raise();
        pushButton_write->raise();
        comboBox_lcd_screensize->raise();
        label->raise();
        label_2->raise();
        line_4->raise();
        line_5->raise();
        comboBox_chip_name->raise();
        comboBox_chip_stepping->raise();
        comboBox_ddr_type->raise();
        comboBox_ddr_size->raise();
        comboBox_emmc_type->raise();
        comboBox_emmc_size->raise();
        pushButton_clear->raise();
        line_6->raise();
        line_7->raise();
        line_8->raise();
        line_9->raise();
        line_10->raise();
        line_11->raise();
        line_12->raise();
        pushButton_read_all->raise();
        textEdit->raise();
        textEdit_log->raise();
        comboBox_ddr_freq->raise();
        comboBox_ddr_lp_type->raise();
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 700, 23));
        MainWindow->setMenuBar(menuBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);
        QObject::connect(pushButton_save, SIGNAL(clicked()), MainWindow, SLOT(save_config()));
        QObject::connect(pushButton_load, SIGNAL(clicked()), MainWindow, SLOT(load_config()));
        QObject::connect(checkBox_select_all, SIGNAL(clicked()), MainWindow, SLOT(select_choose()));
        QObject::connect(lineEdit_board_id, SIGNAL(returnPressed()), lineEdit_board_id, SLOT(selectAll()));
        QObject::connect(comboBox_lcd_screensize, SIGNAL(editTextChanged(QString)), lineEdit_lcd_screensize, SLOT(setText(QString)));
        QObject::connect(comboBox_lcd_resolution, SIGNAL(editTextChanged(QString)), lineEdit_lcd_resolution, SLOT(setText(QString)));
        QObject::connect(comboBox_chip_name, SIGNAL(editTextChanged(QString)), lineEdit_chip_name, SLOT(setText(QString)));
        QObject::connect(comboBox_chip_stepping, SIGNAL(editTextChanged(QString)), lineEdit_chip_stepping, SLOT(setText(QString)));
        QObject::connect(comboBox_emmc_type, SIGNAL(editTextChanged(QString)), lineEdit_emmc_type, SLOT(setText(QString)));
        QObject::connect(comboBox_emmc_size, SIGNAL(editTextChanged(QString)), lineEdit_emmc_size, SLOT(setText(QString)));
        QObject::connect(pushButton_clear, SIGNAL(clicked()), MainWindow, SLOT(clear()));
        QObject::connect(pushButton_read_all, SIGNAL(clicked()), MainWindow, SLOT(read_all()));
        QObject::connect(pushButton_write, SIGNAL(clicked()), MainWindow, SLOT(write_all()));
        QObject::connect(pushButton_read, SIGNAL(clicked()), MainWindow, SLOT(read_item_ini()));
        QObject::connect(comboBox_ddr_size, SIGNAL(editTextChanged(QString)), MainWindow, SLOT(merge_ddr_size_speed()));
        QObject::connect(comboBox_ddr_freq, SIGNAL(editTextChanged(QString)), MainWindow, SLOT(merge_ddr_size_speed()));
        QObject::connect(comboBox_ddr_type, SIGNAL(currentIndexChanged(QString)), MainWindow, SLOT(merge_ddr_type_lp()));
        QObject::connect(comboBox_ddr_lp_type, SIGNAL(currentIndexChanged(QString)), MainWindow, SLOT(merge_ddr_type_lp()));

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "EEPROM Tool", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        MainWindow->setToolTip(QApplication::translate("MainWindow", "eeprom", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        checkBox_board_type->setText(QApplication::translate("MainWindow", "board_type", 0, QApplication::UnicodeUTF8));
        checkBox_board_id->setText(QApplication::translate("MainWindow", "board_id", 0, QApplication::UnicodeUTF8));
        checkBox_chip_name->setText(QApplication::translate("MainWindow", "chip_name", 0, QApplication::UnicodeUTF8));
        checkBox_board_register_date->setText(QApplication::translate("MainWindow", "board_register_date", 0, QApplication::UnicodeUTF8));
        checkBox_chip_stepping->setText(QApplication::translate("MainWindow", "chip_stepping ", 0, QApplication::UnicodeUTF8));
        checkBox_board_eco->setText(QApplication::translate("MainWindow", "board_eco", 0, QApplication::UnicodeUTF8));
        checkBox_current_user->setText(QApplication::translate("MainWindow", "board_owner", 0, QApplication::UnicodeUTF8));
        checkBox_user_team->setText(QApplication::translate("MainWindow", "user_team", 0, QApplication::UnicodeUTF8));
        checkBox_board_status->setText(QApplication::translate("MainWindow", "board_status", 0, QApplication::UnicodeUTF8));
        checkBox_lcd_resolution->setText(QApplication::translate("MainWindow", "lcd_resolution", 0, QApplication::UnicodeUTF8));
        checkBox_lcd_screensize->setText(QApplication::translate("MainWindow", "lcd_screensize", 0, QApplication::UnicodeUTF8));
        checkBox_ddr_type->setText(QApplication::translate("MainWindow", "ddr_type", 0, QApplication::UnicodeUTF8));
        checkBox_ddr_size->setText(QApplication::translate("MainWindow", "ddr_size_freq", 0, QApplication::UnicodeUTF8));
        checkBox_emmc_type->setText(QApplication::translate("MainWindow", "emmc_type", 0, QApplication::UnicodeUTF8));
        checkBox_emmc_size->setText(QApplication::translate("MainWindow", "emmc_size", 0, QApplication::UnicodeUTF8));
        checkBox_rf_type->setText(QApplication::translate("MainWindow", "rf_type", 0, QApplication::UnicodeUTF8));
        checkBox_rf_name->setText(QApplication::translate("MainWindow", "rf_name", 0, QApplication::UnicodeUTF8));
        pushButton_read->setText(QApplication::translate("MainWindow", "Read", 0, QApplication::UnicodeUTF8));
        pushButton_save->setText(QApplication::translate("MainWindow", "Save", 0, QApplication::UnicodeUTF8));
        pushButton_load->setText(QApplication::translate("MainWindow", "Load", 0, QApplication::UnicodeUTF8));
        checkBox_select_all->setText(QApplication::translate("MainWindow", "Select All", 0, QApplication::UnicodeUTF8));
        pushButton_write->setText(QApplication::translate("MainWindow", "Write", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("MainWindow", "Load/Save Configs", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("MainWindow", "Read/Write EEPROM", 0, QApplication::UnicodeUTF8));
        pushButton_clear->setText(QApplication::translate("MainWindow", "Clear", 0, QApplication::UnicodeUTF8));
        pushButton_read_all->setText(QApplication::translate("MainWindow", "Read All", 0, QApplication::UnicodeUTF8));
        textEdit->setHtml(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Sans'; font-size:10pt; font-weight:400; font-style:normal;\">\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:24pt; font-weight:600;\">MARVELL</span></p></body></html>", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
