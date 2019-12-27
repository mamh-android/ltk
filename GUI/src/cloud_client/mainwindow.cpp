#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{


    ui->setupUi(this);


    //ui->widget_burn_image->setStyleSheet("border:1px solid #222");
    //load_blf_map();
    ui->ltk_image_tree->header()->setResizeMode(QHeaderView::ResizeToContents);
    ui->ltk_image_tree->setStyleSheet("font: 8pt");
    QStringList image_tree_title;
    image_tree_title<<"Image List"<<"Detail";
    ui->ltk_image_tree->setHeaderLabels(image_tree_title);
    is_the_latest_version=true;


    trayicon = new QSystemTrayIcon(this);
      //创建QIcon对象，参数是图标资源，值为项目的资源文件中图标的地址
closed_to_tool_bar=false;
      QIcon icon("./image/icon1.png");
      icon_size.setHeight(4);
      icon_size.setWidth(4);
      un_shared_icon.addFile("./image/CheckboxEmpty.png",icon_size);
      shared_icon.addFile("./image/CheckboxFull.png",icon_size);
      //shared_icon.pixmap()
      trayiconMenu = new QMenu(this);
      //为托盘菜单添加菜单项
      QAction *action_quit = new QAction(this);
      action_quit->setText("Quit");
      QAction *action_close = new QAction(this);
      action_close->setText("Open");
      trayiconMenu->addAction(action_close);
      trayiconMenu->addAction(action_quit);
      //为托盘菜单添加分隔符
      trayiconMenu->addSeparator();
      //将创建的QIcon对象作为系统托盘图标
      trayicon->setIcon(icon);
      //显示托盘图标
      trayicon->show();
      //设置系统托盘提示
      trayicon->setToolTip(tr("Cloud Wind"));
      //将创建菜单作为系统托盘菜单
      trayicon->setContextMenu(trayiconMenu);
      //在系统托盘显示气泡消息提示
      trayicon->showMessage(tr("Board_Tracking"), tr("Hello"), QSystemTrayIcon::Information, 5000);
      //为系统托盘绑定单击信号的槽 即图标激活时
      connect(trayicon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(onSystemTrayIconClicked(QSystemTrayIcon::ActivationReason)));
      connect(action_quit,SIGNAL(triggered()),this,SLOT(close()));
      connect(action_close,SIGNAL(triggered()),this,SLOT(close_to_icon()));
      trayicon->hide();
      select_history=0;
      //list_board();
      ubuntu_13=false;
      lock_flag=false;
      progress_lock=false;
      warning_flag=false;
      debug_mode=false;
      stop_connection=false;
      setWindowIcon(icon);
#ifdef Q_OS_WIN
    ubuntu_13=false;
#else
      Gcmd = new QProcess();
      qDebug()<<__LINE__<<__FILE__<<Gcmd->isSequential();
      connect(Gcmd,SIGNAL(readyRead()),this,SLOT(readOutput_byte()));
      waitProcess("cat /etc/issue",10);
      if (tempString.indexOf("13") >= 0 || tempString.indexOf("12") >= 0 )
      {
          ubuntu_13=true;
          qDebug()<<"ubuntu over 10.XX find !!";
      }
#endif
      //QListWidgetItem* test_demo= new QListWidgetItem(icon,"hello",ui->listWidget_list);

      //QListWidgetItem* test_demo2= new QListWidgetItem(shared_icon,"hello2",ui->listWidget_list);

      ui->listWidget_list->setIconSize(QSize(10,10));
      //ui->pushButton_list->setDisabled(false);

      receiver = new QUdpSocket(this);
      bool bind_result=receiver->bind(QHostAddress::LocalHost, 6789);
      if ( !bind_result )
      {
          //QMessageBox::about(this, tr("Warning"),
          //            tr("<h2>Socket bind error</h2>"
          //               "<p>Please you may have already open the GUI!."));
          int debug=QMessageBox::warning(this,tr("Warning"),tr("Socket bind error \n You may have already open the GUI!\n Do you want to enter Debug Mode?"),QMessageBox::Yes,QMessageBox::No);
                  if ( debug == QMessageBox::Yes )
          { debug_mode=true; }else
          { debug_mode=false;}
      }
      connect(receiver, SIGNAL(readyRead()),this, SLOT(readPendingDatagrams()));

if (debug_mode)
      {ui->listWidget_list->setMaximumWidth(200);}


#ifdef ADD_VERSION
      ui->statusBar->setSizeGripEnabled(true);
      QString cloud_version="Version: ";
      cloud_version+=ADD_VERSION;
      QLabel* version= new QLabel(cloud_version);
      ui->statusBar->addPermanentWidget(version,0);
#endif

    register_name_saved=false;
    register_name="Unknow";
    config_setted=false;
    QString config_file="./ltk_cloud_client_config.ini";
    QSettings settings(config_file,QSettings::IniFormat);
    QString user_name=settings.value("user_name").toString();
    QString user_team=settings.value("user_team").toString();
    if ( ! user_name.isEmpty() && ! user_team.isEmpty())
    {
        config_setted=true;
        register_name=user_name;
        register_team=user_team;
        qDebug()<<"find user info:"<<register_name<<register_team;
    }else
    {
        qDebug()<<"can't get user info";
    }
//warning_limit_num=0;
    warning_limit_num=5;

    ui->pushButton_fw_update->setToolTip("Arduino Fw update");
    show_burning_widget=true;
    //show_burning_setting();
    QStringList platform_list;
    platform_list<<"pxa1908"<<"pxa1928"<<"pxa1936";
    ui->comboBox->addItems(platform_list);
    ui->pushButton_burn_switch->hide();
    QStringList branch_list;
    //branch_list<<"Null"<<"kk4.4"<<"lp5.0_k314_alpha1";
    //ui->comboBox_branch_list->addItems(branch_list);
    connect(ui->comboBox_branch_list,SIGNAL(highlighted(QString)),this,SLOT(show_map(QString)));
    list_board();

    qDebug()<<"Add timing 10001 check";
    update_result = new QTimer;
    connect(update_result,SIGNAL(timeout()),this,SLOT(time_update()));
    //update_result->start(43200000);
    update_result->start(1000);
}

void MainWindow::time_update()
{
    qDebug()<<"############ Check 10001 issue start ###########";
    QMap<QString, client_board_info>::iterator it;
    for (it=board_info.begin();it !=board_info.end();++it)
    {
        qDebug()<<"Check "<<it.key()<<" "<<it.value().shared;
        if (it.value().shared)
        {
            QProcess *cmd1=new QProcess();
            cmd1->start("./rcc_utilc operate "+it.key()+" debug \"ol\"");
            while((cmd1->state() == QProcess::Running) || (cmd1->state() == QProcess::Starting) )
            {
                QEventLoop eventloop;
                QTimer::singleShot(100,&eventloop,SLOT(quit()));
                eventloop.exec();
                qDebug()<<"check arduino status"<<cmd1->state();
            }
            tempString=cmd1->readAll();
            delete cmd1;cmd1=NULL;

            if (tempString.indexOf("1001") >=0 || tempString.indexOf("10001") >=0 || tempString.indexOf("Invalid response") >=0 )
            {
                QString rcc_warning_mesg="<h4>"+it.key()+" Arduion Can't be detected.</h4> Please plug out/in the cable between Arduion and board !!!\n ";
                if (tempString.indexOf("10001") >=0 || tempString.indexOf("Invalid response") >=0 )
                {rcc_warning_mesg="<h4>"+it.key()+" Arduion Can't be detected.</h4> Please plug out/in the cable between Arduion and PC !!!\n ";}
                QMessageBox::about(this,tr("Warning"),rcc_warning_mesg);
            }
        }
    }
    qDebug()<<"############ Check 10001 issue End ###########";
    update_result->start(43200000);
}



