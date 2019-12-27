#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <stdlib.h>
#include <QDir>
#include <QDebug>
#include <QtCore>
#include <iostream>
#include <QtGui/QApplication>
#include <unistd.h>
#include <qmessagebox.h>
#include <QCompleter>
#include <QtSql>
#include <QLabel>
#include <QMovie>
#include <QDate>
#include <QTime>
#include <QtXml>
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init_config();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init_config()
{



    reflash_config();
    select_all();
#ifdef Q_OS_WIN
    binary="rcc_eeprom.exe";
#else
    binary="./rcc_eeprom";
#endif
    ui->lineEdit_board_register_date->setDisabled(true);
    QDate date=QDate::currentDate();
    QString strDate = date.toString("yy-MM-dd");
    QTime time=QTime::currentTime();
    strDate = strDate+"-"+time.toString("hh:mm");
    ui->lineEdit_board_register_date->setText(strDate);
/*
#ifdef Q_OS_WIN
    QMovie *movie =new QMovie("logo.jpg");
#else
    QMovie *movie =new QMovie("./logo.jpg");
#endif
    ui->label_logo->setMovie(movie);
    movie->start();
    ui->label_logo->show();
*/
   // ui->label_logo->setPixmap(QPixmap("Z:/eprom/logo.jpg"));

    ui->checkBox_select_all->setChecked(true);
    QSettings settings_add("./Addition_list.ini",QSettings::IniFormat);
    QStringList emmc_ddr_tpye_add=settings_add.value("ddr_type").toString().split(";");
    QStringList lcd_resolution_add=settings_add.value("lcd_resolution").toString().split(";");
    QStringList lcd_screensize_add=settings_add.value("lcd_screensize").toString().split(";");
    //QStringList emmc_size_add=settings_add.value("emmc_size").toString().split(";");
    //QStringList ddr_size_add=settings_add.value("ddr_size").toString().split(";");
    QStringList chip_name_add=settings_add.value("chip_name").toString().split(";");
    QStringList chip_setpping_add=settings_add.value("chip_setpping").toString().split(";");
    //QStringList user_team_add=settings_add.value("user_team").toString().split(";");
    QStringList board_user_add=settings_add.value("board_owner").toString().split(";");



    QStringList lcd_resolution;
    lcd_resolution << "720P" << "1080P";
    if ( ! settings_add.value("lcd_resolution").toString().isEmpty() )
    { lcd_resolution << lcd_resolution_add; }
    ui->comboBox_lcd_resolution->addItems(lcd_resolution);

    QStringList lcd_screensize;
    lcd_screensize << "4.3" << "5.0";
    if ( ! settings_add.value("lcd_screensize").toString().isEmpty() )
    { lcd_screensize << lcd_screensize_add; }
    ui->comboBox_lcd_screensize->addItems(lcd_screensize);

    QStringList emmc_ddr_type;
    QStringList emmc_type;
    emmc_type << "Samsung" << "Micron" << "Hynix" << "Elpida" << "Sandisk"<<"Toshiba";
    if ( ! settings_add.value("ddr_type").toString().isEmpty() )
    { emmc_ddr_type << emmc_ddr_tpye_add; }
    emmc_ddr_type << "EPA@P" << "EPA@D" ;
    ui->comboBox_ddr_type->addItems(emmc_ddr_type);
    ui->comboBox_ddr_type->setToolTip("Samsung => SSG\nToshiba  => THA\nHynix     =>  HYX\nSandisk =>  SDK\nMicron =>   MCN\nELPIDA =>  EPA\n@P means for PoP\n@D means for DIS");
    ui->comboBox_emmc_type->addItems(emmc_type);

    QStringList ddr_lp_type;
    ddr_lp_type<<"LPDDR3"<<"LPDDR2"<<"DDR3"<<"DDR2";
    ui->comboBox_ddr_lp_type->addItems(ddr_lp_type);

    QStringList emmc_size;
    emmc_size << "4GB" << "8GB" << "16GB" << "32GB";
    ui->comboBox_emmc_size->addItems(emmc_size);

    QStringList ddr_size;
    ddr_size << "1G" << "2G" << "512M";
    ui->comboBox_ddr_size->addItems(ddr_size);

    QStringList ddr_freq;
    ddr_freq << "800" << "667" << "533" << "400" << "312";
    ui->comboBox_ddr_freq->addItems(ddr_freq);

    QStringList chip_name;
    chip_name << "PXA988" << "PXA1088" << "PXA1L88" << "PXA1U88" << "PXA1928" << "PXA1986";
    if ( ! settings_add.value("chip_name").toString().isEmpty() )
    { chip_name << chip_name_add; }
    ui->comboBox_chip_name->addItems(chip_name);

    QStringList chip_setpping;
    chip_setpping << "A0" << "A1" << "B0" << "B1"<<"Z1"<<"Z2";
    if ( ! settings_add.value("chip_setpping").toString().isEmpty() )
    { chip_setpping << chip_setpping_add; }
    ui->comboBox_chip_stepping->addItems(chip_setpping);

    QStringList user_team;
    user_team << "APSE-PV" << "APSE-OSE" << "APSE-SE1" << "APSE-SE2" << "APSE-MPE";
    //ui->comboBox_user_team->addItems(user_team);






    QStringList wordList;
    //ui->comboBox_current_user->addItem("");

    #ifdef Q_OS_WIN
    QFile file1("./user_list.txt");
     if(!file1.open(QIODevice::ReadOnly | QIODevice::Text)) {
         qDebug()<<"Can't open the file!"<<endl;
     }
     while(!file1.atEnd()) {
         QByteArray line = file1.readLine();
         QString str(line);
         str= str.replace("\n","");
         wordList << str;
    }
    #else
    QSqlDatabase dbMySQL=QSqlDatabase::addDatabase("QMYSQL");
    dbMySQL.setHostName("10.38.34.91");
    dbMySQL.setDatabaseName("ltk");
    dbMySQL.setUserName("hhuan10");
    dbMySQL.setPassword("123456");
    if (!dbMySQL.open())
        qDebug()<< "link fail";
    else
         qDebug()<< "link OK";
    QSqlQuery query;
    query.exec("select * from Outlook_Address_Book;");
    while (query.next())
    {
         wordList << query.value(0).toString();
    }
    #endif
    wordList << board_user_add;




    QCompleter *completer = new QCompleter(wordList, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    ui->lineEdit_current_user->setCompleter(completer);

    QCompleter *completer_user_team = new QCompleter(user_team, this);
    completer_user_team->setCaseSensitivity(Qt::CaseInsensitive);
    completer_user_team->setCompletionMode(QCompleter::PopupCompletion);
    ui->lineEdit_user_team->setCompleter(completer_user_team);





    QFile file("eeprom_def.xml");
    QDomDocument doc;
    QString errStr;
    int errLine, errCol;
    if(!doc.setContent(&file, false, &errStr, &errLine, &errCol))
    {
        qDebug()<<"Error： "+errStr;
    }
    QDomElement root=doc.documentElement();
     qDebug()<<root.nodeName();
    QDomNode child = root.firstChild();
    while (!child.isNull())
    {
        //qDebug()<<child.toElement().tagName()<<":"<<child.toElement().text()<<child.toElement().attribute("size");
        QString string_size,item_name;
        item_name=child.toElement().tagName();
        string_size=child.toElement().attribute("size");
        int write_size;
        write_size=string_size.toInt();
        qDebug()<<item_name<<write_size<<item_name.indexOf("board_type")<<write_size;
        if ( item_name.indexOf("board_type") >= 0 )
        {
            ui->lineEdit_board_type->setMaxLength(write_size);
        }
        if ( item_name.indexOf("board_id") >= 0 )
        {
            ui->lineEdit_board_id->setMaxLength(write_size);
        }
        if ( item_name.indexOf("chip_name") >= 0 )
        {
            ui->lineEdit_chip_name->setMaxLength(write_size);
        }
        if ( item_name.indexOf("chip_stepping") >= 0 )
        {
            ui->lineEdit_chip_stepping->setMaxLength(write_size);
        }
        if ( item_name.indexOf("board_register_date") >= 0 )
        {
            ui->lineEdit_board_register_date->setMaxLength(write_size);
        }
        if ( item_name.indexOf("board_status") >= 0 )
        {
            ui->lineEdit_board_status->setMaxLength(write_size);
        }
        if ( item_name.indexOf("board_eco") >= 0 )
        {
            ui->lineEdit_board_eco->setMaxLength(write_size);
        }
        if ( item_name.indexOf("user_team") >= 0 )
        {
            ui->lineEdit_user_team->setMaxLength(write_size);
        }
        if ( item_name.indexOf("current_user") >= 0 )
        {
            ui->lineEdit_current_user->setMaxLength(write_size);
        }
        if ( item_name.indexOf("lcd_resolution") >= 0 )
        {
            ui->lineEdit_lcd_resolution->setMaxLength(write_size);
        }
        if ( item_name.indexOf("lcd_screensize") >= 0 )
        {
            ui->lineEdit_lcd_screensize->setMaxLength(write_size);
        }
        if ( item_name.indexOf("ddr_type") >= 0 )
        {
            ui->lineEdit_ddr_type->setMaxLength(24);
        }
        if ( item_name.indexOf("ddr_size") >= 0 )
        {
            ui->lineEdit_ddr_size->setMaxLength(write_size);
        }
        if ( item_name.indexOf("emmc_type") >= 0 )
        {
            ui->lineEdit_emmc_type->setMaxLength(write_size);
        }
        if ( item_name.indexOf("emmc_size") >= 0 )
        {
            ui->lineEdit_emmc_size->setMaxLength(write_size);
        }
        if ( item_name.indexOf("rf_name") >= 0 )
        {
            ui->lineEdit_rf_name->setMaxLength(write_size);
        }
        if ( item_name.indexOf("rf_type") >= 0 )
        {
            ui->lineEdit_rf_type->setMaxLength(write_size);
        }
        child=child.nextSibling();
    }

}

QString MainWindow::ddr_type_lp_transform(QString ddr_ori_str)
{
    QString type_s,mount_s,lp_s;
    QString type_t,mount_t,lp_t;
    QString ddr_ori_str_transformed;
    QMap<QString,QString> ddr_type_map;
    ddr_type_map["L2"]="LPDDR2";
    ddr_type_map["L3"]="LPDDR3";
    ddr_type_map["D2"]="DDR2";
    ddr_type_map["D3"]="DDR3";
    ddr_type_map["LPDDR2"]="L2";
    ddr_type_map["LPDDR3"]="L3";
    ddr_type_map["DDR3"]="D3";
    ddr_type_map["DDR2"]="D2";

    ddr_type_map["SSG"]="Samsung";
    ddr_type_map["HYX"]="Hynix";
    ddr_type_map["THA"]="Toshiba";
    ddr_type_map["SDK"]="Sandisk";
    ddr_type_map["MCN"]="Micron";
    ddr_type_map["EPA"]="ELPIDA";
    ddr_type_map["Samsung"]="SSG";
    ddr_type_map["Hynix"]="HYX";
    ddr_type_map["Toshiba"]="THA";
    ddr_type_map["Sandisk"]="SDK";
    ddr_type_map["Micron"]="MCN";
    ddr_type_map["ELPIDA"]="EPA";
    /*
    ddr_type_map["D"]="DIS";
    ddr_type_map["P"]="POP";
    */

    if (ddr_ori_str.size()==7)
    {
        type_s=ddr_ori_str.mid(0,3);
        type_t=ddr_type_map[type_s];
        ddr_ori_str_transformed=ddr_ori_str.mid(0,5)+"@"+lp_t;
        //lp_s=ddr_ori_str.mid(5,2);
        //lp_t=ddr_type_map[lp_s];
        //ddr_ori_str_transformed=type_t+ddr_ori_str.mid(3,2)+"@"+lp_t;
    }
    else
    {
        QStringList ddr_type_t=ddr_ori_str.split("@");
        if (ddr_type_t.count() == 3)
        {

            lp_t=ddr_type_t.at(2);
            lp_s=ddr_type_map[lp_t];
            ddr_ori_str_transformed=ddr_type_t.at(0)+"@"+ddr_type_t.at(1)+lp_s;
            //type_t=ddr_type_t.at(0);
            //type_s=ddr_type_map[type_t];
            //ddr_ori_str_transformed=type_s+"@"+ddr_type_t.at(1)+lp_s;
        }else
        {
            ddr_ori_str_transformed="Unknow";
        }

    }
    qDebug()<<"DDR type transform"<<ddr_ori_str_transformed;
    return ddr_ori_str_transformed;

}

void MainWindow::merge_ddr_size_speed()
{
    QString size_speed=ui->comboBox_ddr_size->currentText()+"@"+ui->comboBox_ddr_freq->currentText();
    ui->lineEdit_ddr_size->setText(size_speed);

}

void MainWindow::merge_ddr_type_lp()
{
    QString ddr_type_lp=ui->comboBox_ddr_type->currentText()+"@"+ui->comboBox_ddr_lp_type->currentText();
    //ddr_type_lp=ddr_type_lp_transform(ddr_type_lp);
    ui->lineEdit_ddr_type->setText(ddr_type_lp);
}


void MainWindow::reflash_config()
{
    ui->comboBox_config->clear();
    QDir dir = QDir::currentPath()+"/config";
    qDebug()<<dir;
    dir.setFilter( QDir::Files );
    QFileInfoList list = dir.entryInfoList();
    int i=0;
    for (i=0;i<list.size();i++)
    {
    QFileInfo fileInfo = list.at(i);
    qDebug()<<fileInfo.fileName();
    QString file_name=fileInfo.fileName();
    //qDebug()<<QDir::currentPath();
    ui->comboBox_config->addItem(fileInfo.fileName());
    }
}

void MainWindow::save_config()
{
    QString eeprom_save,eeprom_save_cmd,save_file;

    save_file=ui->comboBox_config->currentText();

    QSettings settings("./config/"+save_file,QSettings::IniFormat);
    settings.setValue("board_type",ui->lineEdit_board_type->text());
    settings.setValue("board_id",ui->lineEdit_board_id->text());
    settings.setValue("chip_name",ui->lineEdit_chip_name->text());
    settings.setValue("chip_stepping",ui->lineEdit_chip_stepping->text());
    settings.setValue("board_register_date",ui->lineEdit_board_register_date->text());
    settings.setValue("board_status",ui->lineEdit_board_status->text());
    settings.setValue("board_eco",ui->lineEdit_board_eco->text());
    settings.setValue("user_team",ui->lineEdit_user_team->text());
    settings.setValue("current_user",ui->lineEdit_current_user->text());
    settings.setValue("lcd_resolution",ui->lineEdit_lcd_resolution->text());
    //settings.setValue("lcd_resolution",ui->comboBox_lcd_resolution->currentText());
    settings.setValue("lcd_screensize",ui->lineEdit_lcd_screensize->text());
    settings.setValue("ddr_type",ui->lineEdit_ddr_type->text());
    settings.setValue("ddr_size",ui->lineEdit_ddr_size->text());
    settings.setValue("emmc_type",ui->lineEdit_emmc_type->text());
    settings.setValue("emmc_size",ui->lineEdit_emmc_size->text());
    settings.setValue("rf_name",ui->lineEdit_rf_name->text());
    settings.setValue("rf_type",ui->lineEdit_rf_type->text());
    settings.sync();

    //sleep(1);
    reflash_config();
    ui->statusBar->showMessage("Saved !!!");
}

void MainWindow::load_config()
{
    QString load_file;
    load_file="./config/"+ui->comboBox_config->currentText();
    QSettings settings(load_file,QSettings::IniFormat);
    settings.sync();

    ui->lineEdit_board_type->setText(settings.value("board_type").toString());
    ui->lineEdit_board_id->setText(settings.value("board_id").toString());
    ui->lineEdit_chip_name->setText(settings.value("chip_name").toString());
    ui->lineEdit_chip_stepping->setText(settings.value("chip_stepping").toString());
    ui->lineEdit_board_register_date->setText(settings.value("board_register_date").toString());
    ui->lineEdit_board_status->setText(settings.value("board_status").toString());
    ui->lineEdit_board_eco->setText(settings.value("board_eco").toString());
    ui->lineEdit_user_team->setText(settings.value("user_team").toString());
    ui->lineEdit_current_user->setText(settings.value("current_user").toString());
    ui->lineEdit_lcd_resolution->setText(settings.value("lcd_resolution").toString());
    ui->lineEdit_lcd_screensize->setText(settings.value("lcd_screensize").toString());
    ui->lineEdit_ddr_type->setText(settings.value("ddr_type").toString());
    ui->lineEdit_ddr_size->setText(settings.value("ddr_size").toString());
    ui->lineEdit_emmc_type->setText(settings.value("emmc_type").toString());
    ui->lineEdit_emmc_size->setText(settings.value("emmc_size").toString());
    ui->lineEdit_rf_name->setText(settings.value("rf_name").toString());
    ui->lineEdit_rf_type->setText(settings.value("rf_type").toString());

}


void MainWindow::select_choose()
{
    if ( ui->checkBox_select_all->isChecked() )
    {
        select_all();
    }
    else
    {
        unselect_all();
    }
}

void MainWindow::select_all()
{
    ui->checkBox_board_type->setChecked(true);
    ui->checkBox_board_id->setChecked(true);
    ui->checkBox_chip_name->setChecked(true);
    ui->checkBox_chip_stepping->setChecked(true);
    ui->checkBox_board_register_date->setChecked(true);
    ui->checkBox_board_status->setChecked(true);
    ui->checkBox_user_team->setChecked(true);
    ui->checkBox_current_user->setChecked(true);
    ui->checkBox_board_eco->setChecked(true);
    ui->checkBox_lcd_resolution->setChecked(true);
    ui->checkBox_lcd_screensize->setChecked(true);
    ui->checkBox_ddr_type->setChecked(true);
    ui->checkBox_ddr_size->setChecked(true);
    ui->checkBox_emmc_type->setChecked(true);
    ui->checkBox_emmc_size->setChecked(true);
    ui->checkBox_rf_name->setChecked(true);
    ui->checkBox_rf_type->setChecked(true);
}


void MainWindow::unselect_all()
{
    ui->checkBox_board_type->setChecked(false);
    ui->checkBox_board_id->setChecked(false);
    ui->checkBox_chip_name->setChecked(false);
    ui->checkBox_chip_stepping->setChecked(false);
    ui->checkBox_board_register_date->setChecked(false);
    ui->checkBox_board_status->setChecked(false);
    ui->checkBox_user_team->setChecked(false);
    ui->checkBox_current_user->setChecked(false);
    ui->checkBox_board_eco->setChecked(false);
    ui->checkBox_lcd_resolution->setChecked(false);
    ui->checkBox_lcd_screensize->setChecked(false);
    ui->checkBox_ddr_type->setChecked(false);
    ui->checkBox_ddr_size->setChecked(false);
    ui->checkBox_emmc_type->setChecked(false);
    ui->checkBox_emmc_size->setChecked(false);
    ui->checkBox_rf_name->setChecked(false);
    ui->checkBox_rf_type->setChecked(false);
}

void MainWindow::clear()
{
    ui->lineEdit_board_type->clear();
    ui->lineEdit_board_id->clear();
    ui->lineEdit_chip_name->clear();
    ui->lineEdit_chip_stepping->clear();
    ui->lineEdit_board_register_date->clear();
    ui->lineEdit_board_status->clear();
    ui->lineEdit_user_team->clear();
    ui->lineEdit_current_user->clear();
    ui->lineEdit_board_eco->clear();
    ui->lineEdit_lcd_resolution->clear();
    ui->lineEdit_lcd_screensize->clear();
    ui->lineEdit_ddr_type->clear();
    ui->lineEdit_ddr_size->clear();
    ui->lineEdit_emmc_type->clear();
    ui->lineEdit_emmc_size->clear();
    ui->lineEdit_rf_name->clear();
    ui->lineEdit_rf_type->clear();
    ui->textEdit_log->clear();
    ui->textEdit->setHtml(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
                                                  "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
                                                  "p, li { white-space: pre-wrap; }\n"
                                                  "</style></head><body style=\" font-family:'Sans'; font-size:10pt; font-weight:400; font-style:normal;\">\n"
                                                  "<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:24pt; font-weight:600;\">MARVELL</span></p></body></html>", 0, QApplication::UnicodeUTF8));
}

