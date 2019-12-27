#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QCloseEvent>
#include <stdlib.h>
#include <assert.h>
#include <QThread>
#include <QtCore>
#include <iostream>
#include <unistd.h>
#include <QtNetwork>
#include <QMessageBox>
#include <QGridLayout>
#include <QLineEdit>
#include <QCompleter>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMouseEvent>
#include <qmap.h>
#include <QComboBox>
#include <QMap>

namespace Ui {
class MainWindow;
}

class mylineedit : public QLineEdit
{
    Q_OBJECT
public:
    explicit mylineedit(QWidget *parent = 0);
protected:
    //重写mousePressEvent事件
    virtual void mousePressEvent(QMouseEvent *event);
signals:
    //自定义clicked()信号,在mousePressEvent事件发生时触发
    void clicked();
public slots:

};



struct client_board_info{
    bool shared;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QProcess::ExitStatus waitProcess(const QString command,const int timeout);
    QProcess *Gcmd;
    QString Goutput;
    volatile bool Gstopped;
    void setGstopped(bool status){Gstopped = status;}
    bool getGstopped() {return Gstopped;}
    int GcostTimes;
    bool progress_result;
    QString tempString;
    QString board_id_action;
   QSystemTrayIcon *trayIcon;
   bool closed_to_tool_bar;
   bool ubuntu_13;
   bool config_setted;
   //QSettings cloud_client_setting;
   bool stop_connection;
   void lock_usage();
   void lock_button(bool lock_flag1);
   bool lock_flag,warning_flag;
   QIcon shared_icon;
   QIcon un_shared_icon;
   QSize icon_size;
   int select_history;
   bool progress_lock;
   bool show_burning_widget;
   QUdpSocket *receiver;
   QByteArray datagram_sender;
   QString changed_board,job_status;
   QMap<QString,QString> job_status_map;
   int warning_limit_num;
   bool debug_mode;
   void progress_wait_protect();
   void lock_free() {progress_lock=false;};
   bool get_lock_status() {return progress_lock;};
   bool board_connection_ok;
   QDialog* ask_user_name;
   QString register_name,register_team,register_level;
   QPushButton* button_ok;
   //QLineEdit* register_name_line;
   mylineedit* register_name_line;
   mylineedit* register_team_line;
   bool register_name_saved;
   bool is_the_latest_version;
   void ask_for_user_name();
   QString ddr_type_lp_transform(QString ddr_ori_str);
   QStringList board_list;
   QString adb_connect_status,serial_connect_stauts,rcc_connect_status;
   QString file_text;
   QMap<QString,QString> daily_image_map;
   QMap<QString,client_board_info> board_info;
   QString serial_node;
   QComboBox *commbox_share_level;
   QTimer* update_result;
public slots:
   void close_to_icon();
   void onSystemTrayIconClicked(QSystemTrayIcon::ActivationReason reason);
   void readOutput_byte();
   void list_board();
   void share_board();
   void unshare_board();
   void show_board_detail(QString board_id);
   void reboot();
   void emmd_dump();
   void download();
   void power_off();
   void power_on();
   void power_off_soft();
   void onkey();
   void fw_update();
   void readPendingDatagrams();
   void save_register_name();
   void reg_name_select_all();
   void show_burning_setting();
   void load_blf_map();
   void show_blf_map();
   void version_check();
   void close_gui();
   void time_update();
//private slots:
      void show_map(QString map_key);
protected:
       void closeEvent(QCloseEvent *event);

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon *trayicon;
    QMenu *trayiconMenu;


};

#endif // MAINWINDOW_H

struct gui_msg_t{
        unsigned int magic;
        unsigned int op;
        char param[256];
};