void MainWindow::load_blf_map()
{
    ui->comboBox_branch_list->clear();
    qDebug()<<"load_blf_map";
    int system_result;
    system_result=system("wget ftp://10.38.164.23/Daily_Build/daily_image_map.txt -O ./daily_image_map.txt");
    if ( system_result != 0 )
    {
        QMessageBox::about(this,tr("Wget"),tr("Download daily_image_map.txt Fail!! \n please try :  wget ftp://10.38.164.23/Daily_Build/daily_image_map.txt -O ./daily_image_map.txt-"));
    }


    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForCStrings(codec);

    QFile file("daily_image_map.txt");
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug()<<"Can't open the file!"<<endl;
    }
    file_text.clear();
    while(!file.atEnd()) {
        QByteArray line = file.readLine();
        QString str(line);
        file_text=file_text+str;



        for (int i=1;i<str.split("=").count();i++)
        {
            QString branch=str.split("=").at(0);
            branch="pxa"+branch.split("pxa").at(1);
            QString branch_detail=str.split("=").at(1);
            QStringList branch_detail_list=branch_detail.split(";");
            for (int a=0;a<branch_detail_list.count();a++)
            {
                QStringList hw_detail=branch_detail_list.at(a).split(":");
                if (hw_detail.count()>1)
                {
                QString hw=hw_detail.at(0);
                QString hw_blf=hw_detail.at(1);
                daily_image_map[branch+"/"+hw]=str.split("=").at(0)+";"+hw+";"+hw_blf.remove("\n");
                }
            }

        }

    }

/*
    QStringList branch_list;
    QMap<QString,QString>::Iterator it2;
    for (it2=daily_image_map.begin();it2 !=daily_image_map.end();++it2)
    {
        branch_list<<it2.key();
    }
    ui->comboBox_branch_list->addItems(branch_list);
*/

    //show_blf_map();
}

void MainWindow::show_blf_map()
{

    qDebug()<<"show image tree\n";




    QString platform=ui->comboBox->currentText();
    ui->ltk_image_tree->clear();
    QStringList image_by_day=file_text.split("\n");
    QTreeWidgetItem* image_day;
    for (int i=0;i<(image_by_day.count()-1);i++)
    {
        bool not_only_1908=true;
        if ( platform=="pxa1908" && image_by_day.at(i).indexOf("pxa1U88") >= 0 )
        {not_only_1908=false;}
        if ( image_by_day.at(i).indexOf(platform) >= 0 && not_only_1908 )
        {
            image_day=new QTreeWidgetItem(ui->ltk_image_tree);

            QStringList image_by_day_branch1=image_by_day.at(i).split("=");
            QStringList image_by_day_branch2=image_by_day.at(i).split("=").at(1).split(";");
            image_day->setText(0,image_by_day_branch1.at(0));
            //image_day->setText(1,"version");
            QString blf_string_by_day="version---";
            for (int a=0;a<image_by_day_branch2.count();a++)
            {
                QTreeWidgetItem* image_image=new QTreeWidgetItem;
                QString image=image_by_day_branch2.at(a).split(":").at(0);
                QStringList image_blf=image_by_day_branch2.at(a).split(":").at(1).split("+");
                image_image->setText(0,image);
                //image_image->setText(1,"platform_def---");
                QString blf_string_by_def="platform_def---"+image_by_day_branch1.at(0)+"++"+image+":";
                image_day->addChild(image_image);
                blf_string_by_day+=image+":";
                for (int b=0;b<image_blf.count();b++)
                {
                    QTreeWidgetItem* image_blf_item=new QTreeWidgetItem;
                    image_blf_item->setText(0,image_blf.at(b));
                    if (b== (image_blf.count()-1) )
                    {
                        blf_string_by_day+=image_blf.at(b);
                        blf_string_by_def+=image_blf.at(b);
                    }else
                    {
                        blf_string_by_day+=image_blf.at(b)+";";
                        blf_string_by_def+=image_blf.at(b)+";";
                    }
                    image_image->addChild(image_blf_item);
                    image_blf_item->setText(1,image_by_day_branch1.at(0)+";"+image+";"+image_blf.at(b));
                }
                image_image->setText(1,blf_string_by_def);
                //image_image->setExpanded(true);

            }
            image_day->setText(1,blf_string_by_day);
            //image_day->setExpanded(true);

        }

    }
    ui->ltk_image_tree->sortItems(0,Qt::DescendingOrder);
    ui->ltk_image_tree->setCurrentItem(ui->ltk_image_tree->topLevelItem(0)->child(0)->child(0));
}


void MainWindow::show_burning_setting()
{
    if (show_burning_widget)
    {
        ui->textEdit_detail->setHidden(true);
        ui->widget_burn_image->setHidden(false);
        ui->pushButton_burn_switch->setText("Return Cloud");
        lock_button(true);
        ui->pushButton_fw_update->setDisabled(true);
        show_burning_widget=false;
    }else
    {
        ui->textEdit_detail->setHidden(false);
        ui->widget_burn_image->setHidden(true);
        ui->pushButton_burn_switch->setText("Burn Image");
        lock_button(false);
        ui->pushButton_fw_update->setDisabled(false);
        ui->pushButton_unshare->setDisabled(true);
        show_burning_widget=true;
    }
}

void MainWindow::ask_for_user_name()
{
    //ask for user name
        ask_user_name=new QDialog;
        ask_user_name->setWindowTitle("Please Enter Your Register Info");
        button_ok=new QPushButton("OK");
        //register_name_line=new QLineEdit;
        register_name_line=new mylineedit;
        register_team_line=new mylineedit;
        //commbox_share_level=new QComboBox;
        QGridLayout *gridLayout_ask_dialog=new QGridLayout(ask_user_name);
        gridLayout_ask_dialog->addWidget(register_name_line,0,0);
        gridLayout_ask_dialog->addWidget(register_team_line,0,1);
        //gridLayout_ask_dialog->addWidget(commbox_share_level,0,2);
        gridLayout_ask_dialog->addWidget(button_ok,0,2);
        gridLayout_ask_dialog->setSpacing(1);
        //button_ok->setMinimumHeight(45);
        //gridLayout_ask_dialog->addWidget(button_ok,0,1,1,2);
        connect(button_ok,SIGNAL(clicked()),this,SLOT(save_register_name()));

        //commbox_share_level->clear();
        //QStringList share_level_list;
        //share_level_list<<"Share_For_All"<<"Share_For_Team"<<"Share_For_Local";
        //commbox_share_level->addItems(share_level_list);

        QStringList wordList;
        /*
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
        */
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



        QCompleter *completer = new QCompleter(wordList, this);
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        completer->setCompletionMode(QCompleter::PopupCompletion);
        register_name_line->setCompleter(completer);



        register_name_line->setText("User_name");
        register_team_line->setText("User_Team");
        //register_name_line->setSelection(0,5);
    register_name_line->setMaximumWidth(100);
    register_team_line->setMaximumWidth(200);
    //register_name_line->selectAll();

        QStringList userteam_list;
        QFile file1("user_team_list.txt");
         if(!file1.open(QIODevice::ReadOnly | QIODevice::Text)) {
             qDebug()<<"Can't open the file!"<<endl;
         }
         while(!file1.atEnd()) {
             QByteArray line = file1.readLine();
             QString str(line);
             str= str.replace("\n","");
             userteam_list << str;
        }
         file1.close();

         QCompleter *completer_team_list = new QCompleter(userteam_list, this);
         completer_team_list->setCaseSensitivity(Qt::CaseInsensitive);
         completer_team_list->setCompletionMode(QCompleter::PopupCompletion);
         register_team_line->setCompleter(completer_team_list);

connect(register_name_line,SIGNAL(clicked()),register_name_line,SLOT(clear()));
connect(register_team_line,SIGNAL(clicked()),register_team_line,SLOT(clear()));
        ask_user_name->exec();

        //ask for user name
}

void MainWindow::reg_name_select_all()
{
    register_name_line->selectAll();
}


void MainWindow::save_register_name()
{
    register_name=register_name_line->text();
    register_name=register_name.replace(" ","_");
    register_team=register_team_line->text();
    register_team=register_team.replace(" ","_");
    QString config_file="./ltk_cloud_client_config.ini";
    QSettings settings(config_file,QSettings::IniFormat);
    if ( register_name.isEmpty() || register_name=="User_name" )
    {register_name="Unknow";}
    else
    {
        settings.setValue("user_name",register_name);
    }
    if ( register_team.isEmpty() || register_team=="User_Team" )
    {register_team="Unknow";}
    else
    {
        settings.setValue("user_team",register_team);
    }
    settings.sync();
    qDebug()<<"read register name"<<register_name<<register_team;
    register_name_saved=true;
    ask_user_name->close();
}