void MainWindow::print_result(bool result)
{
    if (result)
    {
    ui->textEdit->setHtml(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
                                                  "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
                                                  "p, li { white-space: pre-wrap; }\n"
                                                  "</style></head><body style=\" font-family:'Sans'; font-size:10pt; font-weight:400; font-style:normal;\">\n"
                                                  "<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; color:#00ff00; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:24pt; font-weight:600;\">PASS</span></p></body></html>", 0, QApplication::UnicodeUTF8));
    }
    else
    {
    ui->textEdit->setHtml(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
                                                  "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
                                                  "p, li { white-space: pre-wrap; }\n"
                                                  "</style></head><body style=\" font-family:'Sans'; font-size:10pt; font-weight:400; font-style:normal;\">\n"
                                                  "<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; color:#aa0000; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:24pt; font-weight:600;\">FAIL</span></p></body></html>", 0, QApplication::UnicodeUTF8));
    }
}



void MainWindow::read_item()
{

    Gcmd = new QProcess();
    qDebug()<<__LINE__<<__FILE__<<Gcmd->isSequential();
    connect(Gcmd,SIGNAL(readyRead()),this,SLOT(readOutput_byte()));

    if (ui->checkBox_board_type->isChecked())
    {
        waitProcess(binary+" 0 board_type",10);
        ui->lineEdit_board_type->setText(tempString);
    }
    if (ui->checkBox_board_id->isChecked())
    {
        waitProcess(binary+" 0 board_id",10);
        ui->lineEdit_board_id->setText(tempString);
    }
    if (ui->checkBox_chip_name->isChecked())
    {
        waitProcess(binary+" 0 chip_name",10);
        ui->lineEdit_chip_name->setText(tempString);
    }
    if (ui->checkBox_chip_stepping->isChecked())
    {
        waitProcess(binary+" 0 chip_stepping",10);
        ui->lineEdit_chip_stepping->setText(tempString);
    }
    if (ui->checkBox_board_register_date->isChecked())
    {
        waitProcess(binary+" 0 board_register_date",10);
        ui->lineEdit_board_register_date->setText(tempString);
    }
    if (ui->checkBox_board_status->isChecked())
    {
        waitProcess(binary+" 0 board_status",10);
        ui->lineEdit_board_status->setText(tempString);
    }
    if (ui->checkBox_user_team->isChecked())
    {
        waitProcess(binary+" 0 user_team",10);
        ui->lineEdit_user_team->setText(tempString);
    }
    if (ui->checkBox_current_user->isChecked())
    {
        waitProcess(binary+" 0 current_user",10);
        ui->lineEdit_current_user->setText(tempString);
    }
    if (ui->checkBox_board_eco->isChecked())
    {
        waitProcess(binary+" 0 board_eco",10);
        ui->lineEdit_board_eco->setText(tempString);
    }
    if (ui->checkBox_lcd_resolution->isChecked())
    {
        waitProcess(binary+" 0 lcd_resolution",10);
        ui->lineEdit_lcd_resolution->setText(tempString);
    }
    if (ui->checkBox_lcd_screensize->isChecked())
    {
        waitProcess(binary+" 0 lcd_screensize",10);
        ui->lineEdit_lcd_screensize->setText(tempString);
    }
    if (ui->checkBox_ddr_type->isChecked())
    {
        waitProcess(binary+" 0 ddr_type",10);
        ui->lineEdit_ddr_type->setText(tempString);
    }
    if (ui->checkBox_ddr_size->isChecked())
    {
        waitProcess(binary+" 0 ddr_size",10);
        ui->lineEdit_ddr_size->setText(tempString);
    }
    if (ui->checkBox_emmc_type->isChecked())
    {
        waitProcess(binary+" 0 emmc_type",10);
        ui->lineEdit_emmc_type->setText(tempString);
    }
    if (ui->checkBox_emmc_size->isChecked())
    {
        waitProcess(binary+" 0 emmc_size",10);
        ui->lineEdit_emmc_size->setText(tempString);
    }
    if (ui->checkBox_rf_name->isChecked())
    {
        waitProcess(binary+" 0 rf_name",10);
        ui->lineEdit_rf_name->setText(tempString);
    }
    if (ui->checkBox_rf_type->isChecked())
    {
        waitProcess(binary+" 0 rf_type",10);
        ui->lineEdit_rf_type->setText(tempString);
    }
}

