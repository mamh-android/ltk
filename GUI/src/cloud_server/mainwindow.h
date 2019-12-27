#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtXml>
#include <qmap.h>
#include <QToolBox>
#include <QWidget>
#include <QTextEdit>
#include <QTreeWidget>
#include <QDesktopServices>
#include <QMessageBox>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QMenu>
#include <QWidgetAction>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include "dialog.h"
#include <QDockWidget>
#include <QComboBox>
#include <QtNetwork>
#include <QProgressBar>
#include <QFileDialog>
#include <QClipboard>
#include <QCompleter>
#include <QCheckBox>
//#include </home/wsun/git/wenjiu_git/ATD_testsuite/LTK/GUI/include/mysql/mysql.h>

namespace Ui {
class MainWindow;
}

struct staf_board_info{
        QString board_id,board_type,board_register_date,board_eco;
        QString lcd_resolution,lcd_screensize;
        QString ddr_type,ddr_size,emmc_type,emmc_size,dro,chip_type;
        QString rf_name,rf_type;
        QString connect_staf,connect_board;
        QString board_status,machine;
        QString allinfo;
        QString board_detail_string;
        QString chip_name;
        QString chip_stepping;
        QString connect_serial;
        QString connect_mcu;
        QString connect_usb;
        QString online;
        QString board;
        int task_num;

};


struct gui_msg_t{
        unsigned int magic;
        unsigned int op;
        char param[256];
};

class StringList_SSP :public QStringList
{
public:
    StringList_SSP(){};
    StringList_SSP(QStringList list);
    QString at_sp(int a);
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void xml_read();
    void test();
    void show_tree();
    void show_board();
    void init_windows();
    bool debug_mode;
    QMap<QString,QStringList> case_map;
    QMap<QString,QString> platform_map;
    QMap<QString,int> board_map;
    QMap<QString,QTreeWidgetItem*> task_map;
    QMap<QString,QTreeWidgetItem*> mod_map;
    QMap<QString,QTableWidgetItem*> staf_task_case_map_tab;
    QMap<QString,QTableWidgetItem*> staf_task_log_map_tab;
    QMap<QString,QTableWidget*> task_tab_map;
    QMap<QString,staf_board_info> staf_board_map;
    QMap<QString,QString> staf_task_map;
    QMap<QString,QProgressBar*> task_progress;
    QMap<QString,QString> image_map;
    QMap<QString,QString> task_board_entry;
    int task_tree_row;
    QProcess::ExitStatus waitProcess(const QString command,const int timeout);
    QProcess *Gcmd;
    QString Goutput;
    volatile bool Gstopped;
    void setGstopped(bool status){Gstopped = status;}
    bool getGstopped() {return Gstopped;}
    int GcostTimes;
    bool progress_result;
    QString tempString,warning_mesg;
    QString test_package_server_ip;
    bool progress_lock;
    bool use_data_base;
    bool set_success;
    QDate Date;
    QString platform,task_name,selected_item,selected_case,delay_setting,task_date;
    QString blf_modified,blf_modified_name,blf_modified_folder,blf_modified_path,blf_selected;
    QStringList blf_modified_name_list;
    QStringList user_group_list;
    QString update_image,image_platform,image_version,image_def,image_burning_blf,image_modify,special_image_ip,specail_image_path,replaced_image,ltk_version,ltk_branch,image_burn_config,ltk_config;
    QStringList board_list,case_list,task_list,task_list_all,case_list_loop;
    void check_tree();
    void check_board_list();
    void check_package_image();
    QTabWidget *toolBox;
    QTextEdit* start_config;
    QTreeWidget* task_tree;
    QTreeWidget* ltk_package_tree,*ltk_image_tree;
    QTreeWidgetItem* selected_tree_item,*selected_package_item;
    void start_eable(bool eable);
    QString ddr_type_lp_transform(QString ddr_ori_str);
    void show_package_tree();
    QTimer* update_result;
    QString staf_ip;
    QString board_detail_id;
    int pxa1908_sum,pxa1928_sum,pxa1936_sum;
    int order_col;
    int process_retry;
    void generate_tab();
    void create_tool_bar();
    void create_menu();
    void show_image_tree();
    int progress_return;
    QSqlDatabase dbMySQL;
    QString link_for_blf;
    QString ex_setting;
    QString finished_job_history;
    QCheckBox *real_time_log_dump,*stop_auto_list_board,*use_new_swdl,*stop_auto_update_job_status,*setup_ltk_checkbox,*skip_image_checkbox,*panic_detection_checkbox;
    QLineEdit *skip_image_lineedit;
    int filter_row_num;
    void create_filter_menu();
    QStringList filter_board_id_list,filter_platform_list,filter_stepping_list,filter_ddr_type_list,filter_chip_type_list;
    QStringList filter_task_list,filter_online_list,filter_board_list;
    QStringList filter_mcu_list,filter_serial_list,filter_usb_list;
    QStringList filter_user_list,filter_owner_list,filter_ip_list;
    QStringList filter_ddr_size_list,filter_dro_list;
    bool filter_table_enable;
    QComboBox *combobox_filter_board_id,*combobox_filter_platform,*combobox_filter_stepping,*combobox_filter_ddr_type,*combobox_filter_chip_type;
    QComboBox *combobox_filter_task,*combobox_filter_online,*combobox_filter_board;
    QComboBox *combobox_filter_mcu,*combobox_filter_serial,*combobox_filter_usb;
    QComboBox *combobox_filter_user,*combobox_filter_owner,*combobox_filter_ip;
    QComboBox *combobox_filter_ddr_size,*combobox_filter_dro;
    QIcon filter_icon,unfilter_icon,lock_icon;
    QUdpSocket *receiver;
    QByteArray datagram_sender;
    int board_list_reflesh_count;
    QString work_dir;
    QTextEdit* per_shell_setting;
    QCheckBox* per_set_pc;
    bool hide_unavailable_board;
    bool use_mysql_database;