void MainWindow::readPendingDatagrams()
{
    while (receiver->hasPendingDatagrams())
    {
    QByteArray datagram;
    datagram.resize(receiver->pendingDatagramSize());
    receiver->readDatagram(datagram.data(), datagram.size());
    //qDebug()<<datagram.data();
    gui_msg_t *read=(gui_msg_t *)datagram.data();
    qDebug()<<read->op<<read->magic<<read->param;
    QString read_mesg=read->param;
	qDebug()<<stop_connection;
    //if ( read->magic == 0xABCDBDAC )
    {
        if ( read->op == 0 )
        {
            qDebug()<<"do nothing";
        }
        if ( read->op == 1 )
        {
            ui->statusBar->showMessage(read_mesg);
        }
        if ( read->op == 2 )
        {
            QMessageBox::about(this, tr("Warning"),
                        read_mesg);
            warning_flag=true;
            /*QMessageBox::about(this, tr("Warning"),
                        tr("<h2>Serial Port Conflict !!</h2>"
                           "<p>Please close your serial tool such as minicom."));*/
        }
        if ( read->op == 3 && !stop_connection )
        {
            qDebug()<<"list board";
            list_board();
        }
        if ( read->op == 4 )
        {
            qDebug()<<"update table";
        }
        if ( read->op == 5 )
        {
            qDebug()<<"update table";
            changed_board=read_mesg.split(":").at(0);
            if (read_mesg.split(":").count()>1)
            {
            job_status=read_mesg.split(":").at(1);
            }else
            {
                job_status="Unknow";
            }
            job_status_map[changed_board]=job_status;
            changed_board=changed_board.replace(" ","");
            qDebug()<<changed_board<<ui->listWidget_list->currentItem()->text();
            if ( changed_board == ui->listWidget_list->currentItem()->text().replace(" ","") )
            {
                show_board_detail(changed_board);
            }
        }
        if ( read->op == 6 )
        {
            qDebug()<<"10001 issue and unshare the board";
            QString board_id_10001_issue=read_mesg;
            //QString board_id_bak=board_id_action;
            board_id_action=board_id_10001_issue;
            QString rcc_warning_mesg="<h4>"+board_id_action+" is abnormal: Arduion Can't be detected </h4>\n"
                    +"<p>\n System will auto un-share this board.</p>"
                    +"\n Please plug out/in the cable between Arduion and PC"
                    +"<p>\n Then share this board again , Thank you ^_^ \n </p>";
            QMessageBox::about(this,tr("Warning"),rcc_warning_mesg);
            unshare_board();
            //board_id_action=board_id_bak;
        }
    }

    //NOTIFY_TYPE_UNKNOWN     = 0,  /*means nothing*/
    //NOTIFY_TYPE_INDICATION  = 1,  /*tell GUI to show/display something*/
    //NOTIFY_TYPE_DEV_CHANGED = 3,  /*there is change of device or cables mapping*/
    //NOTIFY_TYPE_GET_NEWJOB  = 4,  /*has new job on some board*/
    //NOTIFY_TYPE_JOB_CHANGED = 5,  /*there is change of job status*/

    }
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::lock_usage()
{
    ui->pushButton_download->setDisabled(lock_flag);
    ui->pushButton_onkey->setDisabled(lock_flag);
    ui->pushButton_power_off->setDisabled(lock_flag);
    ui->pushButton_power_off_soft->setDisabled(lock_flag);
    ui->pushButton_power_on->setDisabled(lock_flag);
    ui->pushButton_reboot->setDisabled(lock_flag);

    ui->pushButton_fw_update->setDisabled(lock_flag);
    ui->pushButton_emmd_dump->setDisabled(lock_flag);
    //ui->pushButton_list->setDisabled(false);
    ui->pushButton_burn_switch->setDisabled(lock_flag);
    if (lock_flag)
    {
        show_burning_widget=false;
        //show_burning_setting();
        ui->pushButton_share->setDisabled(lock_flag);
        ui->pushButton_unshare->setDisabled(false);
    }
    else
    {
        ui->pushButton_share->setDisabled(lock_flag);
        ui->pushButton_unshare->setDisabled(true);
    }
}

void MainWindow::lock_button(bool lock_flag1)
{
    ui->pushButton_download->setDisabled(lock_flag1);
    ui->pushButton_onkey->setDisabled(lock_flag1);
    ui->pushButton_power_off->setDisabled(lock_flag1);
    ui->pushButton_power_off_soft->setDisabled(lock_flag1);
    ui->pushButton_power_on->setDisabled(lock_flag1);
    ui->pushButton_reboot->setDisabled(lock_flag1);
    ui->pushButton_share->setDisabled(lock_flag1);
    ui->pushButton_unshare->setDisabled(lock_flag1);
    ui->pushButton_list->setDisabled(lock_flag1);
    ui->pushButton_emmd_dump->setDisabled(lock_flag1);
   // ui->pushButton_download->set
    qDebug()<<"lock lock_flag1:"<<lock_flag1;

}


void MainWindow::close_to_icon()
{
    //trayIcon = new QSystemTrayIcon(this);
    //QIcon icon("./file2.JPG");
    //trayiconMenu = new QMenu(this);
    qDebug()<<"dd";
    this->show();
    closed_to_tool_bar=false;
    trayicon->hide();
}


void MainWindow::reboot()
{
    ui->pushButton_reboot->setDisabled(true);
    //Gcmd = new QProcess();
    //connect(Gcmd,SIGNAL(readyRead()),this,SLOT(readOutput_byte()));
    //waitProcess("./rcc_utilc operate "+board_id_action+" reset",10);
    qDebug()<<"beginreboot";
    QProcess *cmd1=new QProcess();
    cmd1->start("./rcc_utilc operate "+board_id_action+" reset");
    while((cmd1->state() == QProcess::Running) || (cmd1->state() == QProcess::Starting) )
    {
        QEventLoop eventloop;
        QTimer::singleShot(100,&eventloop,SLOT(quit()));
        eventloop.exec();
        qDebug()<<"rebootboard"<<cmd1->state();
    }
    tempString=cmd1->readAll();
    qDebug()<<"overreoboot";

    if ( cmd1->exitCode() == 0 )
    { ui->statusBar->showMessage("Setting Pass");}
    else
    { ui->statusBar->showMessage("Setting Fail");}
    //delete Gcmd;
    //Gcmd=NULL;
    lock_usage();
}


void MainWindow::emmd_dump()
{
    ui->pushButton_emmd_dump->setDisabled(true);
    //Gcmd = new QProcess();
    //connect(Gcmd,SIGNAL(readyRead()),this,SLOT(readOutput_byte()));
    //waitProcess("./rcc_utilc operate "+board_id_action+" reset",10);
    qDebug()<<"beginreboot";
    QProcess *cmd1=new QProcess();
    cmd1->start("./rcc_utilc operate "+board_id_action+" forceEmmd");
    while((cmd1->state() == QProcess::Running) || (cmd1->state() == QProcess::Starting) )
    {
        QEventLoop eventloop;
        QTimer::singleShot(100,&eventloop,SLOT(quit()));
        eventloop.exec();
        qDebug()<<"rebootboard"<<cmd1->state();
    }
    tempString=cmd1->readAll();
    qDebug()<<"overreoboot";

    if ( cmd1->exitCode() == 0 )
    { ui->statusBar->showMessage("Setting Pass");}
    else
    { ui->statusBar->showMessage("Setting Fail");}
    //delete Gcmd;
    //Gcmd=NULL;
    lock_usage();
    ui->pushButton_emmd_dump->setDisabled(false);
}





void MainWindow::fw_update()
{


    QFile* rcc_fw=new QFile("./arduino.fw");
    if ( rcc_fw->exists() )
    {
    ui->pushButton_fw_update->setDisabled(true);
    ui->pushButton_fw_update->setText("Updating");
    ui->statusBar->showMessage("Writing arduino.fw to "+board_id_action);
    //progress_wait_protect();
    Gcmd = new QProcess();
    waitProcess("./rcc_utilc update firmware "+board_id_action+" arduino.fw",1000);

    if ( progress_result )
    { ui->statusBar->showMessage("Updating Pass");
        QMessageBox::about(this, tr("Firmware"),
                    tr("<h2>Arduino Bunning</h2>"
                       "<p>Arduino firmware burning success!"));
    }
    else
    { ui->statusBar->showMessage("Updating Fail");}
    delete Gcmd;
    Gcmd=NULL;
    lock_usage();
    ui->pushButton_fw_update->setDisabled(false);
    ui->pushButton_fw_update->setText("FW Update");
    }else
    {

        QMessageBox::about(this, tr("Warning"),
                    tr("<h2>Updating Error</h2>"
                       "<p>Can't Find arduino Firmware @ ~/ltk_cloud_client."));

    }
    show_board_detail(board_id_action);


}


void MainWindow::download()
{
    ui->pushButton_download->setDisabled(true);
    //Gcmd = new QProcess();
    //connect(Gcmd,SIGNAL(readyRead()),this,SLOT(readOutput_byte()));
    //waitProcess("./rcc_utilc operate "+board_id_action+" dkSWD",10);

    QProcess *cmd1=new QProcess();
    cmd1->start("./rcc_utilc operate "+board_id_action+" dkSWD");
    while((cmd1->state() == QProcess::Running) || (cmd1->state() == QProcess::Starting) )
    {
        QEventLoop eventloop;
        QTimer::singleShot(100,&eventloop,SLOT(quit()));
        eventloop.exec();
        qDebug()<<"downloadboard"<<cmd1->state();
    }


    if ( cmd1->exitCode() == 0 )
    { ui->statusBar->showMessage("Setting Pass");}
    else
    { ui->statusBar->showMessage("Setting Fail");}
    delete Gcmd;
    Gcmd=NULL;
    lock_usage();
}

void MainWindow::power_off()
{
    ui->pushButton_power_off->setDisabled(true);
   // Gcmd = new QProcess();
   // connect(Gcmd,SIGNAL(readyRead()),this,SLOT(readOutput_byte()));
    //waitProcess("./rcc_utilc operate "+board_id_action+" powerOff",10);


    qDebug()<<"beginpoweroff";
    QProcess *cmd1=new QProcess();
    cmd1->start("./rcc_utilc operate "+board_id_action+" powerOff");
    while((cmd1->state() == QProcess::Running) || (cmd1->state() == QProcess::Starting) )
    {
        QEventLoop eventloop;
        QTimer::singleShot(100,&eventloop,SLOT(quit()));
        eventloop.exec();
        qDebug()<<"poweroffboard"<<cmd1->state();
    }
    tempString=cmd1->readAll();
    qDebug()<<"overpoweroff";


    if (  cmd1->exitCode() == 0 )
    { ui->statusBar->showMessage("Setting Pass");}
    else
    { ui->statusBar->showMessage("Setting Fail");}
    delete Gcmd;
    Gcmd=NULL;
    lock_usage();
}

void MainWindow::power_on()
{
    ui->pushButton_power_on->setDisabled(true);
    Gcmd = new QProcess();
    connect(Gcmd,SIGNAL(readyRead()),this,SLOT(readOutput_byte()));
    waitProcess("./rcc_utilc operate "+board_id_action+" powerOn",100);
    waitProcess("./rcc_utilc operate "+board_id_action+" onkey",100);
    if ( progress_result )
    { ui->statusBar->showMessage("Setting Pass");}
    else
    { ui->statusBar->showMessage("Setting Fail");}
    delete Gcmd;
    Gcmd=NULL;
    lock_usage();
}

void MainWindow::power_off_soft()
{
    ui->pushButton_power_off_soft->setDisabled(true);
    Gcmd = new QProcess();
    connect(Gcmd,SIGNAL(readyRead()),this,SLOT(readOutput_byte()));
    waitProcess("./rcc_utilc operate "+board_id_action+" onkey 3000",100);
    if ( progress_result )
    { ui->statusBar->showMessage("Setting Pass");}
    else
    { ui->statusBar->showMessage("Setting Fail");}
    delete Gcmd;
    Gcmd=NULL;
    lock_usage();
}

void MainWindow::onkey()
{
    ui->pushButton_onkey->setDisabled(true);
    Gcmd = new QProcess();
    connect(Gcmd,SIGNAL(readyRead()),this,SLOT(readOutput_byte()));
    waitProcess("./rcc_utilc operate "+board_id_action+" onkey",100);
    if ( progress_result )
    { ui->statusBar->showMessage("Setting Pass");}
    else
    { ui->statusBar->showMessage("Setting Fail");}
    delete Gcmd;
    Gcmd=NULL;
    lock_usage();

}


void MainWindow::list_board()
{
    //progress_wait_protect();
    /*
    if ( progress_lock )
    {
        qDebug()<<"progress lock";
    }else
    {

    lock_button(true);
    ui->listWidget_list->clear();
    Gcmd = new QProcess();
    qDebug()<<__LINE__<<__FILE__<<Gcmd->isSequential();
    waitProcess("./rcc_utilc list regInfo",10);
    tempString=Gcmd->readAll();
    */
    //if (!progress_lock){

    stop_connection=true;
    lock_button(true);
    ui->listWidget_list->clear();
    qDebug()<<"beginlist";
    QProcess *cmd1=new QProcess();
    cmd1->start("./rcc_utilc list regInfo");
    while((cmd1->state() == QProcess::Running) || (cmd1->state() == QProcess::Starting) )
    {
        QEventLoop eventloop;
        QTimer::singleShot(100,&eventloop,SLOT(quit()));
        eventloop.exec();
        //qDebug()<<"list board info"<<cmd1->state();
    }
    tempString=cmd1->readAll();
    qDebug()<<"overlist";
    stop_connection=false;
    if (tempString.indexOf("PXA8888_FAKE_ID") >= 0 )
    {
        if ( warning_limit_num < 5 )
        {
        QMessageBox::about(this, tr("Warning"),
                    tr("<h2>Fake Board Find</h2>"
                       "<p>Please Plug in/out the usb/serial cable for board remapping!."));
               warning_limit_num++;
        }else
        {
            ui->statusBar->showMessage("Fake Board Find!!");
        }
    }
//    qDebug()<<"list"<<tempString<<" other="<<Gcmd->readAll();
    QStringList board_list_ori=tempString.split("\n");
    //board_list_ori.removeFirst();
    board_list_ori.removeLast();
    qDebug()<<"list"<<tempString;
    int board_number=board_list_ori.count();
    QString temp_board;
    QStringList temp_board_list;
    board_list.clear();
    //un_shared_icon.addFile("./image/CheckboxEmpty.png",icon_size);
    //shared_icon.addFile("./image/CheckboxFull.png",icon_size);
    //QString split_key="\0:\0";
    board_info.clear();
    for ( int i=0;i<board_number;i++ )
    {
        if (board_list_ori.at(i).indexOf("PXA8888_FAKE_ID") < 0 )
        {
            qDebug()<<"ggg"<<board_list_ori.at(i);
            temp_board=board_list_ori.at(i);
            if (temp_board.indexOf("Success") >= 0 )
            {temp_board="No Board Find";}

            temp_board_list=temp_board.split(":");
            if ( temp_board_list.count() == 1 )
                temp_board_list<<"unknow";

            if ( temp_board_list.at(1).indexOf("unshare") >= 0 )
            {
                QListWidgetItem* temp_item=new QListWidgetItem(un_shared_icon,temp_board_list.at(0),ui->listWidget_list);
                //temp_item->setSizeHint(icon_size);
                //ui->listWidget_list->insertItem(temp_item);
                board_info[temp_board_list.at(0)].shared=false;
            }
            else
            {
                QListWidgetItem* temp_item2=new QListWidgetItem(shared_icon,temp_board_list.at(0),ui->listWidget_list);
                //QListWidgetItem* test_demo2= new QListWidgetItem(shared_icon,"hello2",ui->listWidget_list);
                board_info[temp_board_list.at(0)].shared=true;
            }
            ui->statusBar->showMessage("Listing Board");
            board_list<<temp_board_list.at(0);

        }
    }




    //ui->listWidget_list->addItems(board_list_ori);
    ui->listWidget_list->setCurrentRow(select_history);
    if ( cmd1->exitCode() == 0 )
    { ui->statusBar->showMessage("List Pass");}
    else
    { ui->statusBar->showMessage("List Fail");}
    //delete Gcmd;
    //Gcmd=NULL;
    //lock_button(false);
    ui->pushButton_list->setDisabled(false);

    //}
    //}
}

void MainWindow::show_board_detail(QString board_id)
{

    board_id_action=board_id;
    qDebug()<< "line_index" << ui->listWidget_list->currentRow();

    //Gcmd = new QProcess();
    //waitProcess("./rcc_utilc query "+board_id+" jobStatus",10);
    //QString job_status=Gcmd->readAll();

    QFile::remove("board_info.ini");
    QString list_detail="./rcc_utilc query "+board_id+" allInfo > board_info.ini";
    char *cmd;
    qDebug()<<"all info"<<list_detail;
    cmd =list_detail.toLatin1().data();
    system(cmd);
    if ( progress_result )
    { ui->statusBar->showMessage("List Pass");}
    else
    { ui->statusBar->showMessage("List Fail");}

    QString load_file="board_info.ini";
    QSettings settings(load_file,QSettings::IniFormat);
    settings.sync();

    QString item_name_style="<tr><td style=\"font-size:10px;font-weight:bold;\" width=\"120\">";
    QString item_value_style="<td style=\"font-size:10px;\">";
    QString item_group_style="<tr><td style=\"font-size:11px;font-weight:bold;\" colspan=\"2\" bgcolor=\"#99CCFF\">";
    QString item_value_style_warning="<td style=\"font-size:10px;color:#FF0000;\">";

    QString table_string="<table border=\"1\" width=\"270\" >";

    table_string+=item_group_style;
    table_string+="EEPROM INFO";
    table_string+="</td>";
    table_string+="</tr>";

    table_string+=item_name_style;
    table_string+="Board_ID";
    table_string+="</td>";
    table_string+=item_value_style;
    table_string+=settings.value("board_id").toString();
    table_string+="</td>";
    table_string+="</tr>";

    table_string+=item_name_style;
    table_string+="board_type";
    table_string+="</td>";
    table_string+=item_value_style;
    table_string+=settings.value("board_type").toString();
    table_string+="</td>";
    table_string+="</tr>";

    table_string+=item_name_style;
    table_string+="board_register_date";
    table_string+="</td>";
    table_string+=item_value_style;
    table_string+=settings.value("board_register_date").toString();
    table_string+="</td>";
    table_string+="</tr>";

    table_string+=item_name_style;
    table_string+="board_status";
    table_string+="</td>";
    table_string+=item_value_style;
    table_string+=settings.value("board_status").toString();
    table_string+="</td>";
    table_string+="</tr>";

    table_string+=item_name_style;
    table_string+="user_team";
    table_string+="</td>";
    table_string+=item_value_style;
    table_string+=settings.value("user_team").toString();
    table_string+="</td>";
    table_string+="</tr>";

    table_string+=item_name_style;
    table_string+="current_user";
    table_string+="</td>";
    table_string+=item_value_style;
    table_string+=settings.value("current_user").toString();
    table_string+="</td>";
    table_string+="</tr>";

    table_string+=item_name_style;
    table_string+="board_eco";
    table_string+="</td>";
    table_string+=item_value_style;
    table_string+=settings.value("board_eco").toString();
    table_string+="</td>";
    table_string+="</tr>";

    table_string+=item_name_style;
    table_string+="lcd_resolution";
    table_string+="</td>";
    table_string+=item_value_style;
    table_string+=settings.value("lcd_resolution").toString();
    table_string+="</td>";
    table_string+="</tr>";

    table_string+=item_name_style;
    table_string+="lcd_screensize";
    table_string+="</td>";
    table_string+=item_value_style;
    table_string+=settings.value("lcd_screensize").toString();
    table_string+="</td>";
    table_string+="</tr>";

    table_string+=item_name_style;
    table_string+="chip_name";
    table_string+="</td>";
    table_string+=item_value_style;
    table_string+=settings.value("chip_name").toString();
    QString board_chip_name=settings.value("chip_name").toString();
    table_string+="</td>";
    table_string+="</tr>";

    table_string+=item_name_style;
    table_string+="chip_stepping";
    table_string+="</td>";
    table_string+=item_value_style;
    table_string+=settings.value("chip_stepping").toString();
    table_string+="</td>";
    table_string+="</tr>";

    table_string+=item_name_style;
    table_string+="ddr_type";
    table_string+="</td>";
    table_string+=item_value_style;
    table_string+=ddr_type_lp_transform(settings.value("ddr_type").toString());
    table_string+="</td>";
    table_string+="</tr>";

    table_string+=item_name_style;
    table_string+="ddr_size_freq";
    table_string+="</td>";
    table_string+=item_value_style;
    table_string+=settings.value("ddr_size").toString();
    table_string+="</td>";
    table_string+="</tr>";

    table_string+=item_name_style;
    table_string+="emmc_type";
    table_string+="</td>";
    table_string+=item_value_style;
    table_string+=settings.value("emmc_type").toString();
    table_string+="</td>";
    table_string+="</tr>";

    table_string+=item_name_style;
    table_string+="emmc_size";
    table_string+="</td>";
    table_string+=item_value_style;
    table_string+=settings.value("emmc_size").toString();
    table_string+="</td>";
    table_string+="</tr>";

    table_string+=item_name_style;
    table_string+="rf_name";
    table_string+="</td>";
    table_string+=item_value_style;
    table_string+=settings.value("rf_name").toString();
    table_string+="</td>";
    table_string+="</tr>";

    table_string+=item_name_style;
    table_string+="rf_type";
    table_string+="</td>";
    table_string+=item_value_style;
    table_string+=settings.value("rf_type").toString();
    table_string+="</td>";
    table_string+="</tr>";

    table_string+=item_group_style;
    table_string+="EXTRA INFO";
    table_string+="</td>";
    table_string+="</tr>";

    table_string+=item_name_style;
    table_string+="DRO";
    table_string+="</td>";
    table_string+=item_value_style;
    table_string+=settings.value("DRO").toString();
    table_string+="</td>";
    table_string+="</tr>";

    table_string+=item_name_style;
    table_string+="CHIP_TYPE";
    table_string+="</td>";
    table_string+=item_value_style;
    table_string+=settings.value("chip_type").toString();
    table_string+="</td>";
    table_string+="</tr>";

    table_string+=item_group_style;
    table_string+="CONNECTION INFO";
    table_string+="</td>";
    table_string+="</tr>";



    if ( settings.value("rcc_dev").toString().indexOf("ttyACM") < 0)
    {
        ui->pushButton_fw_update->setDisabled(true);
    }else
    {
        ui->pushButton_fw_update->setDisabled(false);
    }

    QString rcc_value=settings.value("rcc_dev").toString();
    //QString adb_value=settings.value("adb_id").toString();
    QString adb_value="MTP";
    QString serial_value=settings.value("serial_dev").toString();



    if (  rcc_value.isEmpty() || adb_value.isEmpty() || serial_value.isEmpty() || rcc_value=="null" || adb_value=="null" || serial_value=="null" )
    {board_connection_ok=false;}
    else
    {board_connection_ok=true;}

    if (  rcc_value.isEmpty() || rcc_value=="null"  )
    {
        rcc_connect_status="(Null)";
        //rcc_value=item_value_style+"null";
        rcc_value=item_value_style_warning+"Please plug in/out the Arduino cable to retry";
    }
    else
    {
        rcc_connect_status="(OK)";
        rcc_value=item_value_style+rcc_value;
    }

    if (  adb_value.isEmpty() || adb_value=="null" )
    {
        adb_connect_status="(Null)";
        adb_value=item_value_style+"null";
        //adb_value=item_value_style_warning+"Please plug in/out the ADB cable to retry";
    }
    else
    {
        adb_connect_status="(OK)";
        adb_value=item_value_style+adb_value;
    }

    if ( serial_value.isEmpty() || serial_value=="null" )
    {
        serial_connect_stauts="(Null)";
        //serial_value=item_value_style+"null";
        serial_value=item_value_style_warning+"Please plug in/out the Serial cable to retry";
        serial_node="Null";
    }
    else
    {
        serial_node=serial_value;
        serial_connect_stauts="(OK)";
        serial_value=item_value_style+serial_value;

    }

    ui->comboBox_branch_list->clear();
    ui->comboBox_branch_list->addItem("Null");
    QStringList branch_list;
    QMap<QString,QString>::Iterator it2;
    for (it2=daily_image_map.begin();it2 !=daily_image_map.end();++it2)
    {
        QString branch_key=it2.key();
        if (branch_key.indexOf(board_chip_name.toLower()) >= 0)
        {branch_list<<it2.key();}
    }
    ui->comboBox_branch_list->addItems(branch_list);

    table_string+=item_name_style;
    table_string+="serial_dev";
    table_string+="</td>";
    table_string+=serial_value;
    table_string+="</td>";
    table_string+="</tr>";

    table_string+=item_name_style;
    table_string+="rcc_dev";
    table_string+="</td>";
    table_string+=rcc_value;
    table_string+="</td>";
    table_string+="</tr>";

    /*
    table_string+=item_name_style;
    table_string+="adb_id";
    table_string+="</td>";
    table_string+=adb_value;
    table_string+="</td>";
    table_string+="</tr>";
    */

    table_string+=item_name_style;
    table_string+="rndis_eth";
    table_string+="</td>";
    table_string+=item_value_style;
    table_string+=settings.value("rndis_eth").toString();
    table_string+="</td>";
    table_string+="</tr>";

    table_string+=item_group_style;
    table_string+="SHARE INFO";
    table_string+="</td>";
    table_string+="</tr>";

    table_string+=item_name_style;
    table_string+="shared";
    table_string+="</td>";
    table_string+=item_value_style;
    table_string+=settings.value("shared").toString();
    table_string+="</td>";
    table_string+="</tr>";

    table_string+=item_name_style;
    table_string+="state";
    table_string+="</td>";
    table_string+=item_value_style;
    table_string+=settings.value("state").toString();
    table_string+="</td>";
    table_string+="</tr>";

    table_string+=item_name_style;
    table_string+="Job state";
    table_string+="</td>";
    table_string+=item_value_style;
    if ( job_status_map[board_id].isEmpty() )
    {
        job_status_map[board_id]="Unknow";
    }
    table_string+=job_status_map[board_id];
    table_string+="</td>";
    table_string+="</tr>";


    table_string+=item_name_style;
    table_string+="Sharer";
    table_string+="</td>";
    table_string+=item_value_style;
    table_string+=register_team+":"+register_name;
    table_string+="</td>";
    table_string+="</tr>";


    table_string+="</table>";

    ui->textEdit_detail->setText(table_string);
    if ( settings.value("shared").toString().indexOf("yes") >= 0 )
    {
    lock_flag=true;
    }else
    {lock_flag=false;}
    qDebug()<<"share_status:"<<settings.value("shared").toString().indexOf("yes")<<stop_connection;
    if ( !stop_connection )
    {lock_usage();}
    if (board_id.indexOf("PXA8888_FAKE_ID")>=0 )
    {
/*
        if ( warning_limit_num < 5 )
        {
            QMessageBox::about(this, tr("Warning"),
                        tr("<h2>You Choose A Fake Board</h2>"
                           "<p>Please Plug in/out the usb/serial cable for board mapping!</p>"
                           "<p>For Fake board, You can't use remote control</p>"
                           ));
            warning_limit_num++;
        }else
        {
            ui->statusBar->showMessage("Fake Board Find!!");
        }
*/
        show_burning_widget=false;
        //show_burning_setting();
        ui->pushButton_burn_switch->setDisabled(true);
        lock_button(true);
    }

}

void MainWindow::version_check()
{
    if ( system("ltkcclient -v") != 0 )
    {
        is_the_latest_version=false;
        close_gui();
    }
}

void MainWindow::close_gui()
{
    QMessageBox::about(this,tr("Warning:LTK Cloud Client will auto closed !!!"),QString("There is a new LTK Cloud Client is released !!! \nLTK Cloud will auto closed and install the latest LTK Cloud. \n\nAfter LTK Cloud closed. \Please use \"sudo ltkcclient -s\" to restart LTK Cloud.\n\nPlease press \"O.K\" button to continue"));
    qDebug()<<"exit gui";

    stop_connection=true;
    bool job_running=false;
    for (int i=0;i<board_list.count();i++)
    {
        QProcess *cmd1=new QProcess();
        cmd1->start("./rcc_utilc query "+board_list.at(i)+" jobStatus");
        int event_loop=0;
        while(((cmd1->state() == QProcess::Running) || (cmd1->state() == QProcess::Starting)) && event_loop<100 )
        {
            QEventLoop eventloop;
            QTimer::singleShot(100,&eventloop,SLOT(quit()));
            eventloop.exec();
            event_loop++;
            qDebug()<<"list job status"<<cmd1->state()<<event_loop;
        }
            tempString=cmd1->readAll();
            if ( tempString.indexOf("No job running") < 0 )
            {
                job_running=true;
            }
    }

        ui->pushButton_unshare->setEnabled(false);

        ui->statusBar->showMessage("Job Canceling");
        for (int i=0;i<board_list.count();i++)
        {
        Gcmd = new QProcess();
        connect(Gcmd,SIGNAL(readyRead()),this,SLOT(readOutput_byte()));
        waitProcess("./rcc_utilc ltkex "+board_list.at(i)+" killJob",10000);
        if ( progress_result )
        {
            ui->statusBar->showMessage("Kill Job Success");
        }
        else
        { ui->statusBar->showMessage("Kill Job Fail");}
        delete Gcmd;
        Gcmd=NULL;
        }
        ui->statusBar->showMessage("Closing rcc_util");

        qDebug()<<"after job killed , wait job cancel";
        if (job_running)
        {
        //for(int i=1;i>0;i--)
        for(int i=20;i>0;i--)
        {

            QEventLoop eventloop2;
            QTimer::singleShot(1000,&eventloop2,SLOT(quit()));
            eventloop2.exec();
            qDebug()<<i;
            ui->statusBar->showMessage("Canceling Job,please wait "+QString::number(i,10)+" seconds");
        }
        }
        qDebug()<<"close rcc";
        system("./rcc_utilc sysctl exit");
        stop_connection=false;
        ui->pushButton_unshare->setEnabled(true);
        exit(1);
}


void MainWindow::share_board()
{
    stop_connection=true;
    version_check();
    bool share_board_check=true;
    bool arduino_online_check=true;

    QString check_minicom="sudo fuser "+serial_node;
    int check_minicom_result=system(check_minicom.toLatin1().data());
    if (check_minicom_result == 0)
    {
        QMessageBox::about(this,tr("Serial Warning"),QString("A serial tools such as minicom for "+serial_node+" is opened\n Please close it."));
        share_board_check=false;
    }else
    {
        QString check_adb="ltkcclient -b "+board_id_action;
        if ( system(check_adb.toLatin1().data()) != 0 )
        {
           board_connection_ok=false;
           adb_connect_status="(Null)";
        }else
        {
            adb_connect_status="(OK)";
        }


        if (!board_connection_ok)
        {

            //int debug=QMessageBox::warning(this,tr("Warning"),QString(" USB/Serial/Arduion Can't be detected.\n Please plug out/in these cable to your board!!!\n Or The test will limited as following: \n\n Serial\t"+serial_connect_stauts+"\t: Panic/hung detect\n Arduino\t"+rcc_connect_status+"\t: Reboot/Image burning control \n\n Do you want to continue sharing the board?"),QMessageBox::Yes,QMessageBox::No);
            int debug=QMessageBox::warning(this,tr("Warning"),QString(" USB/Serial/Arduion Can't be detected.\n Please plug out/in these cable to your board!!!\n Or The test will limited as following: \n USB\t"+adb_connect_status+"\t: Image Burning, Test Package Setup\n Serial\t"+serial_connect_stauts+"\t: Panic/hung detect\n Arduino\t"+rcc_connect_status+"\t: Reboot/Image burning control \n\n Do you want to continue sharing the board?"),QMessageBox::Yes,QMessageBox::No);
            if ( debug == QMessageBox::Yes )
            { share_board_check=true; }else
            { share_board_check=false;}

            /*
            QMessageBox::about(this,tr("Warning"),QString(" USB/Serial/Arduion Can't be detected.\n Please plug out/in these cable to your board!!!\n Or The test will limited as following: \n USB\t"+adb_connect_status+"\t: Image Burning, Test Package Setup\n Serial\t"+serial_connect_stauts+"\t: Panic/hung detect\n Arduino\t"+rcc_connect_status+"\t: Reboot/Image burning control"));
            share_board_check=false;
            */
        }
    }
    //Gcmd = new QProcess();
    //qDebug()<<__LINE__<<__FILE__<<Gcmd->isSequential();
    //waitProcess("./rcc_utilc operate "+board_id_action+" debug \"ol\"",300);
    //tempString=Gcmd->readAll();


    QProcess *cmd1=new QProcess();
    cmd1->start("./rcc_utilc operate "+board_id_action+" debug \"ol\"");
    while((cmd1->state() == QProcess::Running) || (cmd1->state() == QProcess::Starting) )
    {
        QEventLoop eventloop;
        QTimer::singleShot(100,&eventloop,SLOT(quit()));
        eventloop.exec();
        qDebug()<<"check arduino status"<<cmd1->state();
    }
    tempString=cmd1->readAll();
    delete cmd1;cmd1=NULL;


    if (tempString.indexOf("1001") >=0 || rcc_connect_status == "(Null)" || tempString.indexOf("10001") >=0 || tempString.indexOf("Invalid response") >=0 )
    {
        QString rcc_warning_mesg=" Arduion Can't be detected.\n Please plug out/in the cable between Arduion and board !!!\n ";
        if (tempString.indexOf("10001") >=0 || tempString.indexOf("Invalid response") >=0 )
        {rcc_warning_mesg=" Arduion Can't be detected.\n Please plug out/in the cable between Arduion and PC !!!\n ";}
        QMessageBox::about(this,tr("Warning"),rcc_warning_mesg);
        arduino_online_check=false;
    }
    qDebug()<<"check arudino status end";

    if (share_board_check && arduino_online_check )
    {
        if (!register_name_saved && !config_setted )
        { ask_for_user_name();
            if (register_name.indexOf("Unknow") >= 0 || register_team.indexOf("Unknow") >= 0 )
            {
                register_name_saved=false;
            }else
            {
                QMessageBox::about(this,tr("Your user info is saved "),tr("Your user and user team info is saved.\nIf you want to remove this info,\nyou can use \"sudo ltkcclient -r\" to init setting;\nor use \"sudo ltkcclient -i\" to re-install LTK cloud."));
            }
        }
        if ( ! register_name_saved && !config_setted  )
        {
            QMessageBox::about(this,tr("Warning"),tr(" please fill your register name/team"));
        }else
        {
            ui->pushButton_share->setDisabled(true);

            qDebug()<<"check register level";
            if ( ui->comboBox_register_level->currentIndex() == 0 )
                    register_level="share_level_3";
            if ( ui->comboBox_register_level->currentIndex() == 1 )
                    register_level="share_level_2";
            if ( ui->comboBox_register_level->currentIndex() == 2 )
                    register_level="share_level_1";



            QString share_cmd="./rcc_utilc ltkex "+board_id_action+" register "+"\""+register_name+";"+register_team+";"+register_level+"\"";
            //Gcmd = new QProcess();
            //waitProcess("./rcc_utilc ltkex "+board_id_action+" register "+"\""+register_name+";"+register_team+"\"",300);
            qDebug()<<ui->comboBox_register_level->currentIndex()<<"beging to share board"<<share_cmd;
            QProcess *cmd2=new QProcess();
            int cmd2_time_out=0;
            cmd2->start(share_cmd);
            while( ( (cmd2->state() == QProcess::Running) || (cmd2->state() == QProcess::Starting) ) && cmd2_time_out < 300 )
            {
                QEventLoop eventloop;
                QTimer::singleShot(100,&eventloop,SLOT(quit()));
                eventloop.exec();
                cmd2_time_out++;
                qDebug()<<cmd2->isSequential();
            }
            qDebug()<<"share result is"<<"exitStatus"<<cmd2->exitStatus()<<"exitCode"<<cmd2->exitCode()<<"state"<<cmd2->state();
            bool cmd2_result=true;
            if (cmd2->exitCode() !=0 || cmd2_time_out >= 300 )
            { cmd2_result=false;}
            delete cmd2;cmd2=NULL;


            if ( cmd2_result )
            {
                ui->statusBar->showMessage("Register Pass");
                lock_flag=true;
                //lock_button(false);
                lock_usage();
                QString config_file="./ltk_cloud_client_config.ini";
                QSettings settings(config_file,QSettings::IniFormat);
                if (ui->comboBox_branch_list->currentText() == "Null")
                {settings.setValue(board_id_action,"null;null;null");}
                else
                {settings.setValue(board_id_action,daily_image_map[ui->comboBox_branch_list->currentText()]);}

            }
            else
            {
                ui->statusBar->showMessage("Register Fail");
                qDebug()<<"Dump network status";
                QProcess *cmd3=new QProcess();
                cmd3->start("ltkcclient -e");
                while((cmd3->state() == QProcess::Running) || (cmd3->state() == QProcess::Starting) )
                {
                    QEventLoop eventloop;
                    QTimer::singleShot(100,&eventloop,SLOT(quit()));
                    eventloop.exec();
                    qDebug()<<"dump network status"<<cmd3->state();
                }
            }
            delete Gcmd;Gcmd=NULL;
            show_board_detail(board_id_action);
            select_history=ui->listWidget_list->currentRow();
            list_board();
        }
    }
    stop_connection=false;
}

void MainWindow::unshare_board()
{
    stop_connection=true;
    ui->listWidget_list->setDisabled(true);
    bool allow_unshare=true;
    QProcess *cmd1=new QProcess();
    qDebug()<<"unshare_board:"<< board_id_action;
    cmd1->start("./rcc_utilc query "+board_id_action+" jobStatus");
    while((cmd1->state() == QProcess::Running) || (cmd1->state() == QProcess::Starting) )
    {
        QEventLoop eventloop;
        QTimer::singleShot(100,&eventloop,SLOT(quit()));
        eventloop.exec();
        qDebug()<<"list job status"<<cmd1->state();
    }
    tempString=cmd1->readAll();

    if ( tempString.indexOf("No job running") < 0 )
    {
        int answer=QMessageBox::warning(this,tr("Warning"),tr("The board is running job !!! \nSystem will take about 5mins to cancel job\nDo you want to continue unshare the board ?"),QMessageBox::Yes,QMessageBox::No);
        if ( answer == QMessageBox::Yes )
        {
            allow_unshare=true;
            ui->statusBar->showMessage("Job Canceling");
        }else
        { allow_unshare=false;}
    }
    if (allow_unshare)
    {
    ui->pushButton_unshare->setDisabled(true);
    stop_connection=true;

    ui->statusBar->showMessage("Job Canceling");





    /*
    Gcmd = new QProcess();
    connect(Gcmd,SIGNAL(readyRead()),this,SLOT(readOutput_byte()));
    waitProcess("./rcc_utilc ltkex "+board_id_action+" killJob",10000);
    if ( progress_result )
    {
        ui->statusBar->showMessage("Kill Job Success");
    }
    else
    { ui->statusBar->showMessage("Kill Job Fail");}
    delete Gcmd;
    Gcmd=NULL;
    */

    QProcess *cmd1=new QProcess();
    int time_out=10000;
    int time_count=0;
    cmd1->start("./rcc_utilc ltkex "+board_id_action+" killJob");
    while( ((cmd1->state() == QProcess::Running) || (cmd1->state() == QProcess::Starting)) && ( time_count < time_out) )
    {
        QEventLoop eventloop;
        QTimer::singleShot(100,&eventloop,SLOT(quit()));
        eventloop.exec();
        qDebug()<<"kill jobing"<<cmd1->state();
        time_count++;
    }
    delete cmd1;cmd1=NULL;


    Gcmd = new QProcess();
    qDebug()<<__LINE__<<__FILE__<<Gcmd->isSequential();
    connect(Gcmd,SIGNAL(readyRead()),this,SLOT(readOutput_byte()));
    waitProcess("./rcc_utilc ltkex "+board_id_action+" unregister",300);
    qDebug()<<tempString;
    if ( progress_result )
    {
        ui->statusBar->showMessage("Unregister Pass");
        lock_flag=false;
        delete Gcmd;
        Gcmd=NULL;
        lock_usage();
    }
    else
    { ui->statusBar->showMessage("Unregister Fail");}
    delete Gcmd;
    Gcmd=NULL;
    show_board_detail(board_id_action);
    select_history=ui->listWidget_list->currentRow();
    list_board();
    stop_connection=false;
    }
    //load_blf_map();
    stop_connection=false;
    ui->listWidget_list->setDisabled(false);
}





void MainWindow::onSystemTrayIconClicked(QSystemTrayIcon::ActivationReason reason)

{

  switch(reason)
{
  //单击
  case QSystemTrayIcon::Trigger:
  //双击
  case QSystemTrayIcon::DoubleClick:
      //恢复窗口显示
       list_board();
       closed_to_tool_bar=false;
      this->setWindowState(Qt::WindowActive);
      this->show();
trayicon->hide();
      break;
  default:
      break;
  }

}


void MainWindow::closeEvent(QCloseEvent *event)
{
    /*
    qDebug()<<"quit_check";
    if ( closed_to_tool_bar )
    {
        event->accept();
    }else
      {
          if ( ubuntu_13 )
          {
              qDebug()<<"event->accept()";
              event->accept();
          }
          else
          {
              hide();
              event->ignore();
              qDebug()<<"hide to icon";
              closed_to_tool_bar=true;
              trayicon->show();
          }
      }
      */

    bool allow_close=true;
    bool job_running=false;
    QStringList kill_board_list;
    for (int i=0;i<board_list.count();i++)
    {
        QProcess *cmd1=new QProcess();
        cmd1->start("./rcc_utilc query "+board_list.at(i)+" jobStatus");
        int event_loop=0;
        while(((cmd1->state() == QProcess::Running) || (cmd1->state() == QProcess::Starting)) && event_loop<100 )
        {
            QEventLoop eventloop;
            QTimer::singleShot(100,&eventloop,SLOT(quit()));
            eventloop.exec();
            event_loop++;
            qDebug()<<"list job status"<<cmd1->state()<<event_loop;
        }
        tempString=cmd1->readAll();
            if ( tempString.indexOf("No job running") < 0 )
            {
                job_running=true;
                kill_board_list<<board_list.at(i);
            }
    }

    if ( job_running )
    {
        int answer=QMessageBox::warning(this,tr("Warning"),tr("There are job running on board!!! \nSystem will take about 5mins to cancel job\nDo you want to continue close client ?"),QMessageBox::Yes,QMessageBox::No);
        if ( answer == QMessageBox::Yes )
        {
            ui->statusBar->showMessage("Job Canceling");
        }else
        { allow_close=false;}
    }


    if ( ! allow_close )
    {
        event->ignore();
    }else
    {
        ui->pushButton_unshare->setEnabled(false);
        stop_connection=true;
        ui->statusBar->showMessage("Job Canceling");
        for (int i=0;i<kill_board_list.count();i++)
        {
        Gcmd = new QProcess();
        connect(Gcmd,SIGNAL(readyRead()),this,SLOT(readOutput_byte()));
        waitProcess("./rcc_utilc ltkex "+board_list.at(i)+" killJob",10000);
        if ( progress_result )
        {
            ui->statusBar->showMessage("Kill Job Success");
        }
        else
        { ui->statusBar->showMessage("Kill Job Fail");}
        delete Gcmd;
        Gcmd=NULL;
        }
        ui->statusBar->showMessage("Closing rcc_util");

        qDebug()<<"after job killed , wait job cancel";
        if (job_running)
        {
        //for(int i=1;i>0;i--)
        for(int i=20;i>0;i--)
        {

            QEventLoop eventloop2;
            QTimer::singleShot(1000,&eventloop2,SLOT(quit()));
            eventloop2.exec();
            qDebug()<<i;
            ui->statusBar->showMessage("Canceling Job,please wait "+QString::number(i,10)+" seconds");
        }
        }
        qDebug()<<"close rcc";
        system("./rcc_utilc sysctl exit");
        stop_connection=false;
        ui->pushButton_unshare->setEnabled(true);
    }
}

void MainWindow::show_map(QString map_key)

{
    qDebug()<<map_key<<daily_image_map[map_key];

 }


void MainWindow::readOutput_byte()
{
    //qDebug()<<"AAAAAAAA";
    QCoreApplication::processEvents();
    //QString tempString = Gcmd->readAllStandardOutput();
    tempString = Gcmd->readAllStandardOutput();
    Goutput += tempString;
    //tempString= tempString.replace("\n","");
    //    ui->loginfo->append(tempString);
    //ui->textEdit->moveCursor(QTextCursor::End,QTextCursor::MoveAnchor);
   // ui->textEdit->insertPlainText(tempString);

    //ui->lineEdit_board_type->setText(tempString);

}

QProcess::ExitStatus MainWindow::waitProcess(const QString command,const int timeout)
{
    if ( ! progress_lock )
    {
        progress_lock=true;
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
        QTimer::singleShot(100,&eventloop,SLOT(quit()));
        eventloop.exec();
        qDebug()<<__LINE__<<__FILE__<<command<<" has Not finished,times="<<times;
        if ( times%10 == 0)
        {
            QString execution_mesg="execute rcc for ";
            execution_mesg+=QString::number(times/10,10)+" seconds";
            ui->statusBar->showMessage(execution_mesg);
        }

    }
    lock_free();
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
        progress_lock=false;
        return QProcess::CrashExit;
    }
    else if(getGstopped() == true)
    {
        qDebug()<<__LINE__<<__FILE__<<"StoppedExit";
        progress_lock=false;
        return QProcess::CrashExit;
    }
    else{
        qDebug()<<__LINE__<<__FILE__<<"NormalExit";
        progress_lock=false;
        return QProcess::NormalExit;
    }
}
    qDebug()<<"progress_lock"<<progress_lock;
}