void MainWindow::write_item()
{
    Gcmd = new QProcess();
    qDebug()<<__LINE__<<__FILE__<<Gcmd->isSequential();
    connect(Gcmd,SIGNAL(readyRead()),this,SLOT(readOutput_byte()));
    if (ui->checkBox_board_type->isChecked())
    {
        eeprom_write=ui->lineEdit_board_type->text();
        eeprom_write_cmd=binary+" 1 board_type \""+eeprom_write+"\"";
        ui->statusBar->showMessage(eeprom_write_cmd);
        waitProcess(eeprom_write_cmd,10);
        if ( ! progress_result )
            ui->lineEdit_board_type->setText("Write error");
    }
    if (ui->checkBox_board_id->isChecked())
    {
        eeprom_write=ui->lineEdit_board_id->text();
        eeprom_write_cmd=binary+" 1 board_id \""+eeprom_write+"\"";
        ui->statusBar->showMessage(eeprom_write_cmd);
        waitProcess(eeprom_write_cmd,10);
        if ( ! progress_result )
            ui->lineEdit_board_id->setText("Write error");
    }
    if (ui->checkBox_chip_name->isChecked())
    {
        eeprom_write=ui->lineEdit_chip_name->text();
        eeprom_write_cmd=binary+" 1 chip_name \""+eeprom_write+"\"";
        ui->statusBar->showMessage(eeprom_write_cmd);
        waitProcess(eeprom_write_cmd,10);
        if ( ! progress_result )
            ui->lineEdit_chip_name->setText("Write error");
    }
    if (ui->checkBox_chip_stepping->isChecked())
    {
        eeprom_write=ui->lineEdit_chip_stepping->text();
        eeprom_write_cmd=binary+" 1 chip_stepping \""+eeprom_write+"\"";
        ui->statusBar->showMessage(eeprom_write_cmd);
        waitProcess(eeprom_write_cmd,10);
        if ( ! progress_result )
            ui->lineEdit_chip_stepping->setText("Write error");
    }
    if (ui->checkBox_board_register_date->isChecked())
    {
        QDate date=QDate::currentDate();
        QString strDate = date.toString("yyyy-MM-dd");
        QTime time=QTime::currentTime();
        strDate = strDate+" "+time.toString("hh:mm");
        ui->lineEdit_board_register_date->setText(strDate);
        eeprom_write=ui->lineEdit_board_register_date->text();
        eeprom_write_cmd=binary+" 1 board_register_date \""+eeprom_write+"\"";
        ui->statusBar->showMessage(eeprom_write_cmd);
        waitProcess(eeprom_write_cmd,10);
        if ( ! progress_result )
            ui->lineEdit_board_register_date->setText("Write error");
    }
    if (ui->checkBox_board_status->isChecked())
    {
        eeprom_write=ui->lineEdit_board_status->text();
        eeprom_write_cmd=binary+" 1 board_status \""+eeprom_write+"\"";
        ui->statusBar->showMessage(eeprom_write_cmd);
        waitProcess(eeprom_write_cmd,10);
        if ( ! progress_result )
            ui->lineEdit_board_status->setText("Write error");
    }
    if (ui->checkBox_user_team->isChecked())
    {
        eeprom_write=ui->lineEdit_user_team->text();
        eeprom_write_cmd=binary+" 1 user_team \""+eeprom_write+"\"";
        ui->statusBar->showMessage(eeprom_write_cmd);
        waitProcess(eeprom_write_cmd,10);
        if ( ! progress_result )
            ui->lineEdit_user_team->setText("Write error");
    }
    if (ui->checkBox_current_user->isChecked())
    {
        eeprom_write=ui->lineEdit_current_user->text();
        eeprom_write_cmd=binary+" 1 current_user \""+eeprom_write+"\"";
        ui->statusBar->showMessage(eeprom_write_cmd);
        waitProcess(eeprom_write_cmd,10);
        if ( ! progress_result )
            ui->lineEdit_current_user->setText("Write error");
    }
    if (ui->checkBox_board_eco->isChecked())
    {
        eeprom_write=ui->lineEdit_board_eco->text();
        eeprom_write_cmd=binary+" 1 board_eco \""+eeprom_write+"\"";
        ui->statusBar->showMessage(eeprom_write_cmd);
        waitProcess(eeprom_write_cmd,10);
        if ( ! progress_result )
            ui->lineEdit_board_eco->setText("Write error");
    }
    if (ui->checkBox_lcd_resolution->isChecked())
    {
        eeprom_write=ui->lineEdit_lcd_resolution->text();
        eeprom_write_cmd=binary+" 1 lcd_resolution \""+eeprom_write+"\"";
        ui->statusBar->showMessage(eeprom_write_cmd);
        waitProcess(eeprom_write_cmd,10);
        if ( ! progress_result )
            ui->lineEdit_lcd_resolution->setText("Write error");
    }
    if (ui->checkBox_lcd_screensize->isChecked())
    {
        eeprom_write=ui->lineEdit_lcd_screensize->text();
        eeprom_write_cmd=binary+" 1 lcd_screensize \""+eeprom_write+"\"";
        ui->statusBar->showMessage(eeprom_write_cmd);
        waitProcess(eeprom_write_cmd,10);
        if ( ! progress_result )
            ui->lineEdit_lcd_screensize->setText("Write error");
    }
    if (ui->checkBox_ddr_type->isChecked())
    {
        eeprom_write=ui->lineEdit_ddr_type->text();
        eeprom_write_cmd=binary+" 1 ddr_type \""+eeprom_write+"\"";
        ui->statusBar->showMessage(eeprom_write_cmd);
        waitProcess(eeprom_write_cmd,10);
        if ( ! progress_result )
            ui->lineEdit_ddr_type->setText("Write error");
    }
    if (ui->checkBox_ddr_size->isChecked())
    {
        eeprom_write=ui->lineEdit_ddr_size->text();
        eeprom_write_cmd=binary+" 1 ddr_size \""+eeprom_write+"\"";
        ui->statusBar->showMessage(eeprom_write_cmd);
        waitProcess(eeprom_write_cmd,10);
        if ( ! progress_result )
            ui->lineEdit_ddr_size->setText("Write error");
    }
    if (ui->checkBox_emmc_type->isChecked())
    {
        eeprom_write=ui->lineEdit_emmc_type->text();
        eeprom_write_cmd=binary+" 1 emmc_type \""+eeprom_write+"\"";
        ui->statusBar->showMessage(eeprom_write_cmd);
        waitProcess(eeprom_write_cmd,10);
        if ( ! progress_result )
            ui->lineEdit_emmc_type->setText("Write error");
    }
    if (ui->checkBox_emmc_size->isChecked())
    {
        eeprom_write=ui->lineEdit_emmc_size->text();
        eeprom_write_cmd=binary+" 1 emmc_size \""+eeprom_write+"\"";
        ui->statusBar->showMessage(eeprom_write_cmd);
        waitProcess(eeprom_write_cmd,10);
        if ( ! progress_result )
            ui->lineEdit_emmc_size->setText("Write error");
    }
    if (ui->checkBox_rf_name->isChecked())
    {
        eeprom_write=ui->lineEdit_rf_name->text();
        eeprom_write_cmd=binary+" 1 rf_name \""+eeprom_write+"\"";
        ui->statusBar->showMessage(eeprom_write_cmd);
        waitProcess(eeprom_write_cmd,10);
        if ( ! progress_result )
            ui->lineEdit_rf_name->setText("Write error");
    }
    if (ui->checkBox_rf_type->isChecked())
    {
        eeprom_write=ui->lineEdit_rf_type->text();
        eeprom_write_cmd=binary+" 1 write rf_type \""+eeprom_write+"\"";
        ui->statusBar->showMessage(eeprom_write_cmd);
        waitProcess(eeprom_write_cmd,10);
        if ( ! progress_result )
            ui->lineEdit_rf_type->setText("Write error");
    }
    ui->statusBar->showMessage("WRITE FINISH !!!");

}