    bool use_gui_delay;
    QString gui_delay_time_str;
    QDialog *ask_debug_config;
    QDialog *ask_delay_time;
    QDialog *ask_login_dialog;
    QPushButton *button_ok,*button_delay_ok,*button_login_ok;
    QLineEdit *lineedit_ip,*lineedit_delay_time,*lineedit_login_username,*lineedit_login_password;
    QComboBox *combobox_login_level;
    int login_level;
    QPushButton *open_file,*save_config,*clear_ex_setting;
    QWidget *widget_ex_setting;
    QGridLayout *layout_ex_setting;
    QString pre_set_link;
    QString pre_set_pc_config;
    QString sp_setting_string;
    bool per_setting_changed;
    bool stop_socket_connection;
    QClipboard *qt_clipboard;
    QString clipboard_str;
    bool delay_task;
    int delay_board_num;
    QMap<QString,QStringList> delay_task_case_id;
    QMap<QString,QStringList> delay_task_setting;
    QString add_delay_board,add_delay_task;
    QString login_name,login_password;
    QLineEdit *delay_board_num_lineedit;
    QComboBox *delay_board_type_combobox,*delay_chip_stepping_combobox,*delay_ddr_type_combobox,*delay_ddr_size_combobox,*delay_emmc_type_combobox,*delay_dro_combobox,*delay_lcd_resulation_combobox,*delay_chip_type_combobox,*delay_rf_name_combobox,*delay_rf_type_combobox,*delay_board_id_combobox,*delay_user_team_combobox;
private:
    Ui::MainWindow *ui;
    QAction* reflash;
    QAction* show_hide_board;
    QAction* enter_debug_mode;
    QAction* user_login;
    QAction* gui_delay_time;
    QAction* use_mysql;
    QAction* start;
    QAction* about;
    QAction* reload_blf_map;
    QAction* reload_ltk_map;
    //QAction* select_all_board;
    //QAction* unselect_all_board;
    QMenu *menu_bar;
    QMenuBar* top_menu;
    QToolBar *toolBar;
protected:
       void closeEvent(QCloseEvent *event);

public slots:
    void confirm_setting();
       void change_list();
       void readOutput_byte();
        void generate_config();
        void reset_config();
        void start_test();
        void clear_config();
        void update_status();
        void show_board_detail(int row,int col);
        void case_select_all(QTreeWidgetItem* before_select,QTreeWidgetItem* after_select);
        void tree_menu(QTreeWidgetItem* item_menu);
        void open_link();
        void delete_staf_task();
        void show_package();
        void check_pakcage_tree(QTreeWidgetItem* select_package,int select_col);
        void open_dialog_for_special_image(QTreeWidgetItem* select_image,int select_col);
        void image_dialog_open();
        void time_update();
        void sort_col(int col);
        void sort_table_order();
        void sort_table_desorder();
        void close_tab();
        void show_image_config();
        void show_version();
        void filter_item(QString filter_item_name);
        void show_hidden_table();
        void show_filter_menu();
        void readPendingDatagrams();
        void readPendingDatagrams_delay_key();
        void staf_reboot_board();
        void open_tab_log_link(QTableWidgetItem* link_item);
        void update_progress_bar(QString progress_task_name,QString progress_board_name,int progress_value);
        void tabCloseSlot(int tab_index);
        void register_gui();
        void add_ex_setting();
        void add_sp_setting();
        void show_hide_unavailable_board();
        void open_ex_setting_file();
        void save_ex_setting();
        void set_per_setting_changed(){per_setting_changed=true;}
        void create_delay_setting();
        void get_delay_setting();
        void hide_show_log_setting();
        void check_delay_task_for_blf();
        void add_delay_tree_tab();
        void choose_delay_tab(bool delay_chosed);
        void check_runcase_mode(bool runcase_mode_chosed);
        void copy_str_to_clipboard();
        void act_reload_blf_map();
        void act_reload_ltk_map();
        void act_enter_debug_mode();
        void save_debug_config();
        void act_gui_delay_time();
        void save_delay_time();
        void board_lock_menu(QTableWidgetItem* item_menu);
        void lock_board();
        void unlock_board();
        void check_mysql_use();
        void show_sort_tree_menu();
        void sort_tree_order();
        void sort_tree_desorder();
        void ttt();
        void cancel_job(QTreeWidgetItem* board);
        void cancel_task();
        void cancel_board();
        void select_board();
        void unselect_board();
        void add_pc_per_set_config();
        void gui_action_config();
        void act_user_login();
        void account_confirm();
        void account_function();
};

#endif // MAINWINDOW_H