void MainWindow::progress_wait_protect()
{
    int time=0;
    while (get_lock_status())
    {
        QEventLoop eventloop;
        QTimer::singleShot(100,&eventloop,SLOT(quit()));
        eventloop.exec();
        qDebug()<<"wait process for "<<time<<get_lock_status();
        time++;
        //progress_lock=NULL;
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
        lp_s=ddr_ori_str.mid(5,2);
        lp_t=ddr_type_map[lp_s];
        //type_s=ddr_ori_str.mid(0,3);
        //type_t=ddr_type_map[type_s];
        ddr_ori_str_transformed=ddr_ori_str.mid(0,5)+"@"+lp_t;
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
            ddr_ori_str_transformed=ddr_ori_str;
        }

    }
    qDebug()<<"DDR type transform"<<ddr_ori_str_transformed;
    return ddr_ori_str_transformed;

}

mylineedit::mylineedit(QWidget *parent) :
    QLineEdit(parent)
{
}

//重写mousePressEvent事件,检测事件类型是不是点击了鼠标左键
void mylineedit::mousePressEvent(QMouseEvent *event) {
    //如果单击了就触发clicked信号
    if (event->button() == Qt::LeftButton) {
        //触发clicked信号
        emit clicked();
    }
    //将该事件传给父类处理
    QLineEdit::mousePressEvent(event);
}