void MainWindow::set_pushbottom(bool set_status)
{
    ui->pushButton_clear->setEnabled(set_status);
    ui->pushButton_load->setEnabled(set_status);
    ui->pushButton_read->setEnabled(set_status);
    ui->pushButton_read_all->setEnabled(set_status);
    ui->pushButton_save->setEnabled(set_status);
    ui->pushButton_write->setEnabled(set_status);

}


void MainWindow::read_item_ini()
{
    set_pushbottom(false);
    //ui->pushButton_read->setText("In Progress");
    QFile::remove(".lastread.ini");
    Gcmd = new QProcess();
    qDebug()<<__LINE__<<__FILE__<<Gcmd->isSequential();
    connect(Gcmd,SIGNAL(readyRead()),this,SLOT(readOutput_byte()));
    waitProcess(binary+" 0 board_type board_id chip_name chip_stepping board_register_date emmc_size board_eco board_status user_team current_user lcd_resolution lcd_screensize ddr_type ddr_size emmc_type rf_name rf_type",10);
    ui->textEdit_log->setText(tempString);
    QString load_file;
    load_file=".lastread.ini";

    QSettings settings(load_file,QSettings::IniFormat);
    settings.sync();

    if (ui->checkBox_board_type->isChecked())
    {
        ui->lineEdit_board_type->setText(settings.value("board_type").toString());
    }
    if (ui->checkBox_board_id->isChecked())
    {
        ui->lineEdit_board_id->setText(settings.value("board_id").toString());
    }
    if (ui->checkBox_chip_name->isChecked())
    {
        ui->lineEdit_chip_name->setText(settings.value("chip_name").toString());
    }
    if (ui->checkBox_chip_stepping->isChecked())
    {
        ui->lineEdit_chip_stepping->setText(settings.value("chip_stepping").toString());
    }
    if (ui->checkBox_board_register_date->isChecked())
    {
        ui->lineEdit_board_register_date->setText(settings.value("board_register_date").toString());
    }
    if (ui->checkBox_board_status->isChecked())
    {
        ui->lineEdit_board_status->setText(settings.value("board_status").toString());
    }
    if (ui->checkBox_user_team->isChecked())
    {
        ui->lineEdit_user_team->setText(settings.value("user_team").toString());
    }
    if (ui->checkBox_current_user->isChecked())
    {
        ui->lineEdit_current_user->setText(settings.value("current_user").toString());
    }
    if (ui->checkBox_board_eco->isChecked())
    {
        ui->lineEdit_board_eco->setText(settings.value("board_eco").toString());
    }
    if (ui->checkBox_lcd_resolution->isChecked())
    {
        ui->lineEdit_lcd_resolution->setText(settings.value("lcd_resolution").toString());
    }
    if (ui->checkBox_lcd_screensize->isChecked())
    {
        ui->lineEdit_lcd_screensize->setText(settings.value("lcd_screensize").toString());
    }
    if (ui->checkBox_ddr_type->isChecked())
    {
        ui->lineEdit_ddr_type->setText(ddr_type_lp_transform(settings.value("ddr_type").toString()));
    }
    if (ui->checkBox_ddr_size->isChecked())
    {
        ui->lineEdit_ddr_size->setText(settings.value("ddr_size").toString());
    }
    if (ui->checkBox_emmc_type->isChecked())
    {
        ui->lineEdit_emmc_type->setText(settings.value("emmc_type").toString());
    }
    if (ui->checkBox_emmc_size->isChecked())
    {
        ui->lineEdit_emmc_size->setText(settings.value("emmc_size").toString());
    }
    if (ui->checkBox_rf_name->isChecked())
    {
        ui->lineEdit_rf_name->setText(settings.value("rf_name").toString());
    }
    if (ui->checkBox_rf_type->isChecked())
    {
        ui->lineEdit_rf_type->setText(settings.value("rf_type").toString());
    }
    if ( ! progress_result)
    {
        ui->statusBar->showMessage("Read Error!!!");
        print_result(false);
    }
    else
    {
        ui->statusBar->showMessage("Read Finished !!!");
        print_result(true);
    }
    set_pushbottom(true);
}

