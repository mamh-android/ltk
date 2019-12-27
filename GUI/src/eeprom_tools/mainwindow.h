#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <stdlib.h>
#include <assert.h>
#include <QThread>
#include <QtCore>
#include <iostream>
#include <unistd.h>

namespace Ui {
class MainWindow;
}

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
    QString tempString;
    QString eeprom_write;
    QString eeprom_write_cmd;
    QString binary;
    bool progress_result;
public slots:
    void select_all();
    void reflash_config();
    void unselect_all();
    void save_config();
    void read_item();
    void read_item_ini();
    void load_config();
    void write_item();
    void readOutput_byte();
    void select_choose();
    void init_config();
    void clear();
    void read_all();
    void write_all();
    void set_pushbottom(bool set_status);
    void print_result(bool result);
    void merge_ddr_size_speed();
    void merge_ddr_type_lp();
    QString ddr_type_lp_transform(QString ddr_ori_str);
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