void MainWindow::read_all()
{
    set_pushbottom(false);
    //ui->pushButton_read_all->setText("In Progress");
    QFile::remove(".lastread.ini");
    Gcmd = new QProcess();
    qDebug()<<__LINE__<<__FILE__<<Gcmd->isSequential();
    connect(Gcmd,SIGNAL(readyRead()),this,SLOT(readOutput_byte()));
    waitProcess(binary+" 0 board_type board_id chip_name chip_stepping board_register_date emmc_size board_eco board_status user_team current_user lcd_resolution lcd_screensize ddr_type ddr_size emmc_type rf_name rf_type",10);
    ui->textEdit_log->setText(tempString);
    QString load_file;
    load_file=".lastread.ini";
    QSettings settings(load_file,QSettings::IniFormat);
    settings.sync();
    ui->lineEdit_board_type->setText(settings.value("board_type").toString());
    ui->lineEdit_board_id->setText(settings.value("board_id").toString());
    ui->lineEdit_chip_name->setText(settings.value("chip_name").toString());
    ui->lineEdit_chip_stepping->setText(settings.value("chip_stepping").toString());
    ui->lineEdit_board_register_date->setText(settings.value("board_register_date").toString());
    ui->lineEdit_board_status->setText(settings.value("board_status").toString());
    ui->lineEdit_board_eco->setText(settings.value("board_eco").toString());
    ui->lineEdit_user_team->setText(settings.value("user_team").toString());
    ui->lineEdit_current_user->setText(settings.value("current_user").toString());
    ui->lineEdit_lcd_resolution->setText(settings.value("lcd_resolution").toString());
    ui->lineEdit_lcd_screensize->setText(settings.value("lcd_screensize").toString());
    ui->lineEdit_ddr_type->setText(ddr_type_lp_transform(settings.value("ddr_type").toString()));
    ui->lineEdit_ddr_size->setText(settings.value("ddr_size").toString());
    ui->lineEdit_emmc_type->setText(settings.value("emmc_type").toString());
    ui->lineEdit_emmc_size->setText(settings.value("emmc_size").toString());
    ui->lineEdit_rf_name->setText(settings.value("rf_name").toString());
    ui->lineEdit_rf_type->setText(settings.value("rf_type").toString());
    if ( ! progress_result)
    {
        ui->statusBar->showMessage("Read Error!!!");
        print_result(false);
    }
    else
    {
    ui->statusBar->showMessage("Read Finished !!!");
    print_result(true);
    }
    set_pushbottom(true);

}



void MainWindow::write_all()
{
    set_pushbottom(false);
    QString save_file;
    save_file=".lastwrite.ini";
    QFile::remove(".lastwrite.ini");
    QSettings settings(save_file,QSettings::IniFormat);
    Gcmd = new QProcess();
    qDebug()<<__LINE__<<__FILE__<<Gcmd->isSequential();
    connect(Gcmd,SIGNAL(readyRead()),this,SLOT(readOutput_byte()));


    if (ui->checkBox_board_type->isChecked())
    {
        settings.setValue("board_type",ui->lineEdit_board_type->text());
    }
    if (ui->checkBox_board_id->isChecked())
    {
        settings.setValue("board_id",ui->lineEdit_board_id->text());
    }
    if (ui->checkBox_chip_name->isChecked())
    {
       settings.setValue("chip_name",ui->lineEdit_chip_name->text());
    }
    if (ui->checkBox_chip_stepping->isChecked())
    {
        settings.setValue("chip_stepping",ui->lineEdit_chip_stepping->text());
    }
    if (ui->checkBox_board_register_date->isChecked())
    {
        QDate date=QDate::currentDate();
        QString strDate = date.toString("yy-MM-dd");
        QTime time=QTime::currentTime();
        strDate = strDate+"-"+time.toString("hh:mm");
        ui->lineEdit_board_register_date->setText(strDate);
        settings.setValue("board_register_date",ui->lineEdit_board_register_date->text());
    }
    if (ui->checkBox_board_status->isChecked())
    {
        settings.setValue("board_status",ui->lineEdit_board_status->text());
    }
    if (ui->checkBox_user_team->isChecked())
    {
        settings.setValue("user_team",ui->lineEdit_user_team->text());
    }
    if (ui->checkBox_current_user->isChecked())
    {
        settings.setValue("current_user",ui->lineEdit_current_user->text());
    }
    if (ui->checkBox_board_eco->isChecked())
    {
        settings.setValue("board_eco",ui->lineEdit_board_eco->text());
    }
    if (ui->checkBox_lcd_resolution->isChecked())
    {
        settings.setValue("lcd_resolution",ui->lineEdit_lcd_resolution->text());
    }
    if (ui->checkBox_lcd_screensize->isChecked())
    {
        settings.setValue("lcd_screensize",ui->lineEdit_lcd_screensize->text());
    }
    if (ui->checkBox_ddr_type->isChecked())
    {
        settings.setValue("ddr_type",ddr_type_lp_transform(ui->lineEdit_ddr_type->text()));
    }
    if (ui->checkBox_ddr_size->isChecked())
    {
        settings.setValue("ddr_size",ui->lineEdit_ddr_size->text());
    }
    if (ui->checkBox_emmc_type->isChecked())
    {
        settings.setValue("emmc_type",ui->lineEdit_emmc_type->text());
    }
    if (ui->checkBox_emmc_size->isChecked())
    {
        settings.setValue("emmc_size",ui->lineEdit_emmc_size->text());
    }
    if (ui->checkBox_rf_name->isChecked())
    {
        settings.setValue("rf_name",ui->lineEdit_rf_name->text());
    }
    if (ui->checkBox_rf_type->isChecked())
    {
        settings.setValue("rf_type",ui->lineEdit_rf_type->text());
    }
    settings.sync();
    ui->statusBar->showMessage("Write in Progress !!!");
    waitProcess(binary+" 1 file .lastwrite.ini",10);
    ui->textEdit_log->setText(tempString);
    if ( ! progress_result)
    {
        ui->statusBar->showMessage("WRITE ERROR !!!");
        print_result(false);
    }
    else
    {
    ui->statusBar->showMessage("WRITE FINISH !!!");
    print_result(true);
    }
    set_pushbottom(true);
}


void MainWindow::readOutput_byte()
{
    //qDebug()<<"AAAAAAAA";
    QCoreApplication::processEvents();
    //QString tempString = Gcmd->readAllStandardOutput();
    tempString += Gcmd->readAllStandardOutput();
    Goutput += tempString;
    //tempString= tempString.replace("\n","");
    //    ui->loginfo->append(tempString);
    //ui->textEdit->moveCursor(QTextCursor::End,QTextCursor::MoveAnchor);
   // ui->textEdit->insertPlainText(tempString);

    //ui->lineEdit_board_type->setText(tempString);

}

QProcess::ExitStatus MainWindow::waitProcess(const QString command,const int timeout)
{
    int times=0;
    Gcmd->start(command);
    if(true==Gcmd->waitForStarted())
    {
        qDebug()<<__LINE__<<__FILE__<<command<<"has started";
        progress_result=true;
    }
    else
    {
        qDebug()<<__LINE__<<__FILE__<<command<<"has Failed to start";
        progress_result=false;
        return QProcess::CrashExit;
    }
    setGstopped(false);
    while((Gcmd->state() != QProcess::NotRunning) && (times <= timeout) &&(getGstopped()==false))
    {
        times++;
        QEventLoop eventloop;
        QTimer::singleShot(1000,&eventloop,SLOT(quit()));
        eventloop.exec();
        qDebug()<<__LINE__<<__FILE__<<command<<" has Not finished,times="<<times;
    }

    qDebug()<<__LINE__<<__FILE__<<"the result is"<<"exitStatus"<<Gcmd->exitStatus()<<"exitCode"<<Gcmd->exitCode()<<"state"<<Gcmd->state();
    GcostTimes=times;//record the cost times
    if ( Gcmd->exitCode() != 0 )
    {
        progress_result=false;
    }
    if(times > timeout)
    {
        //Gstopped = true;//running testcase will stop, and set the result column with Fail.
        qDebug()<<__LINE__<<__FILE__<<"CrashExit";
        tempString="Time out !!!!";
        progress_result=false;
        Gcmd->kill();
        return QProcess::CrashExit;
    }
    else if(getGstopped() == true)
    {
        qDebug()<<__LINE__<<__FILE__<<"StoppedExit";
        return QProcess::CrashExit;
    }
    else{
        qDebug()<<__LINE__<<__FILE__<<"NormalExit";
        return QProcess::NormalExit;
    }

}
