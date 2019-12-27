#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QClipboard>


QString StringList_SSP::at_sp(int a)
{
    QStringList* list=this;
    QString unknow_str="unknow_str";
    if ( a < list->count() )
    {
        return this->at(a);
    }else
    {
        return unknow_str;
    }
}
StringList_SSP::StringList_SSP(QStringList list)
{
    for (int i=0;i<list.count();i++)
    {this->operator <<(list.at(i));}
}



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QDir current_dir("./");
    work_dir= current_dir.absolutePath();


    receiver = new QUdpSocket(this);
    bool bind_result=receiver->bind(QHostAddress::LocalHost, 6788);
    debug_mode=false;
    if ( !bind_result )
    {
        QMessageBox::about(this, tr("Warning"),
                    tr("<h2>Socket bind error</h2>"
                       "<p>You may have already open the GUI!."));
        int debug=QMessageBox::warning(this,tr("Warning"),tr("Socket bind error \n You may have already open the GUI!\n Do you want to enter Debug Mode?"),QMessageBox::Yes,QMessageBox::No);
                if ( debug == QMessageBox::Yes )
        { debug_mode=true; }else
        { debug_mode=false;}
    }
    //connect(receiver, SIGNAL(readyRead()),this, SLOT(readPendingDatagrams()));
    connect(receiver, SIGNAL(readyRead()),this, SLOT(readPendingDatagrams_delay_key()));
    init_windows();
}


void MainWindow::readPendingDatagrams_delay_key()
{
    qDebug()<<"readPendingDatagrams";


    QByteArray datagram;
    datagram.resize(receiver->pendingDatagramSize());
    receiver->readDatagram(datagram.data(), datagram.size());
    gui_msg_t *read=(gui_msg_t *)datagram.data();
    qDebug()<<read->op<<read->magic<<read->param;
    QString read_mesg=read->param;
    if ( read->magic == 0x12345678 )
    {
        if ( read->op == 0 )
        {
            qDebug()<<"do nothing";
        }
        if ( read->op == 1 && !stop_auto_update_job_status->isChecked() )
        {
            qDebug()<<"update status";
            update_status();
        }
        if ( read->op == 2 && ! stop_socket_connection )
        {
            qDebug()<<"reflase board list";
            clear_config();
        }
        if ( read->op == 3 )
        {
            qDebug()<<"board add:"<<read_mesg;
            if (read_mesg.split(";").count() > 1)
            {
            add_delay_task=read_mesg.split(";").at(0);
            add_delay_board=read_mesg.split(";").at(1);
            add_delay_tree_tab();
            }

        }
        if ( read->op == 4 )
        {
            QString task=read_mesg.split(";").at(0);
            QString board=read_mesg.split(";").at(1);
            int value=read_mesg.split(";").at(2).toInt();
            update_progress_bar(task,board,value);
        }
    }

    while (receiver->hasPendingDatagrams())
    {
    QByteArray datagram;
    datagram.resize(receiver->pendingDatagramSize());
    receiver->readDatagram(datagram.data(), datagram.size());
    gui_msg_t *read=(gui_msg_t *)datagram.data();
    qDebug()<<read->op<<read->magic<<read->param;
    QString read_mesg=read->param;
    if ( read->magic == 0x12345678 )
    {
        if ( read->op == 3 )
        {
            qDebug()<<"board add:"<<read_mesg;
            if (read_mesg.split(";").count() > 1)
            {
            add_delay_task=read_mesg.split(";").at(0);
            add_delay_board=read_mesg.split(";").at(1);
            add_delay_tree_tab();
            }
        }
        else if ( read->op == 1 && !stop_auto_update_job_status->isChecked() )
        {
            qDebug()<<"update pending status";
            update_status();
        }
        else
        {
            qDebug()<<"udp pending,skip:"<<read->op<<read->param;
        }
    }
    }

}




void MainWindow::readPendingDatagrams()
{
    qDebug()<<"readPendingDatagrams";
    while (receiver->hasPendingDatagrams())
    {
    QByteArray datagram;
    datagram.resize(receiver->pendingDatagramSize());
    qDebug()<<"udp1"<<receiver->hasPendingDatagrams();
    receiver->readDatagram(datagram.data(), datagram.size());
    qDebug()<<"udp2"<<receiver->hasPendingDatagrams();
    gui_msg_t *read=(gui_msg_t *)datagram.data();
    qDebug()<<read->op<<read->magic<<read->param;
    QString read_mesg=read->param;
    bool udp_busy=receiver->hasPendingDatagrams();
    if ( read->magic == 0x12345678 )
    {
        if ( read->op == 0 )
        {
            qDebug()<<"do nothing";
        }
        if ( read->op == 1 && !udp_busy )
        {
            qDebug()<<"update status";
            update_status();
        }
        if ( read->op == 2 && ! stop_socket_connection && !udp_busy)
        {
            qDebug()<<"reflase board list";
            clear_config();
        }
        if ( read->op == 3 )
        {
            qDebug()<<"board add:"<<read_mesg;
            if (read_mesg.split(";").count() > 1)
            {
            add_delay_task=read_mesg.split(";").at(0);
            add_delay_board=read_mesg.split(";").at(1);
            add_delay_tree_tab();
            }

        }
        if ( read->op == 4 )
        {
            QString task=read_mesg.split(";").at(0);
            QString board=read_mesg.split(";").at(1);
            int value=read_mesg.split(";").at(2).toInt();
            update_progress_bar(task,board,value);
        }
    }
    }

}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::confirm_setting()
{
    check_tree();
    check_board_list();
    generate_config();
    start_eable(true);
}

void MainWindow::case_select_all(QTreeWidgetItem* before_select,QTreeWidgetItem* after_select)
{
    qDebug()<<before_select->text(1);

    if (before_select->text(1).isEmpty())
    {
        qDebug()<<"select all";
        /*
        if (after_select->checkState(0))
        {
            for (int i=0;i<after_select->childCount();i++)
            {
                after_select->child(i)->setCheckState(0,Qt::Checked);
            }
        }else
        {
            for (int i=0;i<after_select->childCount();i++)
            {
                after_select->child(i)->setCheckState(0,Qt::Unchecked);
            }
        }
        */
    }
}

void MainWindow::add_delay_tree_tab()
{
    qDebug()<<"add delay setting for tree"<<add_delay_task<<add_delay_board;
    QStringList add_case_list=delay_task_case_id[add_delay_task];

    if (task_map.find(add_delay_task+"-B-"+add_delay_board) != task_map.end())
    {qDebug()<<"task-board has existed";}
    else{
    QTreeWidgetItem* task;
    if (task_map.find(add_delay_task) != task_map.end() )
    {
        qDebug()<<"find delay task in tree map";
                QTreeWidgetItemIterator check_tree(task_tree);
                while (*check_tree)
                {

                    if ( (*check_tree)->text(0) == add_delay_task )
                    {
                        qDebug()<<"finded task tree";
                        task=*check_tree;
                    }
                    ++check_tree;
                }
                task->setText(1,"running");
    QTreeWidgetItem* board=new QTreeWidgetItem;
    board->setText(0,add_delay_board);

    board->setText(1,"running");
    board->setText(2,"job_for:"+add_delay_task+":"+task_board_entry[add_delay_task]);
    task_map[add_delay_task+"-B-"+add_delay_board]=board;
    for (int a=0;a<add_case_list.count();a++)
    {
        QTreeWidgetItem* leaf= new QTreeWidgetItem;
        leaf->setText(0,add_case_list.at(a));
        leaf->setText(2,"smb://"+test_package_server_ip+"/daily_build/Cloud_Test/"+task_date+"/"+task_name);
        task_map[add_delay_task+"-B-"+add_delay_board+"-C-"+add_case_list.at(a)]=leaf;
        board->addChild(leaf);
    }
    task->addChild(board);
    board->setExpanded(true);
    task->setExpanded(true);
    }else{qDebug()<<"rev delay teak not in task";}



qDebug()<<"add delay setting for tab";

       QTableWidget* task_tab;
        int row_begin=0;
        if (task_tab_map.find(task_name) != task_tab_map.end())
        {
            task_tab=task_tab_map[task_name];
            row_begin=task_tab->rowCount();
            task_tab->setRowCount(task_tab->rowCount()+add_case_list.count());

     for (int a=0;a<add_case_list.count();a++)
            {
                task_tab->setRowHeight(a+row_begin,20);
                task_tab->setItem(a+row_begin,2,new QTableWidgetItem(add_case_list.at(a)));
                task_tab->setItem(a+row_begin,0,new QTableWidgetItem(add_delay_task));
                task_tab->setItem(a+row_begin,1,new QTableWidgetItem(add_delay_board));
                staf_task_case_map_tab[add_delay_task+"-B-"+add_delay_board+"-C-"+add_case_list.at(a)]=new QTableWidgetItem("Submitted");
                staf_task_log_map_tab[add_delay_task+"-B-"+add_delay_board+"-C-"+add_case_list.at(a)]=new QTableWidgetItem("...");
                task_tab->setItem(a+row_begin,3,staf_task_case_map_tab[add_delay_task+"-B-"+add_delay_board+"-C-"+add_case_list.at(a)]);
                task_tab->setItem(a+row_begin,4,staf_task_log_map_tab[add_delay_task+"-B-"+add_delay_board+"-C-"+add_case_list.at(a)]);
                task_tab->setItem(a+row_begin,7,new QTableWidgetItem(delay_task_setting[add_delay_task].at(0)));
                task_tab->setItem(a+row_begin,8,new QTableWidgetItem(delay_task_setting[add_delay_task].at(1)));
            }
        }else{
            qDebug()<<"rev delay teak not in task";
        }
    }

}



void MainWindow::generate_config()
{
    qDebug()<<"generate config";
    if (delay_task)
    {
        QTreeWidgetItem* task;
        task=new QTreeWidgetItem(task_tree);
        task->setText(0,task_name);
        task->setText(1,"running");
        task->setText(2,"delay_task:"+delay_setting);
        task_map[task_name]=task;
    }else
    {
        QTreeWidgetItem* task;
        if (task_map.find(task_name) != task_map.end() )
        {

                    QTreeWidgetItemIterator check_tree(task_tree);
                    while (*check_tree)
                    {

                        if ( (*check_tree)->text(0) == task_name )
                        {
                            qDebug()<<"finded task tree";
                            task=*check_tree;
                        }
                        ++check_tree;
                    }
                    task->setText(0,task_name);
                    task->setText(1,"running");
                    task->setText(2,"normal_task");

        }else
        {
        task=new QTreeWidgetItem(task_tree);
        task->setText(0,task_name);
        task->setText(1,"running");
        task->setText(2,"normal_task:normal");
        task_map[task_name]=task;
        }
        for (int i=0; i<board_list.count();i++)
        {
            QTreeWidgetItem* board=new QTreeWidgetItem;
            board->setText(0,board_list.at(i));

            board->setText(1,"running");
            //QString job_entry="normal_job_for:"+task_board_entry[task_name+"-B-"+board_list.at(i)];
            //qDebug()<<"normal_job_for:"<<task_board_entry[task_name+"-B-"+board_list.at(i)]<<task_name<<board_list.at(i);
            //board->setText(2,job_entry);
            task_map[task_name+"-B-"+board_list.at(i)]=board;
            for (int a=0;a<case_list.count();a++)
            {
                QTreeWidgetItem* leaf= new QTreeWidgetItem;
                QString case_id=case_list.at(a);
                case_id=case_id.remove(QRegExp("-loop-.*"));
                leaf->setText(0,case_id);
                leaf->setText(2,"smb://"+test_package_server_ip+"/daily_build/Cloud_Test/"+task_date+"/"+task_name);
                task_map[task_name+"-B-"+board_list.at(i)+"-C-"+case_id]=leaf;
                //leaf->setText(1,status);
                board->addChild(leaf);
                qDebug()<<"ddddd"<<task_name+board_list.at(i)+case_list.at(a);

            }
            task->addChild(board);
            board->setExpanded(true);
            task_progress[task_name+"-B-"+board_list.at(i)]=new QProgressBar;
            task_tree->setItemWidget(board,3,task_progress[task_name+"-B-"+board_list.at(i)]);
            task_progress[task_name+"-B-"+board_list.at(i)]->setMaximum(100);
            task_progress[task_name+"-B-"+board_list.at(i)]->setValue(1);
            task_progress[task_name+"-B-"+board_list.at(i)]->setMaximumSize(30,15);
            task_progress[task_name+"-B-"+board_list.at(i)]->setTextVisible(true);
            //QProgressBar* dd=new QProgressBar;
            //task_tree->setItemWidget(board,3,dd);
        }


        task->setExpanded(true);
        //start_config->setText(table_string);
        board_map.clear();
        for ( int i=0;i<board_list.count();i++ )
        {
            //QTextEdit* board_config =new QTextEdit;
            //toolBox->addItem(board_config,board_list.at(i));
            board_map[board_list.at(i)]=i+1;
            //toolBox->setItemText(i,"board1");
        }
        qDebug()<<"add configggg";
}


}

void MainWindow::update_progress_bar(QString progress_task_name, QString progress_board_name,int progress_value)
{

    //update_result = new QTimer;
    //connect(update_result,SIGNAL(timeout()),this,SLOT(time_update()));
    //update_result->start(2000);
    //update_result->stop();

    task_progress[progress_task_name+"-B-"+progress_board_name]->setValue(progress_value);
}

void MainWindow::generate_tab()
{
    qDebug()<<"generate_tab()";
    if (delay_task)
    {
        QTableWidget* task_tab;
        task_tab_map[task_name]=new QTableWidget;
        task_tab=task_tab_map[task_name];
        //QIcon icon1("CheckboxFull.png");
        //ui->tabWidget->addTab(task_tab,icon1,task_name);
        ui->tabWidget->addTab(task_tab,task_name);
        task_tab->setColumnCount(9);
        QStringList task_tab_title;
        task_tab_title<<"Task"<<"Board"<<"Case ID"<<"Status"<<"Log link"<<"Start Time"<<"End Time"<<"Image Version"<<"LTK Version";
        task_tab->setHorizontalHeaderLabels(task_tab_title);
        task_tab->setRowCount(0);
        task_tab->setColumnWidth(4,300);
        task_tab->setColumnWidth(0,150);
        task_tab->setColumnWidth(1,160);
        task_tab->setColumnWidth(2,100);
        task_tab->setColumnWidth(3,100);
        task_tab->horizontalHeader()->setFont(QFont("simsun",9));
        task_tab->setFont(QFont("simsun",9));
        connect(task_tab,SIGNAL(itemDoubleClicked(QTableWidgetItem*)),this,SLOT(open_tab_log_link(QTableWidgetItem*)));
    }else
    {
    QTableWidget* task_tab;
    int row_begin=0;
    if (task_tab_map.find(task_name) != task_tab_map.end())
    {
        task_tab=task_tab_map[task_name];
        row_begin=task_tab->rowCount();
        task_tab->setRowCount(task_tab->rowCount()+(board_list.count()*case_list.count()));

    }else
    {
        task_tab_map[task_name]=new QTableWidget;
        task_tab=task_tab_map[task_name];
        //QIcon icon1("CheckboxFull.png");
        //ui->tabWidget->addTab(task_tab,icon1,task_name);
        ui->tabWidget->addTab(task_tab,task_name);
        task_tab->setColumnCount(9);
        QStringList task_tab_title;
        task_tab_title<<"Task"<<"Board"<<"Case ID"<<"Status"<<"Log link"<<"Start Time"<<"End Time"<<"Image Version"<<"LTK Version";
        task_tab->setHorizontalHeaderLabels(task_tab_title);
        task_tab->setRowCount((board_list.count()*case_list.count()));
        task_tab->setColumnWidth(4,300);
        task_tab->setColumnWidth(0,150);
        task_tab->setColumnWidth(1,160);
        task_tab->setColumnWidth(2,100);
        task_tab->setColumnWidth(3,100);
        task_tab->horizontalHeader()->setFont(QFont("simsun",9));
        task_tab->setFont(QFont("simsun",9));
        connect(task_tab,SIGNAL(itemDoubleClicked(QTableWidgetItem*)),this,SLOT(open_tab_log_link(QTableWidgetItem*)));
    }

    for (int e=0;e < task_tab->rowCount() ; e++)
    {
        task_tab->setRowHeight(e,20);
    }

    qDebug()<<"row_begin:"<<row_begin<<task_tab->rowCount()<<(board_list.count()*case_list.count());

    for (int i=0;i<board_list.count();i++)
    {
        for (int a=0;a<case_list.count();a++)
        {
            task_tab->setRowHeight((i*case_list.count()),20);
            task_tab->setItem((i*case_list.count())+a+row_begin,2,new QTableWidgetItem(case_list.at(a)));
            task_tab->setItem((i*case_list.count())+a+row_begin,0,new QTableWidgetItem(task_name));
            task_tab->setItem((i*case_list.count())+a+row_begin,1,new QTableWidgetItem(board_list.at(i)));
            staf_task_case_map_tab[task_name+"-B-"+board_list.at(i)+"-C-"+case_list.at(a)]=new QTableWidgetItem("Submitted");
            staf_task_log_map_tab[task_name+"-B-"+board_list.at(i)+"-C-"+case_list.at(a)]=new QTableWidgetItem("...");
            task_tab->setItem((i*case_list.count())+a+row_begin,3,staf_task_case_map_tab[task_name+"-B-"+board_list.at(i)+"-C-"+case_list.at(a)]);
            task_tab->setItem((i*case_list.count())+a+row_begin,4,staf_task_log_map_tab[task_name+"-B-"+board_list.at(i)+"-C-"+case_list.at(a)]);
            //staf_task_log_map_tab[task_name+"-B-"+board_list.at(i)+"-C-"+case_list.at(a)]->setTextColor(QColor("white"));
            if ( ui->checkBox_last_image->isChecked() )
            {
                if ( image_modify == "yes" )
                {
                    task_tab->setItem((i*case_list.count())+a+row_begin,7,new QTableWidgetItem("Special Image:"+image_version+"_"+image_def+"_"+image_burning_blf+"_"+replaced_image));
                }else
                { task_tab->setItem((i*case_list.count())+a+row_begin,7,new QTableWidgetItem(image_version+"_"+image_def+"_"+image_burning_blf)); }

            }else
            {
                task_tab->setItem((i*case_list.count())+a+row_begin,7,new QTableWidgetItem("N/A"));
            }
            task_tab->setItem((i*case_list.count())+a+row_begin,8,new QTableWidgetItem(ltk_version+"_"+ltk_branch));
        }
    }
    qDebug()<<"tab index"<< ui->tabWidget->indexOf(task_tab_map[task_name]);
    }

}


void MainWindow::open_tab_log_link(QTableWidgetItem *link_item)
{
    if ( link_item->text().indexOf("smb://") >= 0 )
    {
        QString url=link_item->text();
        QUrl open_url(url,QUrl::TolerantMode);
        bool url_open_success=QDesktopServices::openUrl(open_url);
        if ( ! url_open_success )
        {
            QMessageBox::about(this, tr("Warning"),
                        tr("<h2>Log link open fail</h2>"
                           "<p>Log link open fail!."));
        }
    }
}


void MainWindow::close_tab()
{
    ui->tabWidget->removeTab(ui->tabWidget->currentIndex());
}

void MainWindow::show_image_config(){
    if (ui->checkBox_last_image->isChecked())
    {
        toolBox->setCurrentIndex(3);
        ltk_image_tree->setDisabled(false);

    }else
    {ltk_image_tree->setDisabled(true);}
}




void MainWindow::check_delay_task_for_blf()
{
QString tree_item;

    qDebug()<<"check_delay_task_for_blf";
    tree_item=ltk_image_tree->currentItem()->text(1);

if ( tree_item.indexOf("platform_def") < 0 )
{
    warning_mesg="WARNING: For Delay task, you should choose a image by platform def";
    set_success=false;
}
else
{
    qDebug()<<"get_delay_blf"<<tree_item;
    QStringList image_config=tree_item.split("---");
    qDebug()<<image_config.at(1)<<image_config.at(1).split("++").at(0);
    image_version=image_config.at(1).split("++").at(0);
    QString image_version_date=image_version.split("_").at(0);
    image_def=image_config.at(1).split("++").at(1).split(":").at(0);
    //if ( image_version_date > "2014-05-23" && ui->comboBox_platform->currentText() == "EDEN" )
    //{
        image_def="\""+image_def+"/flash\"";
    //}
    image_burning_blf="\""+image_config.at(1).split("++").at(1).split(":").at(1)+"\"";
}
}



void MainWindow::check_package_image()
{
    set_success=true;
    qDebug()<<"check_package_image";
    image_platform=platform_map[ui->comboBox_platform->currentText()];


    QString tree_item;
    qDebug()<<"get image config";
    if ( ! ltk_image_tree->topLevelItem(0)->child(0)->child(0)->isSelected())
    {
        qDebug()<<"get image config1";
        tree_item=ltk_image_tree->currentItem()->text(1);
    }else
    {
        qDebug()<<"get image config2";
        tree_item=ltk_image_tree->topLevelItem(0)->child(0)->child(0)->text(1);
    }
    QStringList image_config=tree_item.split(";");

        if ( tree_item.indexOf("version") >= 0 && ui->checkBox_last_image->isChecked() )
        {
            warning_mesg="WARNING: Please select the special image!";
            set_success=false;
        }else
        {

            qDebug()<<"GGGGGGGGGGGGGG"<<image_platform;
            if ( image_platform=="pxa1908" && ( tree_item.indexOf("pxa1936-lp5.0_k314_alpha2") >= 0 || tree_item.indexOf("pxa1936-lp5.0_k314_beta1")  >= 0 || tree_item.indexOf("pxa1936-lp5.1") >= 0) )
            {
                image_platform="pxa1936";
            }
            qDebug()<<"GGGGGGGGGGGGGG"<<image_platform<<tree_item;
            if ( tree_item.indexOf("platform_def") >= 0 )
            {
                QStringList image_config=tree_item.split("---");
                qDebug()<<image_config.at(1)<<image_config.at(1).split("++").at(0);
                image_version=image_config.at(1).split("++").at(0);
                QString image_version_date=image_version.split("_").at(0);
                image_def=image_config.at(1).split("++").at(1).split(":").at(0);
                //if ( image_version_date > "2014-05-23" && ui->comboBox_platform->currentText() == "EDEN" )
                //{
                    image_def="\""+image_def+"/flash\"";
                //}
                image_burning_blf="\""+image_config.at(1).split("++").at(1).split(":").at(1)+"\"";
            }else
            {
                image_version=image_config.at(0);
                image_def=image_config.at(1);

                QStringList image_link_item_list=tree_item.split(";");
                link_for_blf="http://10.38.116.40/autobuild/android/"+image_platform+"/"+image_link_item_list.at(0)+"/"+image_link_item_list.at(1)+"/blf/"+image_link_item_list.at(2);
                //QString download_cmd="wget -t 3 -w 1 --http-user=wsun --http-passwd=marvell888 "+link_for_blf;
                //int try_down_load=system(download_cmd.toLatin1().data());
                //if (try_down_load != 0)
                //{
                    image_def="\""+image_config.at(1)+"/flash\"";
                //}
                QFile::remove(image_link_item_list.at(2));


                image_burning_blf=image_config.at(2);
            }
        }


    get_delay_setting();
   // if (ui->checkBox_dealy_task->isChecked() && ui->checkBox_last_image->isChecked() )
    //{ check_delay_task_for_blf();}

    if (ui->checkBox_last_image->isChecked())
    {
        update_image="yes";
        image_burn_config=" General update_image=yes ImageConfig platform="+image_platform+" ImageConfig version="+image_version+" ImageConfig image="+image_def+" ImageConfig blf="+image_burning_blf+" ImageConfig eraseall=no";
        if (image_modify == "yes" )
        {
            image_burn_config+=" ImageConfig modified=yes ImageConfig location2="+special_image_ip+" ImageConfig path2="+specail_image_path+" ImageConfig image2=\""+replaced_image+"\" ";
        }
        if (blf_modified == "yes")
        {
             image_burn_config+=" ImageConfig blf_modified=yes ";
             blf_modified_name=blf_modified_name_list.join(";");
             image_burn_config+=" ImageConfig blf_modified_name=\""+blf_modified_name+"\" ";
             image_burn_config+=" ImageConfig blf_modified_path=\""+blf_modified_path+"\" ";
        }else
        {
            image_burn_config+=" ImageConfig blf_modified=no ";
        }

    }else
    {
        update_image="no";
        image_burn_config="";
    }



    if ( selected_case.isEmpty() && ui->checkBox_last_package->isChecked() )
    {
        warning_mesg="WARNING: Please select a LTK test package";
        set_success=false;
    }else
    {
    QTreeWidgetItem* temp;
    qDebug()<<"get pakcage config";
    if ( selected_package_item->isSelected() )
    {
        temp=selected_package_item;
    }else
    {
        temp=ltk_package_tree->currentItem();    
    }

    if ( (temp->text(1).indexOf("package_by_day") >= 0 ) && (!ui->checkBox_dealy_task->isChecked()) )
    {
        warning_mesg="WARNING: Please select a special image package!";
        set_success=false;
    }else
    {
        ltk_version=temp->text(1);
        ltk_branch=temp->text(0);
        ltk_config=" LTKConfig version="+ltk_version+" LTKConfig branch="+ltk_branch;//" LTKConfig location="+test_package_server_ip+" LogConfig location="+test_package_server_ip;
    }
    }

    if ( ui->checkBox_auto_emmd->isChecked() )
    {
        ltk_config+=" General auto_emmd=yes ";
    }else{ltk_config+=" General auto_emmd=no ";}
    if ( ui->checkBox_auto_reboot->isChecked() )
    {
        ltk_config+=" General auto_reboot=yes ";
    }else{ltk_config+=" General auto_reboot=no ";}

    if ( ui->checkBox_console_log->isChecked() )
    {
        ltk_config+=" General logcat_cap=yes  General console_cap=yes ";
    }else{ltk_config+=" General logcat_cap=no  General console_cap=no ";}
     ltk_config+=" General log_autoupload=yes ";
     QString runcase_mode=ui->comboBox_runcase_mode->currentText();
     if (runcase_mode != "policy" && ui->checkBox_runcase_mode->isChecked() )
     {
         ltk_config+=" General runcase_mode="+runcase_mode+" ";
     }



     add_ex_setting();
     add_sp_setting();

}

void MainWindow::check_runcase_mode(bool runcase_mode_chosed)
{
    ui->comboBox_runcase_mode->setEnabled(runcase_mode_chosed);
}


void MainWindow::start_test()
{     if ( ! progress_lock ){
        ui->pushButton_start->setDisabled(true);
        task_name=ui->lineEdit_task_name->text();
        check_package_image();
        check_tree();
        if (!delay_task)
        {check_board_list();}


        QDate date=QDate::currentDate();
        task_date=date.toString("yyyy-MM-dd");
        QString log_folder_config="LogConfig path=\"/Daily_Build/Cloud_Test/"+task_date+"\"";
        QString strDate = date.toString("-yyMMdd-");
        QTime time=QTime::currentTime();
        strDate = strDate+time.toString("hhmm");

        if (ui->checkBox_add_time_flag->isChecked())
        {
            task_name=task_name+strDate;
        }

        if (task_name.isEmpty())
        {
            warning_mesg="WARNING: Please Fill the task name";
            set_success=false;
        }
        QRegExp sp_word("[\*\[\]\:]");
        //if ( task_name.indexOf("\"") >=0 || task_name.indexOf("\*") >=0 || task_name.indexOf("\:") >=0  || task_name.indexOf("\[") >=0 || task_name.indexOf("\]") >=0 )
        if ( task_name.indexOf(QRegExp("[\:\*\?\<\>\"\'\|]")) >=0  || task_name.indexOf("\[") >=0 || task_name.indexOf("\]") >=0 )
        {
            warning_mesg="WARNING: Task name can't include :/\*?\<>\"[]| ";
            qDebug()<<task_name;
            set_success=false;
        }
        if (case_list.isEmpty())
        {
            warning_mesg="WARNING: Please Select at least one case";
            set_success=false;
        }
        if (per_setting_changed && ui->checkBox_ex_setting->isChecked() )
        {
            warning_mesg="WARNING: Please Save your Ex setting before start task";
            set_success=false;
        }
        qDebug()<<"board list is empty0";
        if (board_list.isEmpty() )
        {
            qDebug()<<"board list is empty";
            if (!delay_task)
            {
            warning_mesg="WARNING: Please Select at least one board";
            set_success=false;
            }
        }else
        {
            for (int v=0;v<board_list.count();v++)
            {
                QString task_board=task_name+"-B-"+board_list.at(v);
            if (task_map.find(task_board) != task_map.end() )
            {
                warning_mesg="WARNING:board has already in the task";
                set_success=false;
            }
            }
        }
        if (task_list.indexOf(task_name,0)>=0 && delay_task )
        {
            warning_mesg="WARNING: delay task can't merger into existed task";
            set_success=false;
        }




        toolBox->setCurrentIndex(1);
        if (set_success)
        {
            generate_config();
            generate_tab();
            bool request_task_result=false;
            QString board_string;
            QString case_string;
            QString entry_str;
            Gcmd = new QProcess();
            qDebug()<<__LINE__<<__FILE__<<Gcmd->isSequential();
            for (int i=0;i<case_list_loop.count();i++)
            {
                case_string=case_string+" casename "+case_list_loop.at(i);
            }

            int request_number;
            if ( delay_task )
            {
                request_number=delay_board_num;
            }else
            {
                request_number=board_list.count();
            }

            for (int i=0;i<request_number;i++)
            {

                if ( delay_task )
                {
                    entry_str=" ENTRY \""+delay_setting;
                    task_board_entry[task_name]=delay_setting;
                    delay_task_case_id[task_name]=case_list;
                    if ( ui->checkBox_last_image->isChecked() )
                    {
                    if ( image_modify == "yes" )
                    {
                        delay_task_setting[task_name]<<"Special Image:"+image_version+"_"+image_def+"_"+image_burning_blf+"_"+replaced_image;
                    }else{
                        delay_task_setting[task_name]<<image_version+"_"+image_def+"_"+image_burning_blf;
                    }
                    }
                    else
                    {
                        delay_task_setting[task_name]<<"N/A";
                    }
                    delay_task_setting[task_name]<<ltk_version+"_"+ltk_branch;
                }else
                {
                    board_string=board_list.at(i);
                    entry_str=" ENTRY \"board_id="+board_string;
                    task_board_entry[task_name+"-B-"+board_list.at(i)]="board_id="+board_string;
                }

                if (ui->checkBox_ignore_offline->isChecked())
                {
                    entry_str=entry_str+"&&ignore_offline=1";
                    if ( delay_task )
                    {task_board_entry[task_name]+="&&ignore_offline=1";}else
                    {task_board_entry[task_name+"-B-"+board_list.at(i)]+="&&ignore_offline=1";}
                }
                entry_str=entry_str+"&&task_level="+QString::number(login_level,10);
                if (ui->checkBox_lock_board->isChecked())
                {
                    entry_str=entry_str+"&&lock_task=1\"";
                    if ( delay_task )
                    {task_board_entry[task_name]+="&&lock_task=1";}else
                    {task_board_entry[task_name+"-B-"+board_list.at(i)]+="&&lock_task=1";}
                }else
                {
                    entry_str=entry_str+"\"";
                }

                if ( delay_task )
                {task_board_entry[task_name]+="&&task_level="+QString::number(login_level,10);}else
                {task_board_entry[task_name+"-B-"+board_list.at(i)]+="&&task_level="+QString::number(login_level,10);}

                //entry_str.replace(" ","");
                if (! use_gui_delay )
                {
                    waitProcess(work_dir+"/STAFEnv.sh staf "+staf_ip+" LTKTEST REQUEST TASKNAME "+task_name+entry_str+case_string+ltk_config+image_burn_config+ex_setting+sp_setting_string+log_folder_config,100);
                }else
                {
                    waitProcess(work_dir+"/STAFEnv_delay.sh "+gui_delay_time_str+" staf "+staf_ip+" LTKTEST REQUEST TASKNAME "+task_name+entry_str+case_string+ltk_config+image_burn_config+ex_setting+sp_setting_string+log_folder_config,100);
                }
                if (progress_result)
                {
                    qDebug()<<"request ok.";
                    request_task_result=true;
                    if ( !delay_task )
                    {
                        task_map[task_name+"-B-"+board_string]->setText(1,"Not_Running");
                        task_map[task_name+"-B-"+board_string]->setText(2,"job_for:"+task_name+":"+task_board_entry[task_name+"-B-"+board_list.at(i)]);
                    }
                    task_list<<task_name;
                }else
                {
                    qDebug()<<"requext abornal";
                    if ( !delay_task )
                    {
                        if ( progress_return==4003 ||  progress_return==163 )
                        {
                            request_task_result=true;
                            task_map[task_name+"-B-"+board_string]->setText(1,"Board_Pending");
                        }else
                        {
                            request_task_result=false;
                            task_map[task_name+"-B-"+board_string]->setText(1,"Board_Request_Fail");
                        }
                        task_map[task_name+"-B-"+board_string]->setText(2,"job_for:"+task_name+":"+task_board_entry[task_name+"-B-"+board_list.at(i)]);
                    }else{
                        if ( progress_return==4003 ||  progress_return==163 )
                        {
                            request_task_result=true;
                        }else
                        {
                            request_task_result=false;
                        }
                    }
                }
            }

            if (request_task_result)
            {
                qDebug()<<"task start";
                task_list_all<<task_name;
            }else
            {
                qDebug()<<"request task fail";
                task_map[task_name]->setText(1,"Task Start Fail");
                task_map[task_name]->setForeground(1,QBrush(QColor(Qt::red)));
            }
            image_modify="no";
            blf_modified="no";
            blf_modified_name_list.clear();
            blf_modified_path.clear();
            blf_modified_name.clear();
            blf_modified_folder=QDateTime::currentDateTime().toString(Qt::ISODate).replace(QRegExp("[-:]"),"_")+"tmp_blf";

        }else{
            QMessageBox::about(this, tr("Warning"),warning_mesg);}
        ui->pushButton_start->setDisabled(false);
        show_board();    
    }else
    {
        QMessageBox::about(this, tr("Warning"),tr("System is busy , please try again"));
    }
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug()<<"unregister gui";
    QString unregister_gui="~/ltk_cloud_server/STAFEnv.sh staf "+staf_ip+" ltktest GUIUNREGISTER";
    int system_result=system(unregister_gui.toLatin1().data());

}


void MainWindow::add_ex_setting()
{
    qDebug()<<"check ex setting";
    if (ui->checkBox_ex_setting->isChecked())
    {
        ex_setting=" ExtraAct preset=\""+pre_set_link+"\" "+pre_set_pc_config;
    }else
    {
        ex_setting=" ";
        //per_setting_changed=true;
    }
    toolBox->setCurrentIndex(4);
}

void MainWindow::init_windows()
{

    //create filter menu

    //create filter menu


    qDebug() << QCoreApplication::libraryPaths();
    use_gui_delay=false;




    gui_delay_time_str="0";
    filter_row_num=1;
    hide_unavailable_board=false;
    board_list_reflesh_count=0;
    filter_table_enable=false;
    per_setting_changed=true;
    staf_ip="10.38.34.91";
    stop_socket_connection=true;
    test_package_server_ip="10.38.164.23";
    use_mysql_database=true;
    qt_clipboard = QApplication::clipboard();
    blf_modified_folder=QDateTime::currentDateTime().toString(Qt::ISODate).replace(QRegExp("[-:]"),"_")+"tmp_blf";
    QSize filter_icon_size;
    filter_icon_size.setHeight(12);
    filter_icon_size.setWidth(12);
    filter_icon.addFile("./image/filter.png",filter_icon_size);
    unfilter_icon.addFile("./image/un_filter.png",filter_icon_size);
    lock_icon.addFile("./image/Lock_Closed.png");
    //ui->treeWidget->setFixedWidth(700);
    int system_result;
    system_result=system("wget -T 20 ftp://10.38.164.23/Daily_Build/daily_branch_map.txt -O ./daily_branch_map.txt");
    if ( system_result != 0 )
    {
        QMessageBox::about(this,tr("Wget"),tr("Download daily_branch_map.txt Fail!! \n please try : wget -T 20 ftp://10.38.164.23/Daily_Build/daily_branch_map.txt -O ./daily_branch_map.txt "));
    }
    system_result=system("wget ftp://10.38.164.23/Daily_Build/daily_image_map.txt -O ./daily_image_map.txt");
    if ( system_result != 0 )
    {
        QMessageBox::about(this,tr("Wget"),tr("Download daily_image_map.txt Fail!! \n please try :  wget ftp://10.38.164.23/Daily_Build/daily_image_map.txt -O ./daily_image_map.txt-"));
    }
    QString register_gui_cmd="~/ltk_cloud_server/STAFEnv.sh staf "+staf_ip+" ltktest GUIREGISTER";
    system_result=system(register_gui_cmd.toLatin1().data());
    if ( system_result != 0 )
    {
        QMessageBox::about(this,tr("STAF"),tr("STAF gui register fail"));
    }
    dbMySQL=QSqlDatabase::addDatabase("QMYSQL");
    dbMySQL.setHostName("10.38.34.91");
    dbMySQL.setDatabaseName("ltk");
    dbMySQL.setUserName("hhuan10");
    dbMySQL.setPassword("123456");
    dbMySQL.setConnectOptions("MYSQL_OPT_RECONNECT=1");
    /*
    如果使用了长连接而长期没有对数据库进行任何操作，那么在timeout值后，mysql server就会关闭此连接，而客户端在执行查询的时候就会得到一个类似于“MySQL server has gone away“这样的错误。
    在使用mysql_real_connect连接数据库之后，再使用mysql_options( &mysql, MYSQL_OPT_RECONNECT, … ) 来设置为自动重连。这样当mysql连接丢失的时候，使用mysql_ping能够自动重连数据库。
    如果是在mysql 5.1.6之前，那么则应在每次执行完real_connect 之后执行mysql_options( &mysql, MYSQL_OPT_RECONNECT, … ) ,如果是mysql 5.1.6+，则在connect之前执行一次就够了。
    查看mysql连接数
    mysqladmin -uroot -p  processlist
    实际的测试中我发现，当设置了MYSQL_OPT_RECONNECT为1时，超时后再查看processlist，则自动建立的连接不在列表中，但事实上连接确实建立并被使用了。
    MYSQL的默认设置是在数据库连接超过8小时没有使用后将其断开，如果我们将这个时间改成更大的数值，那么连接超时所需的时间就会更长，也就意味着更不容易超时。
    网络上提供的修改方法一般是修改/etc/my.cnf，在这个文件中添加一行wait_timeout=你需要设置的超时时间 。实际上有一种比较简单的方法来修改这个参数：
    首先作为超级用户登录到MYSQL，注意必须是超级用户，否则后面会提示没有修改权限。然后输入
    show global variables like 'wait_timeout';
    回车执行后显示目前的超时时间：
    +---------------+-------+
    | Variable_name | Value |
    +---------------+-------+
    | wait_timeout | 28800 |
    +---------------+-------+
    1 row in set (0.00 sec)
    上面显示的是默认的超时时间，即8个小时(单位是秒)。现在重新设置该参数，例如我们要将超时时间设置成10个小时，可以输入：
    set global wait_timeout=36000;
    回车执行，显示：
    Query OK, 0 rows affected (0.00 sec)
    表示设置成功，可以重新使用show global variables like 'wait_timeout'来验证。
      */
    qDebug()<< "link sql";
    if (!dbMySQL.open())
    {
        qDebug()<< "link fail";
        QMessageBox::about(this,tr("My SQL"),tr("My SQL connect fail"));
    }
    else
        qDebug()<< "link OK";



    QSqlQuery query;
    query.exec("select * from User_Group;");
    user_group_list.clear();
    while (query.next())
    {
         user_group_list << query.value(0).toString();
    }

    QString config_file="./cloud_server_config.ini";
    QSettings settings(config_file,QSettings::IniFormat);
    login_name=settings.value("login_name").toString();
    login_password=settings.value("login_password").toString();

    if (login_name.isEmpty())
    {
        login_name="Guest";
        login_password="Guest125";
        ui->pushButton_start->setDisabled(true);
    }

    //for linux mysql reconnection
/*
    QVariant v=dbMySQL.driver()->handle();
    if(qstrcmp(v.typeName(),"MYSQL*")==0){
        MYSQL *handle=*static_cast<MYSQL **>(v.data());
        if(handle !=0){
            qDebug()<<"the connector_fd field value"<<handle->connector_fd
                   <<"and the server_status="<<handle->server_status
                  <<"error="<<mysql_error(handle);
            my_bool reconnect=1;
            int mysql_ret=mysql_options(handle,MYSQL_OPT_RECONNECT,&reconnect);
            qDebug()<<__LINE__<<__FILE__<<"mysql_ret="<<mysql_ret<<"error="<<mysql_error(handle);
        }else{
            qDebug()<<"the handle is null";}
    }

     //for linux mysql reconnection
*/

    ui->treeWidget->header()->setResizeMode(QHeaderView::ResizeToContents);


    platform=":"+ui->comboBox_platform->currentText()+":";
    Date=QDate::currentDate();

    platform_map["HELAN3"]="pxa1936";
    platform_map["EDEN"]="pxa1928";
    platform_map["ULC1"]="pxa1908";
    set_success=true;
    progress_lock=false;
    toolBox = new QTabWidget(ui->dockWidget_4);
    ui->gridLayout_6->addWidget(toolBox, 0, 0, 1, 1);

    start_config =new QTextEdit;
    start_config->setTextInteractionFlags(Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);
    toolBox->addTab(start_config,"Board");
    task_tree = new QTreeWidget;

    toolBox->addTab(task_tree,"Task");
    task_tree->header()->setResizeMode(QHeaderView::ResizeToContents);
    task_tree->setStyleSheet("font: 8pt");
    QStringList id;
    id<<"Task"<<"Status"<<"Detail"<<"Progress";
    task_tree->setColumnCount(4);
    task_tree->setColumnHidden(2,true);
    task_tree->setColumnHidden(3,true);
    task_tree->setHeaderLabels(id);
    task_tree_row=0;

    xml_read();
    show_tree();
    ui->tableWidget_board->setColumnCount(16);
    ui->checkBox_console_log->setHidden(true);
    show_board();


    ltk_package_tree=new QTreeWidget;
    toolBox->addTab(ltk_package_tree,"LTK");
    ltk_package_tree->header()->setResizeMode(QHeaderView::ResizeToContents);
    ltk_package_tree->setStyleSheet("font: 8pt");
    ltk_package_tree->setColumnCount(1);
    ltk_package_tree->setHeaderLabel("Package List");
    show_package_tree();
    ltk_image_tree=new QTreeWidget;
    toolBox->addTab(ltk_image_tree,"Image");
    ltk_image_tree->header()->setResizeMode(QHeaderView::ResizeToContents);
    ltk_image_tree->setStyleSheet("font: 8pt");
    QStringList tree_image_list;
    tree_image_list<<"Image List"<<"Detail";
    //ltk_image_tree->setHeaderLabel("Image List");
    ltk_image_tree->setHeaderLabels(tree_image_list);
    ltk_image_tree->setDisabled(true);
    show_image_tree();
    show_package();

    ui->pushButton_start->setDisabled(false);
    connect(task_tree,SIGNAL(itemPressed(QTreeWidgetItem*,int)),this,SLOT(tree_menu(QTreeWidgetItem*)));
    connect(ltk_package_tree,SIGNAL(itemClicked(QTreeWidgetItem*,int)),this,SLOT(check_pakcage_tree(QTreeWidgetItem*,int)));
    connect(ltk_image_tree,SIGNAL(itemPressed(QTreeWidgetItem*,int)),this,SLOT(open_dialog_for_special_image(QTreeWidgetItem*,int)));
    connect(ui->tableWidget_board,SIGNAL(itemPressed(QTableWidgetItem*)),this,SLOT(board_lock_menu(QTableWidgetItem*)));


    widget_ex_setting=new QWidget;
    open_file=new QPushButton("Open");
    save_config=new QPushButton("Save");
    clear_ex_setting=new QPushButton("Clear");
    layout_ex_setting=new QGridLayout(widget_ex_setting);
    per_shell_setting= new QTextEdit;
    per_set_pc=new QCheckBox("PC");
    per_set_pc->setToolTip("PC side script or Board side script");
    layout_ex_setting->addWidget(per_shell_setting,0,0,1,4);
    layout_ex_setting->addWidget(open_file,1,1,1,1);
    layout_ex_setting->addWidget(clear_ex_setting,1,2,1,1);
    layout_ex_setting->addWidget(save_config,1,3,1,1);
    layout_ex_setting->addWidget(per_set_pc,1,0,1,1);

    toolBox->addTab(widget_ex_setting,"EX_SET");
    per_shell_setting->setText("echo \"Welcome to LTK Cloud\"");
    connect(open_file,SIGNAL(clicked()),this,SLOT(open_ex_setting_file()));
    connect(save_config,SIGNAL(clicked()),this,SLOT(save_ex_setting()));
    connect(clear_ex_setting,SIGNAL(clicked()),per_shell_setting,SLOT(clear()));
    connect(per_shell_setting,SIGNAL(textChanged()),this,SLOT(set_per_setting_changed()));
    connect(per_set_pc,SIGNAL(clicked()),this,SLOT(add_pc_per_set_config()));
    //connect(ui->checkBox_dealy_task,SIGNAL(clicked(bool)),ui->tabWidget,SLOT(choose_delay_tab(bool)));
    create_delay_setting();
    toolBox->setCurrentIndex(3);
    //update_result = new QTimer;
    //connect(update_result,SIGNAL(timeout()),this,SLOT(time_update()));
    //update_result->start(2000);
    //update_result->stop();



    connect(ui->tableWidget_board->horizontalHeader(),SIGNAL(sectionClicked(int)),this,SLOT(sort_col(int)));
    //connect(ui->treeWidget->headerItem(),SIGNAL(sectionClicked()),this,SLOT(show_sort_tree_menu()));
    ui->treeWidget->setSortingEnabled(true);
    ui->treeWidget->sortByColumn(0,Qt::AscendingOrder);

    ui->tabWidget->setTabText(0,"Board_list");
    create_menu();
    create_tool_bar();



    platform=ui->comboBox_platform->currentText();


 ui->groupBox_config->setMaximumWidth(300);

 ui->gridLayout->setSpacing(1);
 ui->gridLayout->setVerticalSpacing(1);
 ui->gridLayout->setContentsMargins(1,1,1,1);
 ui->gridLayout->setRowStretch(6,1);

ui->pushButton_start->setFixedWidth(130);
ui->pushButton_confirm->setFixedWidth(130);
ui->pushButton_update->setFixedWidth(125);
ui->lineEdit_task_name->setFixedWidth(130);
ui->comboBox_platform->setFixedSize(125,25);
ui->comboBox_runcase_mode->setFixedSize(125,25);
QStringList runcase_mode_list;
runcase_mode_list<<"serialonly"<<"serial"<<"adb";
ui->comboBox_runcase_mode->addItems(runcase_mode_list);
ui->comboBox_runcase_mode->setEnabled(false);
toolBox->setMinimumWidth(300);
QWidget *remove_dock_title=new QWidget(this);
QWidget *remove_dock_title1=new QWidget(this);
//ui->gridLayout_3->setSpacing(0);
ui->dockWidget->setTitleBarWidget(remove_dock_title);
ui->dockWidget_4->setTitleBarWidget(remove_dock_title1);
 ui->dockWidget->setMinimumSize(QSize(425, 300));
 ui->groupBox_config->setMinimumSize(QSize(200, 330));



 ui->dockWidget_4->resize(QSize(425, 200));
 ui->tabWidget->resize(QSize(700,200));
toolBox->resize(QSize(425, 200));
//QSettings settings("cloud_server_config.ini",QSettings::IniFormat);
QString window_width=settings.value("window_width").toString();
QString window_height=settings.value("window_height").toString();
int win_h,win_w;
if ( window_width!="0" && !window_width.isEmpty() && window_height!="0" && !window_height.isEmpty())
{
    win_h=window_height.toInt()-300;
    win_w=window_width.toInt()-500;
    ui->dockWidget_4->setMaximumHeight(win_h);
    ui->tabWidget->setMaximumHeight(win_h);
    toolBox->setMaximumHeight(win_h);

    QString font_size=QString::number(window_height.toInt()/100,10);
    QString sheet_style="font:"+font_size+"pt;";
    delay_chip_type_combobox->setStyleSheet(sheet_style);
    delay_ddr_type_combobox->setStyleSheet(sheet_style);
    delay_board_type_combobox->setStyleSheet(sheet_style);
    delay_chip_stepping_combobox->setStyleSheet(sheet_style);
    delay_ddr_size_combobox->setStyleSheet(sheet_style);
    delay_emmc_type_combobox->setStyleSheet(sheet_style);
    delay_dro_combobox->setStyleSheet(sheet_style);
    delay_lcd_resulation_combobox->setStyleSheet(sheet_style);
    delay_rf_name_combobox->setStyleSheet(sheet_style);
    delay_rf_type_combobox->setStyleSheet(sheet_style);
    delay_board_id_combobox->setStyleSheet(sheet_style);
    delay_user_team_combobox->setStyleSheet(sheet_style);

    //ui->treeWidget->setMaximumWidth(win_w);
   //window_width.data()

}


ui->groupBox_config->setStyleSheet("QGroupBox{border-width: 1px;border-style:solid;border-color:lightGray;}");
connect(ui->tabWidget,SIGNAL(tabCloseRequested(int)),this,SLOT(tabCloseSlot(int)));
QIcon icon("./image/icon1.png");
setWindowIcon(icon);


//ui->label_log_setting->setHidden(true);
//ui->commandLinkButton_log->setFixedHeight(25);
//hide_show_log_setting();

if ( ! debug_mode){
ui->pushButton_update->setHidden(true);
ui->checkBox_ignore_offline->setHidden(true);
ui->checkBox_console_log->setHidden(true);
}else
{
}
account_function();
}


void MainWindow::add_pc_per_set_config()
{
    if ( per_set_pc->isChecked() )
    {
        pre_set_pc_config=" ExtraAct preset_pc=yes ";
        per_shell_setting->setText("#!/bin/bash\necho \"board_id is $BOARD_ID\"\necho \"adb_id is $ADB_ID\"\n#wget ftp://10.38.164.23/Daily_Build/preset/modified_Case_information.xml  -O ./Case_information.xml ");
    }else
    {
        pre_set_pc_config=" ExtraAct preset_pc=no ";
        per_shell_setting->setText("echo \"Welcome to LTK Cloud\"");
    }
}




void MainWindow::show_sort_tree_menu()
{
    qDebug()<<"show_sort_tree_menu";
    QMenu* item_menu= new QMenu("table_menu");

    QAction *order = new QAction(tr("Sort A -> Z"),this);
    QAction *desorder = new QAction(tr("Sort Z -> A"),this);


    item_menu->addAction(order);
    item_menu->addAction(desorder);

    connect(order,SIGNAL(triggered()),this,SLOT(sort_tree_order()));
    connect(desorder,SIGNAL(triggered()),this,SLOT(sort_tree_desorder()));
    item_menu->setStyleSheet("font:9pt;text-align:center;");
    item_menu->exec(QCursor::pos());

}

void MainWindow::sort_tree_desorder()
{
    ui->treeWidget->sortByColumn(0,Qt::DescendingOrder);
}

void MainWindow::sort_tree_order()
{
    ui->treeWidget->sortByColumn(0,Qt::DescendingOrder);
}


void MainWindow::choose_delay_tab(bool delay_chosed)
{
    qDebug()<<"choose_delay_tab";
    if (login_level==1)
    {
        QMessageBox::about(this, tr("Warning"),tr("You are the Local user and can't use delay job"));
    }else
    {
    if (delay_chosed)
    {
        ui->tabWidget->setCurrentIndex(1);
        //ui->tableWidget_board->setDisabled(true);

    }else{
        ui->tabWidget->setCurrentIndex(0);
        //ui->tableWidget_board->setDisabled(false);
    }
    }
}

void MainWindow::hide_show_log_setting()
{
    if (! ui->checkBox_auto_emmd->isHidden())
    {
        ui->checkBox_auto_emmd->setHidden(true);
        ui->checkBox_console_log->setHidden(true);
    }else
    {
        ui->checkBox_auto_emmd->setHidden(false);
        ui->checkBox_console_log->setHidden(false);
    }
}


void MainWindow::get_delay_setting()
{
    qDebug()<<"get delay setting";
    delay_setting="";
    if (! ui->checkBox_dealy_task->isChecked())
    {
        delay_setting="";
        delay_task=false;
    }else
    {
        delay_setting+="chip_name="+platform_map[ui->comboBox_platform->currentText()].toUpper();
        if ( delay_board_type_combobox->currentText() != "*" )
        { delay_setting+="&&board_type="+delay_board_type_combobox->currentText(); }
        if ( delay_chip_stepping_combobox->currentText() != "*" )
        { delay_setting+="&&chip_stepping="+delay_chip_stepping_combobox->currentText(); }
        if ( delay_ddr_type_combobox->currentText() != "*" )
        { delay_setting+="&&ddr_type="+ddr_type_lp_transform(delay_ddr_type_combobox->currentText()); }
        if ( delay_ddr_size_combobox->currentText() != "*" )
        { delay_setting+="&&ddr_size="+delay_ddr_size_combobox->currentText(); }
        if ( delay_emmc_type_combobox->currentText() != "*" )
        { delay_setting+="&&emmc_type="+delay_emmc_type_combobox->currentText(); }
        if ( delay_dro_combobox->currentText() != "*" )
        { delay_setting+="&&dro="+delay_dro_combobox->currentText(); }
        if ( delay_lcd_resulation_combobox->currentText() != "*" )
        { delay_setting+="&&lcd_resulation="+delay_lcd_resulation_combobox->currentText(); }
        if ( delay_chip_type_combobox->currentText() != "*" )
        { delay_setting+="&&chip_type="+delay_chip_type_combobox->currentText(); }
        if ( delay_rf_name_combobox->currentText() != "*" )
        { delay_setting+="&&rf_name="+delay_rf_name_combobox->currentText(); }
        if ( delay_rf_type_combobox->currentText() != "*" )
        { delay_setting+="&&rf_type="+delay_rf_type_combobox->currentText(); }
        if ( delay_board_id_combobox->currentText() != "*" )
        { delay_setting+="&&board_id="+delay_board_id_combobox->currentText(); }
        if ( delay_user_team_combobox->currentText() != "*" )
        { delay_setting+="&&user_team="+delay_user_team_combobox->currentText(); }
        //delay_setting+="\"";
        QString board_num_str=delay_board_num_lineedit->text();
        delay_board_num=board_num_str.toInt();
        delay_task=true;
    }
    qDebug()<<"Delay setting:"<<delay_setting;
}




void MainWindow::create_delay_setting()
{

    QWidget *delay_setting_widget,*delay_take_place_widget;

    QLabel *delay_board_type_label,*delay_chip_stepping_label,*delay_ddr_type_label,*delay_ddr_size_label,*delay_emmc_type_label,*delay_dro_label,*delay_lcd_resulation_label,*delay_chip_type_label,*delay_rf_name_label,*delay_rf_type_label,*delay_board_id_label,*delay_user_team_label;
    QGridLayout *delay_gridlayout,*delay_gridlayout_top;
    QLabel *delay_board_num_label;
    QGroupBox* delay_filter_box;
    QFont item1;
    item1.setBold(true);


    delay_setting_widget=new QWidget;
    delay_gridlayout_top=new QGridLayout(delay_setting_widget);

    delay_take_place_widget=new QWidget;
    delay_gridlayout_top->addWidget(delay_take_place_widget,2,2);
    delay_take_place_widget->setMinimumWidth(500);
    delay_filter_box=new QGroupBox;

    qDebug()<<"create delay 1";

    delay_filter_box=new QGroupBox("");
    delay_filter_box->setFont(item1);
    delay_filter_box->setStyleSheet("QGroupBox{border-width: 1px;border-style:solid;border-color:lightGray;}");
    delay_gridlayout_top->addWidget(delay_filter_box,0,0,1,2);
    delay_gridlayout=new QGridLayout(delay_filter_box);
    delay_gridlayout->setSpacing(1);



    delay_board_num_label=new QLabel("Delay Board Number:");
    delay_board_num_label->setFont(item1);
    delay_board_num_lineedit=new QLineEdit("1");
    delay_gridlayout->addWidget(delay_board_num_label,0,0,1,1);
    delay_gridlayout->addWidget(delay_board_num_lineedit,0,1,1,1);

    QFrame *delay_line;
    delay_line=new QFrame;
    delay_line->setFrameShape(QFrame::HLine);
    delay_line->setFrameShadow(QFrame::Sunken);

    delay_gridlayout->addWidget(delay_line,1,0,1,2);

    QLabel *delay_filter_label;
    delay_filter_label=new QLabel("Delay Board Filter:");
    delay_filter_label->setFont(item1);
    delay_gridlayout->addWidget(delay_filter_label,2,0,1,2);

    delay_board_type_label=new QLabel("Board Type");
    delay_gridlayout->addWidget(delay_board_type_label,3,0,1,1);
    delay_board_type_combobox=new QComboBox;
    delay_board_type_combobox->setEditable(true);
    delay_gridlayout->addWidget(delay_board_type_combobox,3,1,1,1);
    QStringList d_board_type;
    d_board_type<<"*"<<"EMPC_V1.0"<<"EMPC_V1.1"<<"EMPC_V2.0";
    delay_board_type_combobox->addItems(d_board_type);

    delay_chip_stepping_label=new QLabel("chip_stepping");
    delay_gridlayout->addWidget(delay_chip_stepping_label,4,0,1,1);
    delay_chip_stepping_combobox=new QComboBox;
    delay_chip_stepping_combobox->setEditable(true);
    delay_gridlayout->addWidget(delay_chip_stepping_combobox,4,1,1,1);
    QStringList d_chip_stepping;
    d_chip_stepping<<"*"<< "A0" << "A1" << "B0" << "B1"<<"Z1"<<"Z2";
    delay_chip_stepping_combobox->addItems(d_chip_stepping);

    delay_ddr_type_label=new QLabel("ddr_type");
    delay_gridlayout->addWidget(delay_ddr_type_label,5,0,1,1);
    delay_ddr_type_combobox=new QComboBox;
    delay_ddr_type_combobox->setEditable(true);
    delay_gridlayout->addWidget(delay_ddr_type_combobox,5,1,1,1);
    QStringList d_ddr_type;
    d_ddr_type<<"*"<<"SSG@P"<<"SSG@D"<<"THA@P"<<"THA@D"<<"HYX@P"<<"HYX@D"<<"SNDK@P"<<"SNDK@D"<<"MCN@P"<<"MCN@D";
    delay_ddr_type_combobox->addItems(d_ddr_type);

    delay_ddr_size_label=new QLabel("ddr_size");
    delay_gridlayout->addWidget(delay_ddr_size_label,6,0,1,1);
    delay_ddr_size_combobox=new QComboBox;
    delay_ddr_size_combobox->setEditable(true);
    delay_gridlayout->addWidget(delay_ddr_size_combobox,6,1,1,1);
    QStringList d_ddr_size;
    d_ddr_size<<"*"<<"1G@800"<<"1G@667"<<"1G@400"<<"2G@800"<<"2G@667"<<"2G@400";
    delay_ddr_size_combobox->addItems(d_ddr_size);

    delay_emmc_type_label=new QLabel("emmc_type");
    delay_gridlayout->addWidget(delay_emmc_type_label,7,0,1,1);
    delay_emmc_type_combobox=new QComboBox;
    delay_emmc_type_combobox->setEditable(true);
    delay_gridlayout->addWidget(delay_emmc_type_combobox,7,1,1,1);
    QStringList d_emmc_type;
    d_emmc_type<<"*"<< "Samsung" << "Micron" << "Hynix" << "Elpida" << "Sandisk"<<"Toshiba";
    delay_emmc_type_combobox->addItems(d_emmc_type);

    delay_dro_label=new QLabel("DRO");
    delay_gridlayout->addWidget(delay_dro_label,8,0,1,1);
    delay_dro_combobox=new QComboBox;
    delay_dro_combobox->setEditable(true);
    delay_gridlayout->addWidget(delay_dro_combobox,8,1,1,1);
    QStringList d_dro;
    d_dro<<"*"<< "4" << "8" << "16" << "32";
    delay_dro_combobox->addItems(d_dro);

    delay_lcd_resulation_label=new QLabel("lcd_resulation");
    delay_gridlayout->addWidget(delay_lcd_resulation_label,9,0,1,1);
    delay_lcd_resulation_combobox=new QComboBox;
    delay_lcd_resulation_combobox->setEditable(true);
    delay_gridlayout->addWidget(delay_lcd_resulation_combobox,9,1,1,1);
    QStringList d_lcd_resulation;
    d_lcd_resulation<<"*"<<"720P"<<"1080P";
    delay_lcd_resulation_combobox->addItems(d_lcd_resulation);

    delay_chip_type_label=new QLabel("chip_type");
    delay_gridlayout->addWidget(delay_chip_type_label,10,0,1,1);
    delay_chip_type_combobox=new QComboBox;
    delay_chip_type_combobox->setEditable(true);
    delay_gridlayout->addWidget(delay_chip_type_combobox,10,1,1,1);
    QStringList d_chip_type;
    d_chip_type<<"*"<<"PXA1926_2L_DISCRETE"<<"PXA1928_POP"<<"PXA1928_4L"<<"PXA1928_2L";
    delay_chip_type_combobox->addItems(d_chip_type);

    delay_rf_name_label=new QLabel("rf_name");
    delay_gridlayout->addWidget(delay_rf_name_label,11,0,1,1);
    delay_rf_name_combobox=new QComboBox;
    delay_rf_name_combobox->setEditable(true);
    delay_gridlayout->addWidget(delay_rf_name_combobox,11,1,1,1);
    QStringList d_rf_name;
    d_rf_name<<"*";
    delay_rf_name_combobox->addItems(d_rf_name);

    delay_rf_type_label=new QLabel("rf_type");
    delay_gridlayout->addWidget(delay_rf_type_label,12,0,1,1);
    delay_rf_type_combobox=new QComboBox;
    delay_rf_type_combobox->setEditable(true);
    delay_gridlayout->addWidget(delay_rf_type_combobox,12,1,1,1);
    QStringList d_rf_type;
    d_rf_type<<"*";
    delay_rf_type_combobox->addItems(d_rf_type);

    delay_board_id_label=new QLabel("board_id");
    delay_gridlayout->addWidget(delay_board_id_label,13,0,1,1);
    delay_board_id_combobox=new QComboBox;
    delay_board_id_combobox->setEditable(true);
    delay_gridlayout->addWidget(delay_board_id_combobox,13,1,1,1);
    QStringList d_board_id;
    d_board_id<<"*";
    delay_board_id_combobox->addItems(d_board_id);

    delay_user_team_label=new QLabel("user_team");
    delay_gridlayout->addWidget(delay_user_team_label,14,0,1,1);
    delay_user_team_combobox=new QComboBox;
    delay_user_team_combobox->setEditable(true);
    delay_gridlayout->addWidget(delay_user_team_combobox,14,1,1,1);
    QStringList d_user_team;
    d_user_team<<"*";
    delay_user_team_combobox->addItems(d_user_team);


    qDebug()<<"add special setting";
    QWidget* special_setting_widget;
    special_setting_widget=new QWidget;
    QGroupBox* special_setting_box;
    special_setting_box=new QGroupBox("Special Setting");

    delay_gridlayout_top->addWidget(special_setting_widget,2,2);
    special_setting_widget->setMinimumWidth(500);
    special_setting_box->setFont(item1);
    special_setting_box->setStyleSheet("QGroupBox{border-width: 1px;border-style:solid;border-color:lightGray;}");


    QGridLayout *special_setting_gridlayout;
    special_setting_gridlayout=new QGridLayout(special_setting_box);
    delay_gridlayout_top->addWidget(special_setting_box,0,2,2,3);

    real_time_log_dump=new QCheckBox("Real time pull log");
    real_time_log_dump->setChecked(true);
    special_setting_gridlayout->addWidget(real_time_log_dump,0,0,1,1);

    stop_auto_list_board=new QCheckBox("Stop auto list board");
    special_setting_gridlayout->addWidget(stop_auto_list_board,1,0,1,1);
    connect(stop_auto_list_board,SIGNAL(clicked()),this,SLOT(gui_action_config()));

    stop_auto_update_job_status=new QCheckBox("Stop auto update job status");
    special_setting_gridlayout->addWidget(stop_auto_update_job_status,2,0,1,1);

    setup_ltk_checkbox=new QCheckBox("Setup LTK");
    special_setting_gridlayout->addWidget(setup_ltk_checkbox,3,0,1,1);
    setup_ltk_checkbox->setChecked(true);

    use_new_swdl=new QCheckBox("Use new swdl");
    special_setting_gridlayout->addWidget(use_new_swdl,4,0,1,1);
    use_new_swdl->setChecked(false);

    panic_detection_checkbox=new QCheckBox("Panic/Hung Detection");
    special_setting_gridlayout->addWidget(panic_detection_checkbox,5,0,1,1);
    panic_detection_checkbox->setChecked(true);

    skip_image_checkbox=new QCheckBox("Skip Image");
    special_setting_gridlayout->addWidget(skip_image_checkbox,6,0,1,1);

    skip_image_lineedit=new QLineEdit;
    special_setting_gridlayout->addWidget(skip_image_lineedit,6,1,1,1);
    special_setting_gridlayout->setSpacing(1);

    ui->tabWidget->addTab(delay_setting_widget,"Special Task");
    stop_auto_list_board->setChecked(true);
}

void MainWindow::gui_action_config()
{
    if (stop_auto_list_board->isChecked())
    {
        stop_socket_connection=true;
    }else
    {
        stop_socket_connection=false;
    }
}


void MainWindow::add_sp_setting()
{
    sp_setting_string="";
    if ( ! real_time_log_dump->isChecked())
    {
        sp_setting_string+=" General real_time_pull_log=no ";
    }
    if ( ! setup_ltk_checkbox->isChecked())
    {
        sp_setting_string+=" General update_ltk=no ";
    }
    if ( use_new_swdl->isChecked())
    {
        sp_setting_string+=" ImageConfig swdl_version=swdl_new ";
    }
    if ( ! panic_detection_checkbox->isChecked())
    {
        sp_setting_string+=" General panic_detection=no ";
    }
    if (skip_image_checkbox->isChecked())
    {
        sp_setting_string+=" ImageConfig skip_image=\""+skip_image_lineedit->text()+"\" ";
    }
}


void MainWindow::open_ex_setting_file()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                   tr("Open File"), "~",
                                   tr("All files (*.*)"));
        qDebug()<<"open:"<<fileName;

        QTextCodec *codec = QTextCodec::codecForName("UTF-8");
        QTextCodec::setCodecForCStrings(codec);

        QFile file(fileName);
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug()<<"Can't open the file!"<<endl;
        }
                QString file_text;
        while(!file.atEnd()) {
            QByteArray line = file.readLine();
            QString str(line);
            file_text=file_text+str;
        }
        per_shell_setting->clear();
        per_shell_setting->setText(file_text);

}

void MainWindow::save_ex_setting()
{

    save_config->setText("Saving");
    qDebug()<<"save setting begins";
    save_config->setEnabled(false);
    ui->pushButton_start->setEnabled(false);
    qDebug()<<"save setting ends";
    //QApplication::processEvents();
    //sleep(5);


    QString read_setting=per_shell_setting->toPlainText();
    QDate date=QDate::currentDate();
    QString strDate = date.toString("MM-dd");
    QTime time=QTime::currentTime();
    QString user_name=QDesktopServices::storageLocation(QDesktopServices::HomeLocation).section("/",-1,-1);
    QString save_file_name= user_name+"_"+strDate+"_"+time.toString("hh-mm-ss")+".sh";
    qDebug()<<save_file_name;
    QFile save_file(save_file_name);
    if(!save_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug()<<"Can't open the file!"<<endl;
    }
    save_file.reset();
    save_file.write(read_setting.toLatin1().data());
    save_file.close();

    save_config->setText("Uploading");


    pre_set_link ="ftp://"+test_package_server_ip+"//Daily_Build/Blf_Update/pre_set/"+save_file_name;
    QString curl_update_cmd="./curl --max-time 300 --retry 3 --retry-delay 5 --ftp-create-dirs -T "+save_file_name+" ftp://"+test_package_server_ip+"//Daily_Build/Blf_Update/pre_set/";
    qDebug()<<curl_update_cmd;
    int curl_update_result=system(curl_update_cmd.toLatin1().data());

    if (curl_update_result!=0)
    {
        QMessageBox::about(this,tr("Network"),tr("Script file update fail"));
    }


    QFile::copy(save_file_name,"local_config/"+save_file_name);
    QFile::remove(save_file_name);
    save_config->setText("Save");
    save_config->setDisabled(false);
    ui->pushButton_start->setEnabled(true);
    per_setting_changed=false;

    //ttt();
}

void MainWindow::ttt()
{
    sleep(5);
    QString read_setting=per_shell_setting->toPlainText();
    QDate date=QDate::currentDate();
    QString strDate = date.toString("MM-dd");
    QTime time=QTime::currentTime();
    QString user_name=QDesktopServices::storageLocation(QDesktopServices::HomeLocation).section("/",-1,-1);
    QString save_file_name= user_name+"_"+strDate+"_"+time.toString("hh-mm-ss")+".sh";
    qDebug()<<save_file_name;
    QFile save_file(save_file_name);
    if(!save_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug()<<"Can't open the file!"<<endl;
    }
    save_file.reset();
    save_file.write(read_setting.toLatin1().data());
    save_file.close();

    save_config->setText("Uploading");


    pre_set_link ="ftp://10.38.164.23//Daily_Build/Blf_Update/pre_set/"+save_file_name;
    QString curl_update_cmd="./curl --max-time 300 --retry 3 --retry-delay 5 --ftp-create-dirs -T "+save_file_name+" ftp://10.38.164.23//Daily_Build/Blf_Update/pre_set/";
    qDebug()<<curl_update_cmd;
    int curl_update_result=system(curl_update_cmd.toLatin1().data());

    if (curl_update_result!=0)
    {
        QMessageBox::about(this,tr("Network"),tr("Script file update fail"));
    }

    save_config->setText("Save");
    save_config->setDisabled(false);
    ui->pushButton_start->setEnabled(true);
    per_setting_changed=false;
}



void MainWindow::tabCloseSlot(int tab_index)
{
    qDebug()<<"close tab"<<tab_index;
    if(tab_index!=0 && tab_index!=1)
    {
     QString del_task_name=ui->tabWidget->tabText(tab_index);
     ui->tabWidget->removeTab(tab_index);




    QTreeWidgetItemIterator check_tree(task_tree);
    while (*check_tree)
    {
        qDebug()<<del_task_name<<(*check_tree)->text(0);
        if ( (*check_tree)->text(0) == del_task_name )
        {
            qDebug()<<"finded task tree";
            int index=task_tree->indexOfTopLevelItem(*check_tree);
            task_tree->takeTopLevelItem(index);
        }
        ++check_tree;
    }

    QMap<QString,QTreeWidgetItem*>::Iterator it1;
    for (it1=task_map.begin();it1 !=task_map.end();++it1)
    {
        QString del_task=it1.key();
        if (del_task.indexOf(del_task_name+"-B-") >=0 || del_task == del_task_name)
        {
            task_map.remove(del_task);
        }
    }

    QMap<QString,QTableWidget*>::Iterator it2;
    for (it2=task_tab_map.begin();it2 !=task_tab_map.end();++it2)
    {
        QString del_task=it2.key();
        if (del_task.indexOf(del_task_name+"-B-") >=0 || del_task == del_task_name)
        {
            task_tab_map.remove(del_task);
        }
    }


    //QMap<QString,QTableWidgetItem*> staf_task_case_map_tab;
    //QMap<QString,QTableWidgetItem*> staf_task_log_map_tab;



    task_list.removeAt(task_list.indexOf(del_task_name));
}
}


void MainWindow::register_gui()
{
    QString register_gui_cmd="~/ltk_cloud_server/STAFEnv.sh staf "+staf_ip+" ltktest GUIREGISTER";
//int system_result=system(register_gui_cmd.toLatin1().data());
    //QString register_gui_cmd=work_dir+"/delay.sh ";
    QProcess *cmd1=new QProcess();
    cmd1->start(register_gui_cmd);
    while((cmd1->state() == QProcess::Running) || (cmd1->state() == QProcess::Starting) )
    {
        QEventLoop eventloop;
        QTimer::singleShot(100,&eventloop,SLOT(quit()));
        eventloop.exec();
    }
    int system_result=cmd1->exitCode();
    if ( system_result != 0 )
    {
        QMessageBox::about(this,tr("STAF"),tr("STAF gui register fail"));
    }else
    {
        QMessageBox::about(this,tr("STAF"),tr("STAF GUI register Success"));
    }
}


void MainWindow::act_reload_blf_map()
{
    int system_result;
    system_result=system("wget ftp://10.38.164.23/Daily_Build/daily_image_map.txt -O ./daily_image_map.txt");
    if ( system_result != 0 )
    {
        QMessageBox::about(this,tr("Wget"),tr("Download daily_image_map.txt Fail!! \n please try :  wget ftp://10.38.164.23/Daily_Build/daily_image_map.txt -O ./daily_image_map.txt-"));
    }
    show_image_tree();
}


void MainWindow::act_reload_ltk_map()
{
    int system_result;
    system_result=system("wget -T 20 ftp://10.38.164.23/Daily_Build/daily_branch_map.txt -O ./daily_branch_map.txt");
    if ( system_result != 0 )
    {
        QMessageBox::about(this,tr("Wget"),tr("Download daily_branch_map.txt Fail!! \n please try : wget -T 20 ftp://10.38.164.23/Daily_Build/daily_branch_map.txt -O ./daily_branch_map.txt "));
    }
    show_package();
}

void MainWindow::act_user_login()
{
    qDebug()<<"User login";
    ask_login_dialog= new QDialog;
    ask_login_dialog->setWindowTitle("Account Login");
    lineedit_login_username=new QLineEdit;
    lineedit_login_password=new QLineEdit;
    lineedit_login_password->setEchoMode(QLineEdit::Password);
    button_login_ok=new QPushButton("OK");
    //combobox_login_level=new QComboBox;
    QGridLayout *gridLayout_ask_dialog=new QGridLayout(ask_login_dialog);
    gridLayout_ask_dialog->addWidget(lineedit_login_username,0,1);
    gridLayout_ask_dialog->addWidget(lineedit_login_password,0,2);
    //gridLayout_ask_dialog->addWidget(combobox_login_level,1,0,2,1);
    //gridLayout_ask_dialog->addWidget(button_login_ok,1,1,2,2);
    gridLayout_ask_dialog->addWidget(button_login_ok,0,3);
    //ask_login_dialog->show();

   //QStringList account_level;
   //account_level<<"Global_user"<<"Group_user"<<"Personal_user";
   //combobox_login_level->addItems(account_level);


   QString config_file="./cloud_server_config.ini";
   QSettings settings(config_file,QSettings::IniFormat);
   login_name=settings.value("login_name").toString();
   login_password=settings.value("login_password").toString();

   if (login_name.isEmpty())
   {
       login_name="Guest";
       login_password="Guest125";
   }
   lineedit_login_username->setText(login_name);
   lineedit_login_password->setText(login_password);
    //connect(lineedit_login_username,SIGNAL(clicked()),lineedit_login_username,SLOT(clear()));
    //connect(lineedit_login_password,SIGNAL(clicked()),lineedit_login_password,SLOT(clear()));


   //QSqlDatabase dbMySQL=QSqlDatabase::addDatabase("QMYSQL");
   //dbMySQL.setHostName("10.38.34.91");
   //dbMySQL.setDatabaseName("ltk");
   //dbMySQL.setUserName("hhuan10");
   //dbMySQL.setPassword("123456");
   //if (!dbMySQL.open())
   //    qDebug()<< "link fail";
   //else
   //     qDebug()<< "link OK";
   QSqlQuery query;
   query.exec("select * from Outlook_Address_Book;");
   QStringList wordList1;
   while (query.next())
   {
       wordList1 << query.value(0).toString().replace(" ","_");
   }

   query.exec("select * from User_Group;");
   user_group_list.clear();
   while (query.next())
   {
        user_group_list << query.value(0).toString();
        wordList1 << query.value(0).toString();
   }

   QCompleter *completer = new QCompleter(wordList1, this);
   completer->setCaseSensitivity(Qt::CaseInsensitive);
   completer->setCompletionMode(QCompleter::PopupCompletion);
   lineedit_login_username->setCompleter(completer);


   connect(button_login_ok,SIGNAL(clicked()),this,SLOT(account_confirm()));
   ask_login_dialog->exec();
}

void MainWindow::account_confirm()
{
    login_name=lineedit_login_username->text();
    login_password=lineedit_login_password->text();
    int login_name_len=login_name.size();
    login_name_len=login_name_len*login_name_len*login_name_len;
    //QString login_password_true=login_name+QString::number(login_name_len,10);
    QString login_password_true=QString::number(login_name_len,10);
    qDebug()<<login_password_true;
    if ( login_password_true != login_password )
    {
        QMessageBox::about(this, tr("Warning"), tr("Password is Wrong"));
        login_name="Guest";
        login_password="Guest125";
        ui->pushButton_start->setDisabled(true);
    }else
    {
        QString config_file="./cloud_server_config.ini";
        QSettings settings(config_file,QSettings::IniFormat);
        settings.setValue("login_name",login_name);
        settings.setValue("login_password",login_password);
        ui->pushButton_start->setDisabled(false);
    }



    account_function();

    ask_login_dialog->close();
    show_board();


}

void MainWindow::account_function()
{
    if ( login_name == "global_user" )
    {
        qDebug()<<"global_user";
        ui->checkBox_dealy_task->setCheckable(true);
        delay_user_team_combobox->clear();
        delay_user_team_combobox->addItem("*");
        delay_user_team_combobox->setEditable(true);
        login_level=3;
    }else
    {
        if ( user_group_list.indexOf(login_name,0) >= 0 )
        {
            ui->checkBox_dealy_task->setCheckable(true);
            delay_user_team_combobox->clear();
            delay_user_team_combobox->addItem(login_name);
            delay_user_team_combobox->setEditable(false);
            login_level=2;
        }else
        {
            ui->checkBox_dealy_task->setChecked(false);
            ui->checkBox_dealy_task->setCheckable(false);
            login_level=1;
        }
    }
}


void MainWindow::act_enter_debug_mode()
{
    ask_debug_config=new QDialog;
    ask_debug_config->setWindowTitle("Please Enter staf ip ");
    button_ok=new QPushButton("OK");
    lineedit_ip=new QLineEdit;
    QGridLayout *gridLayout_ask_dialog=new QGridLayout(ask_debug_config);
    gridLayout_ask_dialog->addWidget(lineedit_ip,0,0,1,1);
    gridLayout_ask_dialog->addWidget(button_ok,0,1,1,2);
    connect(button_ok,SIGNAL(clicked()),this,SLOT(save_debug_config()));
    ask_debug_config->show();
    qDebug()<<staf_ip;
    if (enter_debug_mode->isChecked())
    {
        lineedit_ip->setText("10.38.36.178");
        ui->pushButton_update->setHidden(false);
        ui->checkBox_ignore_offline->setHidden(false);
        ui->checkBox_console_log->setHidden(false);
        task_tree->setColumnHidden(2,false);
        ltk_image_tree->setColumnHidden(1,false);
        //task_tree->setColumnHidden(3,false);
        debug_mode=true;
        stop_socket_connection=true;
    }else
    {
        lineedit_ip->setText("10.38.34.91");
        ui->pushButton_update->setHidden(true);
        ui->checkBox_ignore_offline->setHidden(true);
        ui->checkBox_console_log->setHidden(true);
        task_tree->setColumnHidden(2,true);
        ltk_image_tree->setColumnHidden(1,true);
        debug_mode=false;
        stop_socket_connection=false;
    }
}

void MainWindow::save_debug_config()
{
    QString register_gui_cmd="~/ltk_cloud_server/STAFEnv.sh staf "+staf_ip+" ltktest GUIREGISTER";
    QString unregister_gui_cmd="~/ltk_cloud_server/STAFEnv.sh staf "+staf_ip+" ltktest GUIUNREGISTER";
    int system_result;
    system_result=system(unregister_gui_cmd.toLatin1().data());
    if ( system_result != 0 )
    {
        QMessageBox::about(this,tr("STAF"),tr("STAF gui register/unregister fail"));
    }

    staf_ip=lineedit_ip->text();

    system_result=system(register_gui_cmd.toLatin1().data());
    if ( system_result != 0 )
    {
        QMessageBox::about(this,tr("STAF"),tr("STAF gui register/unregister fail"));
    }
    enter_debug_mode->setText("Debug in"+staf_ip);
    clear_config();
    ask_debug_config->close();


}


void MainWindow::act_gui_delay_time()
{
    qDebug()<<"1";
    ask_delay_time=new QDialog;
    ask_delay_time->setWindowTitle("Please Fill delay time ");
    button_delay_ok=new QPushButton("OK");
    lineedit_delay_time=new QLineEdit;
    qDebug()<<"2";
    QGridLayout *gridLayout_ask_dialog=new QGridLayout(ask_delay_time);
    gridLayout_ask_dialog->addWidget(lineedit_delay_time,0,0,1,1);
    gridLayout_ask_dialog->addWidget(button_delay_ok,0,1,1,2);
    qDebug()<<"3";
    connect(button_delay_ok,SIGNAL(clicked()),this,SLOT(save_delay_time()));
    ask_delay_time->show();
    lineedit_delay_time->setText(gui_delay_time_str);

    if (gui_delay_time->isChecked())
    {
        use_gui_delay=true;
    }else
    {
        use_gui_delay=false;
    }
}

void MainWindow::save_delay_time()
{
    gui_delay_time_str=lineedit_delay_time->text();
    ask_delay_time->close();
}

void MainWindow::create_menu()
{



    show_hide_board = new QAction(tr("Filter the un-available Board"),this);
    show_hide_board->setIcon(QIcon("./image/un_filter_big.png"));
    show_hide_board->setStatusTip(tr("Filter the un-available Board"));
    connect(show_hide_board,SIGNAL(triggered()),this,SLOT(show_hide_unavailable_board()));

    reflash = new QAction(tr("Regist STAF"),this);
    reflash->setIcon(QIcon("./image/paste.png"));
    reflash->setStatusTip(tr("Regist Server GUI to STAF"));
    connect(reflash,SIGNAL(triggered()),this,SLOT(register_gui()));


    about = new QAction(tr("About"),this);
    about->setStatusTip(tr("show version"));
    connect(about,SIGNAL(triggered()),this,SLOT(show_version()));

    start = new QAction(tr("start"),this);
    start->setIcon(QIcon("./image/run.png"));
    start->setStatusTip(tr("start test"));
    connect(start,SIGNAL(triggered()),this,SLOT(start_test()));

    gui_delay_time = new QAction("GUI_delay",this);
    gui_delay_time ->setIcon(QIcon("./image/database.png"));
    gui_delay_time ->setStatusTip(tr("GUI_delay"));
    connect(gui_delay_time ,SIGNAL(triggered()),this,SLOT(act_gui_delay_time()));
    gui_delay_time ->setCheckable(true);

    QString debug_ip="Debug in"+staf_ip;
    enter_debug_mode = new QAction(debug_ip,this);
    enter_debug_mode->setIcon(QIcon("./image/config.png"));
    enter_debug_mode->setStatusTip(tr("debug"));
    connect(enter_debug_mode,SIGNAL(triggered()),this,SLOT(act_enter_debug_mode()));
    enter_debug_mode->setCheckable(true);

    QString user_info="User_name:"+login_name;
    user_login = new QAction(user_info,this);
    user_login->setIcon(QIcon("./image/User.png"));
    user_login->setStatusTip(tr("Login"));
    connect(user_login,SIGNAL(triggered()),this,SLOT(act_user_login()));

    use_mysql = new QAction(tr("use mysql"),this);
    use_mysql->setIcon(QIcon("./image/Computer_On.png"));
    use_mysql->setStatusTip(tr("mysql"));
    connect(use_mysql,SIGNAL(triggered()),this,SLOT(check_mysql_use()));
    //use_mysql->setCheckable(true);

    reload_blf_map = new QAction(tr("reload_blf_map"),this);
    reload_blf_map->setStatusTip(tr("reload blf map"));
    connect(reload_blf_map,SIGNAL(triggered()),this,SLOT(act_reload_blf_map()));

    reload_ltk_map = new QAction(tr("reload_ltk_map"),this);
    reload_ltk_map->setStatusTip(tr("reload ltk map"));
    connect(reload_ltk_map,SIGNAL(triggered()),this,SLOT(act_reload_ltk_map()));

    menu_bar = ui->menuBar->addMenu("File");
    menu_bar->addAction(reload_blf_map);
    menu_bar->addAction(reload_ltk_map);

    menu_bar = ui->menuBar->addMenu("Action");
    menu_bar->addAction(reflash);

    menu_bar = ui->menuBar->addMenu("About");
    menu_bar->addAction(about);
}





void MainWindow::create_tool_bar()
{
    //toolBar->addAction(reflash);
    ui->mainToolBar->addAction(start);
    ui->mainToolBar->addAction(reflash);
    ui->mainToolBar->addAction(show_hide_board);
    ui->mainToolBar->addAction(use_mysql);
    ui->mainToolBar->addAction(gui_delay_time);
    ui->mainToolBar->addAction(enter_debug_mode);
    ui->mainToolBar->addAction(user_login);

}


void MainWindow::check_mysql_use()
{
    if (use_mysql_database)
    {
        qDebug()<<"disable sql";
        use_mysql->setText("Disable MySql");
        use_mysql->setIcon(QIcon("./image/Computer_Off.png"));
        use_mysql_database=false;
    }else
    {
        qDebug()<<"enable sql";
        use_mysql->setIcon(QIcon("./image/Computer_On.png"));
        use_mysql->setText("Enable MySql");
        use_mysql_database=true;
    }

}

void MainWindow::show_version()
{
    QString build_version="Unknow";
#ifdef ADD_VERSION
    build_version=ADD_VERSION;

    //char* baranch = BRANCH_FLAG;

    //char* baranch = "MASTER"

    //build_version="MASTER"
#endif
    QMessageBox::about(this,tr("Cloud Server Version:"),build_version);
}


void MainWindow::time_update()
{
    qDebug()<<"############Timer###########";
    update_status();
    // update_result->start(2000);
}

void MainWindow::show_package_tree()
{
    qDebug()<<"show package tree";
    ltk_package_tree->clear();

    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForCStrings(codec);

    QFile file("daily_branch_map.txt");
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug()<<"Can't open the file!"<<endl;
    }
    QString file_text;
    while(!file.atEnd()) {
        QByteArray line = file.readLine();
        QString str(line);
        file_text=file_text+str;
        //qDebug()<< "TTTT"<<str;
    }

    QStringList package_by_day=file_text.split("\n");
    int search_package_before_day=2; //2 means least,
    //for (int i=0;i<(package_by_day.count()-1);i++)
    for (int i=(package_by_day.count()-2);i>=0;i--)
    {
        QTreeWidgetItem* package_day=new QTreeWidgetItem(ltk_package_tree);

        QStringList package_by_day_branch1=package_by_day.at(i).split(":");
        QStringList package_by_day_branch2=package_by_day.at(i).split(":").at(1).split(";");
        package_day->setText(0,package_by_day_branch1.at(0));
        package_day->setText(1,"package_by_day");
        //package_day->setDisabled(true);
        bool package_selected=false;
        for (int a=0;a<package_by_day_branch2.count();a++)
        {
            QTreeWidgetItem* package_day_branch=new QTreeWidgetItem;
            package_day_branch->setText(0,package_by_day_branch2.at(a));
            package_day_branch->setText(1,package_by_day_branch1.at(0));
            //package_day_branch->setDisabled(false);
            package_day->addChild(package_day_branch);
            if (i==(package_by_day.count()-search_package_before_day) && (package_by_day_branch2.at(a).indexOf(platform_map[ui->comboBox_platform->currentText()]) >=0) && (! package_selected) )
            {
                package_day_branch->setSelected(true);
                package_selected=true;
                selected_case=package_by_day_branch1.at(0);
                selected_package_item=package_day_branch;
            }
        }
        if (i==(package_by_day.count()-search_package_before_day))
        {package_day->setExpanded(true);}
        if (! package_selected)
        {search_package_before_day++;}
    }
    //ltk_package_tree->sortItems(0,Qt::DescendingOrder);
    qDebug()<<"show package tree end";

}









void MainWindow::show_image_tree()
{
    qDebug()<<"show image tree";

    ltk_image_tree->clear();
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForCStrings(codec);

    QFile file("daily_image_map.txt");
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug()<<"Can't open the file!"<<endl;
    }
    QString file_text;
    while(!file.atEnd()) {
        QByteArray line = file.readLine();
        QString str(line);
        file_text=file_text+str;
        //qDebug()<< "TTTT"<<str;
    }

    QStringList image_by_day=file_text.split("\n");
    QTreeWidgetItem* image_day;
    for (int i=0;i<(image_by_day.count()-1);i++)
    {
        bool add_to_tree=false;
        if ( image_by_day.at(i).indexOf(platform_map[platform]) >= 0  )
        { add_to_tree=true; }
        if ( platform_map[platform] == "pxa1936" && image_by_day.at(i).split("pxa1936").count() == 2  )
        {
            add_to_tree=false;
            qDebug()<<"1908_image_named_1936";
        }
        if ( platform_map[platform] == "pxa1936"  )
        {
            qDebug()<<"1908_image_named_1936"<<image_by_day.at(i)<<image_by_day.at(i).contains("pxa1936");
        }
        if ( add_to_tree )
        {
            image_day=new QTreeWidgetItem(ltk_image_tree);

            QStringList image_by_day_branch1=image_by_day.at(i).split("=");
            QStringList image_by_day_branch2=image_by_day.at(i).split("=").at(1).split(";");
            image_day->setText(0,image_by_day_branch1.at(0));
            //image_day->setText(1,"version");
            QString blf_string_by_day="version---";
            for (int a=0;a<image_by_day_branch2.count();a++)
            {

                QString image=image_by_day_branch2.at(a).split(":").at(0);
                if (image.indexOf(platform_map[platform]) >= 0)
                {
                    QTreeWidgetItem* image_image=new QTreeWidgetItem;
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
                    image_image->setExpanded(true);
                }
            }
            image_day->setText(1,blf_string_by_day);
            //image_day->setExpanded(true);

        }

    }
    qDebug()<<"sort_image_tree";
    ltk_image_tree->sortItems(0,Qt::DescendingOrder);
    //ltk_image_tree->topLevelItem(0)->child(0)->child(0)->setSelected(true);
    ltk_image_tree->setCurrentItem(ltk_image_tree->topLevelItem(0)->child(0)->child(0));
    //QTreeWidgetItem* cc=ltk_image_tree->topLevelItem(0)->child(0)->child(0);
    //emit ltk_image_tree->itemClicked(cc,0);
}









void MainWindow::show_package()
{
    if ( ui->checkBox_last_package->isChecked())
    {
        show_package_tree();
        ltk_package_tree->setDisabled(true);
    }
    else
    {
        ltk_package_tree->setDisabled(false);
        toolBox->setCurrentIndex(2);
    }
}


void MainWindow::check_pakcage_tree(QTreeWidgetItem* select_package,int select_col)
{
    selected_case=select_package->text(1)+"/"+select_package->text(select_col);
    qDebug()<<selected_case;
}





void MainWindow::open_dialog_for_special_image(QTreeWidgetItem *select_image, int select_col)
{
    QString image_detail=select_image->text(1);
    image_platform=platform_map[ui->comboBox_platform->currentText()];

    //"http://10.38.116.40/autobuild/android/pxa988/2014-05-15_pxa988-kk4.4/pxa1U88dkb_def/blf/HLN2_Nontrusted_LPDDR3_2G_Hynix.blf

    if (image_detail.indexOf("version") >=0 || image_detail.indexOf("platform_def") >= 0 )
    { qDebug() << "skip menu cerate"; } else
    {
        QStringList image_link_item_list=image_detail.split(";");

        link_for_blf="http://10.38.116.40/autobuild/android/"+image_platform+"/"+image_link_item_list.at(0)+"/"+image_link_item_list.at(1)+"/blf/"+image_link_item_list.at(2);
        //QString download_cmd="wget --http-user=wsun --http-passwd=marvell888 "+link_for_blf;
        //int try_down_load=system(download_cmd.toLatin1().data());
        //if (try_down_load != 0)
        QString image_version_date=image_link_item_list.at(0);
        if ( image_version_date > "2014-05-23" && ui->comboBox_platform->currentText() == "EDEN" )
        {
            link_for_blf="http://10.38.116.40/autobuild/android/"+image_platform+"/"+image_link_item_list.at(0)+"/"+image_link_item_list.at(1)+"/flash/blf/"+image_link_item_list.at(2);
        }
        if ( image_link_item_list.at(0).indexOf("1908") >= 0 )
        {
            link_for_blf="http://10.38.116.40/autobuild/android/pxa1908/"+image_link_item_list.at(0)+"/"+image_link_item_list.at(1)+"/flash/blf/"+image_link_item_list.at(2);
        }
        if ( image_link_item_list.at(0).indexOf("1908") >= 0 && image_version_date > "2014-10-30" )
        {
            link_for_blf="http://10.38.116.40/autobuild/android/pxa1908/"+image_link_item_list.at(0)+"/"+image_link_item_list.at(1)+"/flash/"+image_link_item_list.at(2);
        }
        if ( image_link_item_list.at(0).indexOf("1936") >= 0 && image_version_date > "2014-10-30" )
        {
            link_for_blf="http://10.38.116.40/autobuild/android/"+image_platform+"/"+image_link_item_list.at(0)+"/"+image_link_item_list.at(1)+"/flash/"+image_link_item_list.at(2);
        }
        if ( image_link_item_list.at(0).indexOf("pxa1936-lp5.0_k314_alpha2") >= 0 || image_link_item_list.at(0).indexOf("pxa1936-lp5.0_k314_beta1") >= 0   || image_link_item_list.at(0).indexOf("pxa1936-lp5.1") >= 0 )
        {
            link_for_blf="http://10.38.116.40/autobuild/android/pxa1936/"+image_link_item_list.at(0)+"/"+image_link_item_list.at(1)+"/flash/"+image_link_item_list.at(2);
        }
        blf_selected=image_link_item_list.at(2);
        QFile::remove(image_link_item_list.at(2));

        qDebug()<<image_detail;
        qDebug()<<link_for_blf;
        if(qApp->mouseButtons()==Qt::RightButton){
            QMenu dialog_menu("show_menu");
            QAction *open_dialog = new QAction(tr("Test special Image"),this);
            dialog_menu.addAction(open_dialog);
            connect(open_dialog,SIGNAL(triggered()),this,SLOT(image_dialog_open()));

            dialog_menu.exec(QCursor::pos());
        }
    }

}


void MainWindow::image_dialog_open()
{
    Dialog w;
    w.tempDirDateTime=QDateTime::currentDateTime().toString(Qt::ISODate).replace(QRegExp("[-:]"),"_")+"tmp";
    w.blf_folder=blf_modified_folder;
    w.ParseFile(link_for_blf);
    int ret=w.exec();
    qDebug()<<"link for blf"<<link_for_blf;

    QMap<QString,QString> config2=w.getImageConfig2();
    if(config2.size()){
        image_modify=config2["modified"];
        special_image_ip=config2["location2"];
        specail_image_path=config2["path2"];
        replaced_image=config2["image2"];
        blf_modified=config2["blf_modified"];
        if ( blf_modified == "yes" )
        {
            blf_modified_name_list<<blf_selected;
            blf_modified_path=config2["blf_modified_path"];
        }
        qDebug()<<"blf_modified:"<<blf_modified;
    }else
    {
        qDebug()<<__LINE__<<__FILE__<<"empty";
    }

    //return ret;


}


void MainWindow::select_board()
{
    for (int i=0+filter_row_num;i< (ui->tableWidget_board->rowCount());i++)
    {
        ui->tableWidget_board->item(i,0)->setCheckState(Qt::Checked);
    }
}

void MainWindow::unselect_board()
{
    for (int i=0+filter_row_num;i< (ui->tableWidget_board->rowCount());i++)
    {
        ui->tableWidget_board->item(i,0)->setCheckState(Qt::Unchecked);
    }
}

void MainWindow::board_lock_menu(QTableWidgetItem* item_menu)
{
    qDebug()<<"lock/unlock board";
     if(qApp->mouseButtons()==Qt::RightButton){
         qDebug()<<"create lock menu";
         QMenu lock_menu("show_menu");


         QAction *copy_str = new QAction(tr("Copy String"),this);
         lock_menu.addAction(copy_str);
         connect(copy_str,SIGNAL(triggered()),this,SLOT(copy_str_to_clipboard()));
         clipboard_str=item_menu->text();

         QAction *lock_board = new QAction(tr("Lock_board"),this);
         lock_menu.addAction(lock_board);
         connect(lock_board,SIGNAL(triggered()),this,SLOT(lock_board()));
         QAction *unlock_board = new QAction(tr("Unlock_board"),this);
         lock_menu.addAction(unlock_board);
         connect(unlock_board,SIGNAL(triggered()),this,SLOT(unlock_board()));

         QAction *select_all_board = new QAction(tr("Select_all_board"),this);
         lock_menu.addAction(select_all_board);
         connect(select_all_board,SIGNAL(triggered()),this,SLOT(select_board()));

         QAction *unselect_all_board = new QAction(tr("Unselect_all_board"),this);
         lock_menu.addAction(unselect_all_board);
         connect(unselect_all_board,SIGNAL(triggered()),this,SLOT(unselect_board()));



         lock_menu.setStyleSheet("font:9pt;text-align:center;");
         lock_menu.exec(QCursor::pos());
     }
}

void MainWindow::lock_board()
{
    check_board_list();
    for (int i=0;i<board_list.count();i++)
    {
        QString lock_board_cmd="~/ltk_cloud_server/STAFEnv.sh staf "+staf_ip+" ltktest lock board_id "+board_list.at(i);
        int system_result=system(lock_board_cmd.toLatin1().data());
    }
    clear_config();
}

void MainWindow::unlock_board()
{
    check_board_list();
    for (int i=0;i<board_list.count();i++)
    {
        QString lock_board_cmd="~/ltk_cloud_server/STAFEnv.sh staf "+staf_ip+" ltktest unlock board_id "+board_list.at(i);
        int system_result=system(lock_board_cmd.toLatin1().data());
    }
    clear_config();
}

void MainWindow::tree_menu(QTreeWidgetItem* item_menu)
{
    selected_item=item_menu->text(0);
    selected_tree_item=item_menu;
    qDebug()<<"view menu"<<selected_item;
    //QMenu item_menu("show_menu");;
    if(qApp->mouseButtons()==Qt::RightButton){

        QMenu item_menu("show_menu");

        QAction *copy_str = new QAction(tr("Copy String"),this);
        item_menu.addAction(copy_str);
        connect(copy_str,SIGNAL(triggered()),this,SLOT(copy_str_to_clipboard()));
        clipboard_str=selected_tree_item->text(0);

        if (selected_tree_item->text(2).indexOf("task")>=0)
        {
            QAction *delete_task = new QAction(tr("Delete task"),this);
            item_menu.addAction(delete_task);
            connect(delete_task,SIGNAL(triggered()),this,SLOT(delete_staf_task()));
        }
        /*
        if (selected_tree_item->text(2).indexOf("task")>=0 && ui->checkBox_manual_reboot->isChecked() )
        {
            QAction *reboot_board = new QAction(tr("Reboot Board"),this);
            item_menu.addAction(reboot_board);
            connect(reboot_board,SIGNAL(triggered()),this,SLOT(staf_reboot_board()));
        }
        */
        if (selected_tree_item->text(2).indexOf("smb:")>=0)
        {
            QAction *open_log_link = new QAction(tr("Open Log Link"),this);
            item_menu.addAction(open_log_link);
            connect(open_log_link,SIGNAL(triggered()),this,SLOT(open_link()));
        }

        if (selected_tree_item->text(2).indexOf("job_for")>=0 )
        {
            QAction *cancel_job_act = new QAction(tr("Cancel_Board_Task"),this);
            item_menu.addAction(cancel_job_act);
            connect(cancel_job_act,SIGNAL(triggered()),this,SLOT(cancel_board()));
        }

        if (selected_tree_item->text(2).indexOf("delay_task:")>=0 )
        {
            QAction *cancel_task_act = new QAction(tr("Cancel_Delay_Task"),this);
            item_menu.addAction(cancel_task_act);
            connect(cancel_task_act,SIGNAL(triggered()),this,SLOT(cancel_task()));
        }

        if (selected_tree_item->text(2).indexOf("normal_task:")>=0 )
        {
            QAction *cancel_task_act = new QAction(tr("Cancel_Normal_Task"),this);
            item_menu.addAction(cancel_task_act);
            connect(cancel_task_act,SIGNAL(triggered()),this,SLOT(cancel_task()));
        }

        item_menu.setStyleSheet("font:9pt;text-align:center;");
        //item_menu.setFixedSize(100,20);
        item_menu.exec(QCursor::pos());
    }
}


void MainWindow::cancel_task()
{
    qDebug()<<"cancel_task"<<selected_tree_item->text(0);
    bool task_include_dealy=false;
    if ( selected_tree_item->text(2).indexOf("delay_task:") >=0 )
    { task_include_dealy=true;}

    QString cancel_task_name=selected_tree_item->text(0);
    if (task_include_dealy)
    {
        QString delay_task_entry=" ENTRY_IP \""+task_board_entry[cancel_task_name]+"\"";
        QString staf_cancel_task_name=" TASK_NAME \""+cancel_task_name+"\"";
        QString delay_cancel_cmd=work_dir+"/STAFEnv.sh staf "+staf_ip+" LTKTEST CANCEL"+delay_task_entry+staf_cancel_task_name;
        for (int i=1;i<50;i++)
        {
            int delay_cancel_result=system(delay_cancel_cmd.toLatin1().data());
            qDebug()<<"Cancel job loop"<<i<<delay_cancel_cmd;
            if (delay_cancel_result != 0 )
            {break;}
        }
    }

    for (int i=0;i<selected_tree_item->childCount();i++)
    {
        cancel_job(selected_tree_item->child(i));
    }
    selected_tree_item->setText(1,"Finised");

}


void MainWindow::cancel_board()
{
 cancel_job(selected_tree_item);
}

void MainWindow::cancel_job(QTreeWidgetItem* board)
{
    qDebug()<<"cancel_job"<<board->text(0);
    QString board_id=board->text(0);
    QString board_status=board->text(1);
    bool pending_task;
    if (board_status=="Board_Pending")
    {pending_task=true;}else
    {pending_task=false;}

    if ( board_status !="Board_Request_Fail" && board_status !="Job_Canceled" && board_status.indexOf("Job is canceled") < 0 )
    {
        if (!pending_task)
        {
            bool job_finished=true;
            for (int i=0;i<board->childCount();i++)
            {
                QString case_status=board->child(i)->text(1);
                if (case_status != "NULL")
                {
                    job_finished=job_finished && true;
                }else
                {
                    job_finished=job_finished && false;
                }

            }
            if (! job_finished )//if job finished , the job can't be canceled
            {
                QString staf_cancel_command=work_dir+"/STAFEnv.sh staf "+staf_ip+" LTKTEST CANCEL BOARD_ID "+board_id;

                QProcess *cmd_board_cancel=new QProcess();
                cmd_board_cancel->start(staf_cancel_command);
                while((cmd_board_cancel->state() == QProcess::Running) || (cmd_board_cancel->state() == QProcess::Starting) )
                {
                    QEventLoop eventloop;
                    QTimer::singleShot(100,&eventloop,SLOT(quit()));
                    eventloop.exec();
                }
                if ( cmd_board_cancel->exitCode() == 0 )
                { board->setText(1,"Job_Cancel_start");}
                else
                { board->setText(1,"Job_Cancel_Fail");}


                /*
                int cancel_result=system(staf_cancel_command.toLatin1().data());
                if (cancel_result==0)
                {
                    board->setText(1,"Job_Cancel_Start");
                }else
                {
                    board->setText(1,"Job_Cancel_Fail");
                }
                */
            }else{qDebug()<<"do nothing for job canecl because job finished";}
        }else
        {
            QStringList cancel_detail_list=board->text(2).split(":");
            QString entry_str,staf_cancel_task_name;
            if (cancel_detail_list.count()>2)
            {
                entry_str=cancel_detail_list.at(2);
                staf_cancel_task_name=cancel_detail_list.at(1);
            }
            entry_str=" ENTRY_IP \""+entry_str+"\"";
            staf_cancel_task_name=" TASK_NAME \""+staf_cancel_task_name+"\"";
            QString staf_cancel_command=work_dir+"/STAFEnv.sh staf "+staf_ip+" LTKTEST CANCEL"+entry_str+staf_cancel_task_name;

            QProcess *cmd1=new QProcess();
            cmd1->start(staf_cancel_command);
            while((cmd1->state() == QProcess::Running) || (cmd1->state() == QProcess::Starting) )
            {
                QEventLoop eventloop;
                QTimer::singleShot(100,&eventloop,SLOT(quit()));
                eventloop.exec();
            }
            if ( cmd1->exitCode() == 0 )
            { board->setText(1,"Job_Canceled");}
            else
            { board->setText(1,"Job_Cancel_Fail");}

            /*
            int cancel_result=system(staf_cancel_command.toLatin1().data());
            if (cancel_result==0)
            {
                board->setText(1,"Job_Canceled");
            }else
            {
                board->setText(1,"Job_Cancel_Fail");
            }
            */
        }
    }else{qDebug()<<"do nothing for job canecl";}
}



void MainWindow::copy_str_to_clipboard()
{
    qt_clipboard->setText(clipboard_str);
}

void MainWindow::open_link()
{
    QUrl open_url(selected_tree_item->text(2),QUrl::TolerantMode);
    bool url_open_success=QDesktopServices::openUrl(open_url);
    if ( ! url_open_success )
    {
        QMessageBox::about(this, tr("Warning"),
                    tr("<h2>Log link open fail</h2>"
                       "<p>Log link open fail!."));
    }
}


void MainWindow::staf_reboot_board()
{
    QString reboot_board_id=selected_tree_item->text(0);
    qDebug()<<"reboot board :"<<reboot_board_id;
}

void MainWindow::delete_staf_task()
{
    //selected_tree_item->takeChildren();
    int index=task_tree->indexOfTopLevelItem(selected_tree_item);
    task_tree->takeTopLevelItem(index);
    QString del_task_name=selected_tree_item->text(0);
    ui->tabWidget->removeTab(ui->tabWidget->indexOf(task_tab_map[del_task_name]));
    task_map.remove(del_task_name);
}

void MainWindow::update_status()
{
    qDebug()<<"!!!update status";
    if (! progress_lock)
    {
        ui->pushButton_update->setDisabled(true);
        QMap<QString,QString> staf_task_result;
        qDebug()<<"!!!create update process";
        delete Gcmd;Gcmd=NULL;
        Gcmd = new QProcess();
        QString task_list_string="\""+task_list_all.join("&&")+"\"";
        waitProcess(work_dir+"/STAFEnv.sh staf "+staf_ip+" LTKTEST LIST TASKNAME "+task_list_string,100);
        //waitProcess(work_dir+"/STAFEnv.sh staf "+staf_ip+" LTKTEST LIST jobs",10);
        tempString=Gcmd->readAll();
        QStringList task_list_tmp=tempString.split("{");
        QString task_result,temp_status;
        task_list_tmp.removeFirst();
        for (int i=0;i<task_list_tmp.count();i++)
        {
            QStringList case_result_list;
            bool one_job_finished=false;

            QString temp_task,temp_board,temp_case,case_temp_status,start_time,end_time,job_result,execution_time,job_name="Unknow";
            QStringList staf_temp=task_list_tmp.at(i).split("\n");
            staf_temp.removeLast();
            staf_temp.removeLast();
            for (int a=0;a<staf_temp.count();a++)
            {
                QString temp_string=staf_temp.at(a);
                if (temp_string.indexOf("(AUTO_GENERATE)taskname")>=0)
                { temp_task=temp_string.split(": ").at(1); }
                if (temp_string.indexOf("(AUTO_GENERATE)JobName")>=0)
                { job_name=temp_string.split(": ").at(1); }
                if (temp_string.indexOf("(AUTO_GENERATE)board_id")>=0)
                { temp_board=temp_string.split(": ").at(1); }
                if (temp_string.indexOf("(AUTO_DETECT)statusAll")>=0 )
                {
                  //QStringList temp_status_all1=temp_string.split(": ");
                  QStringList temp_status_all1=temp_string.split("(AUTO_DETECT)statusAll");
                  temp_status=temp_status_all1.at(temp_status_all1.count()-1);
                  QStringList temp_status_all=temp_status.split(";");
                  if ( temp_status_all.count() > 2 )
                  {
                    temp_status=temp_status_all.at(temp_status_all.count()-2);
                  }else
                  {
                      if ( temp_status_all.count() > 1 )
                      {
                          temp_status=temp_status_all.at(temp_status_all.count()-2);
                          temp_status=temp_status.replace("  ","");
                          temp_status=temp_status.replace(":","");
                      }else{
                      temp_status=temp_status_all.at(0);
                      }

                  }

                  if (temp_string.indexOf("Finish 1case(")>=0)
                  {
                      one_job_finished=true;
                      qDebug()<<"STATUS ALL:"<<temp_string;
                      QStringList finish_list1=temp_string.split("Finish 1case");
                      finish_list1.removeFirst();
                      for (int f=0;f<finish_list1.count();f++)
                      {
                          qDebug()<<"PPPPP"<<finish_list1.at(f);
                          QString temp=finish_list1.at(f).split(")").at(0);
                          temp=temp.replace("(","");
                          temp=temp.replace(" ","");
                          temp=temp.replace(")","");
                          case_result_list<<temp;
                      }

                  }


                }
                if (temp_string.indexOf("(AUTO_GENERATE)startTime")>=0)
                { start_time=temp_string.split(": ").at(1); }
                if (temp_string.indexOf("(AUTO_GENERATE)statusUpdateTime")>=0)
                { end_time=temp_string.split(": ").at(1); }
            }

            qDebug()<<"search the job which have reault";

            bool real_time_update=true;

            if ( real_time_update )
            {
                if ( task_list_tmp.at(i).indexOf("=Job Report=")>=0 )
                {
                QStringList out_put_result=task_list_tmp.at(i).split("=Job Report=").at(1).split("\n");
                //QStringList case_result_list;
                case_result_list.clear();
                out_put_result.removeFirst();
                out_put_result.removeLast();
                for (int b=0;b<out_put_result.count();b++)
                {
                    if (out_put_result.at(b).indexOf("TC_") >= 0 )
                    {
                        QString temp=out_put_result.at(b);
                        case_result_list<<temp.replace(" ","");
                    }
                    if (out_put_result.at(b).indexOf("Result:") >= 0 )
                    {
                        job_result=out_put_result.at(b).split(": ").at(1);
                    }
                    if (out_put_result.at(b).indexOf("Total Cost") >= 0 )
                    {
                        execution_time=out_put_result.at(b).split(": ").at(1);
                    }
                }
                }else
                {
                    QStringList out_put_result=task_list_tmp.at(i).split("\n");
                    for (int b=0;b<out_put_result.count();b++)
                    {
                        if ( out_put_result.at(b).indexOf("(AUTO_GENERATE)caseName") >= 0)
                        {
                            QString temp1=out_put_result.at(b).split(":").at(1);
                            QStringList case_id_list=temp1.replace(" ","").split("TC");
                            for (int c=1;c<case_id_list.count();c++)
                            {
                                QString temp1=case_id_list.at(c);
                                qDebug()<<"case name ori:"<<temp1;
                                temp1=temp1.replace(" ","");
                                temp1=temp1.split("-loop-").at(0);
                                temp1="TC"+temp1+":NULL";
                                qDebug()<<"case name add:"<<temp1;
                                case_result_list<<temp1;
                            }
                        }
                        if ( out_put_result.at(b).indexOf("(AUTO_DETECT)statusAll") >= 0)
                        {
                            QStringList temp_list=out_put_result.at(b).split(":");
                            temp_list.removeFirst();
                            QString temp=temp_list.join(":");
                            QStringList job_result_list=temp.split(";");
                            qDebug()<<"Job status:"<<temp.replace(" ","")<<"HHH"<<job_result_list.count();
                            if (job_result_list.count()>2)
                            {
                                job_result=job_result_list.at(job_result_list.count()-2);
                            }else
                            {
                                job_result="NULL";
                            }
                            QStringList case_result_list_check=out_put_result.at(b).split("Finish 1case");
                            if (case_result_list_check.count()>1)
                            {
                                for (int c=1;c<case_result_list_check.count();c++)
                                {
                                    QString temp=case_result_list_check.at(c).split(")").at(0);
                                            temp=temp.remove("(").remove(" ");
                                    QString temp_case_id_null=temp.split(":").at(0)+":NULL";
                                    int remove_list=case_result_list.indexOf(temp_case_id_null);
                                    if ( remove_list >= 0 )
                                    {
                                        case_result_list.removeAt(remove_list);
                                        qDebug()<<"finish 1case:"<<temp;
                                        case_result_list<<temp;
                                    }
                                }
                            }
                        }
                    }
                    execution_time="NULL";
                }


                qDebug()<<"update status############### "<<job_name;

                staf_task_map[temp_task+"-B-"+temp_board]=temp_status;
                staf_task_map[temp_task]=staf_task_map[temp_task]+temp_status;



                for (int c=0;c<case_result_list.count();c++)
                {
                    qDebug()<<case_result_list.at(c);
                    temp_case=case_result_list.at(c).split(":").at(0);
                    qDebug()<<case_result_list.at(c)<<"1";
                    case_temp_status=case_result_list.at(c).split(":").at(1);
                    //qDebug()<<"2";
                    staf_task_map[temp_task+"-B-"+temp_board+"-C-"+temp_case]=case_temp_status;

                    qDebug()<<temp_task+"-B-"+temp_board+"-C-"+temp_case;


                    if (task_map.find(temp_task+"-B-"+temp_board+"-C-"+temp_case) != task_map.end())
                    {
                        qDebug()<<"find table1"<<temp_task+"-B-"+temp_board+"-C-"+temp_case;
                        QTableWidget* table_for_case=staf_task_case_map_tab[temp_task+"-B-"+temp_board+"-C-"+temp_case]->tableWidget();
                        qDebug()<<"find table2";
                        task_map[temp_task+"-B-"+temp_board+"-C-"+temp_case]->setText(1,case_temp_status);

                        int table_row=staf_task_case_map_tab[temp_task+"-B-"+temp_board+"-C-"+temp_case]->row();
                        table_for_case->setItem(table_row,5,new QTableWidgetItem(start_time));
                        table_for_case->setItem(table_row,6,new QTableWidgetItem(end_time));

                        qDebug()<<"find version1"<<table_for_case->columnCount();
                        QString image_for_case=table_for_case->item(table_row,7)->text();
                        qDebug()<<"find version2";
                        QString ltk_for_case=table_for_case->item(table_row,8)->text();
                        QString log_for_case="smb://"+test_package_server_ip+"/daily_build/Cloud_Test/"+task_date+"/"+temp_task+"/"+job_name;

                        qDebug()<<"find version3";
                        staf_task_log_map_tab[temp_task+"-B-"+temp_board+"-C-"+temp_case]->setText(log_for_case);

                        qDebug()<<"update sql";

                        /*
                        mysql> describe TestResult_Cloud ;
                        +----------------+--------------+------+-----+---------+-------+
                        | Field          | Type         | Null | Key | Default | Extra |
                        +----------------+--------------+------+-----+---------+-------+
                        | Task           | varchar(256) | YES  |     | NULL    |       |
                        | Board          | varchar(128) | YES  |     | NULL    |       |
                        | Case_ID        | varchar(64)  | YES  |     | NULL    |       |
                        | Status         | varchar(128) | YES  |     | NULL    |       |
                        | Log_Link       | varchar(256) | YES  |     | NULL    |       |
                        | Start_Time     | varchar(64)  | YES  |     | NULL    |       |
                        | End_Time       | varchar(64)  | YES  |     | NULL    |       |
                        | Image_Version  | varchar(256) | YES  |     | NULL    |       |
                        | LTK_Version    | varchar(256) | YES  |     | NULL    |       |
                        | Job_Name       | varchar(256) | NO   |     | NULL    |       |
                        | Job_Status     | char(128)    | YES  |     | NULL    |       |
                        | Execution_time | char(128)    | YES  |     | NULL    |       |
                        +----------------+--------------+------+-----+---------+-------+
                        12 rows in set (0.00 sec)
                        */
                        QString update_str="UPDATE TestResult_Cloud SET ";



                        update_str+="Status='"+case_temp_status+"',";
                        update_str+="End_Time='"+end_time+"',";
                        update_str+="Status='"+case_temp_status+"',";
                        update_str+="Job_Status='"+job_result+"'";
                        update_str+=" where ";
                        update_str+="Case_ID='"+temp_case+"' and ";
                        update_str+="Job_Name='"+job_name+"';";

                        //insert into TestResult_Cloud values('task1','board1','TC_LCD_01','Pass','smb','2014','2014','beta1','20140520');

                        QString insert_str="insert into TestResult_Cloud values(";
                        insert_str+="'"+temp_task+"',";
                        insert_str+="'"+temp_board+"',";
                        insert_str+="'"+temp_case+"',";
                        insert_str+="'"+case_temp_status+"',";
                        insert_str+="'"+log_for_case+"',";
                        insert_str+="'"+start_time+"',";
                        insert_str+="'"+end_time+"',";
                        insert_str+="'"+image_for_case+"',";
                        insert_str+="'"+ltk_for_case+"',";
                        insert_str+="'"+job_name+"',";
                        insert_str+="'"+job_result+"',";
                        insert_str+="'"+execution_time+"');";



                        qDebug()<<"job_name"<<job_name;
                        QString insert_job_info_str="insert into Cloud_Broad_Info values(";
                        insert_job_info_str+="'"+job_name+"',";
                        QString board_job_id=job_name.replace(" ","");
                        qDebug()<<job_name<<board_job_id;
                        QStringList temp_board_job=board_job_id.split("-");
                        temp_board_job.removeLast();temp_board_job.removeLast();
                        if (temp_board_job.count()>1)
                        {board_job_id=temp_board_job.join("-");}
                        else
                        {board_job_id=temp_board_job.at(0); }
                        insert_job_info_str+="'"+staf_board_map[board_job_id].board_id+"',";
                        insert_job_info_str+="'"+staf_board_map[board_job_id].board_type+"',";
                        insert_job_info_str+="'"+staf_board_map[board_job_id].chip_name+"',";
                        insert_job_info_str+="'"+staf_board_map[board_job_id].chip_stepping +"',";
                        insert_job_info_str+="'"+staf_board_map[board_job_id].board_register_date+"',";
                        insert_job_info_str+="'"+staf_board_map[board_job_id].board_eco+"',";
                        insert_job_info_str+="'"+staf_board_map[board_job_id].lcd_resolution+"',";
                        insert_job_info_str+="'"+staf_board_map[board_job_id].lcd_screensize+"',";
                        insert_job_info_str+="'"+staf_board_map[board_job_id].ddr_type+"',";
                        insert_job_info_str+="'"+staf_board_map[board_job_id].ddr_size+"',";
                        insert_job_info_str+="'"+staf_board_map[board_job_id].emmc_type+"',";
                        insert_job_info_str+="'"+staf_board_map[board_job_id].emmc_size+"',";
                        insert_job_info_str+="'"+staf_board_map[board_job_id].rf_name+"',";
                        insert_job_info_str+="'"+staf_board_map[board_job_id].rf_type+"',";
                        insert_job_info_str+="'"+staf_board_map[board_job_id].dro+"',";
                        insert_job_info_str+="'"+staf_board_map[board_job_id].chip_type+"',";
                        insert_job_info_str+="'"+staf_board_map[board_job_id].machine+"');";

                        QString mysql_job_check="select * from Cloud_Broad_Info where Job_Name=\""+job_name+"\";";
                        QString mysql_job_updated_check="select * from TestResult_Cloud where Job_Name=\""+job_name+"\" and Case_ID=\""+temp_case+"\";";

                        QString latest_case_result=staf_task_case_map_tab[temp_task+"-B-"+temp_board+"-C-"+temp_case]->text();




                        if ( use_mysql_database )
                        {
                            QSqlQuery query;
                            query.exec(mysql_job_updated_check);
                            if ( query.numRowsAffected() == 0 )
                            {
                                query.exec(insert_str);
                            }else
                            {
                                query.exec(update_str);
                            }
                            query.exec(mysql_job_check);
                            if ( query.numRowsAffected() == 0 )
                            {
                            qDebug()<<"connect status"<<dbMySQL.isOpen()<<"MY_SQL_JOB_ADD"<<insert_job_info_str;
                            query.exec(insert_job_info_str);
                            }
                            qDebug()<<"SQL:"<<insert_job_info_str;
                            qDebug()<<"SQL:"<<update_str;
                            qDebug()<<"SQL:"<<insert_str;
                        }


                        staf_task_case_map_tab[temp_task+"-B-"+temp_board+"-C-"+temp_case]->setText(case_temp_status);

                        if (case_temp_status.indexOf("Pass") >= 0 )
                        {
                            task_map[temp_task+"-B-"+temp_board+"-C-"+temp_case]->setForeground(1,QBrush(QColor(Qt::green)));
                            staf_task_case_map_tab[temp_task+"-B-"+temp_board+"-C-"+temp_case]->setTextColor("green");
                            //staf_task_log_map_tab[temp_task+"-B-"+temp_board+"-C-"+temp_case]->setTextColor("green");

                        }
                        if (case_temp_status.indexOf("Fail") >= 0 )
                        {
                            task_map[temp_task+"-B-"+temp_board+"-C-"+temp_case]->setText(1,case_temp_status);
                            staf_task_case_map_tab[temp_task+"-B-"+temp_board+"-C-"+temp_case]->setText(case_temp_status);
                            task_map[temp_task+"-B-"+temp_board+"-C-"+temp_case]->setForeground(1,QBrush(QColor(Qt::red)));
                            staf_task_case_map_tab[temp_task+"-B-"+temp_board+"-C-"+temp_case]->setTextColor("red");
                        }
                        //task_map.remove(temp_task+"-B-"+temp_board+"-C-"+temp_case);
                    }else
                    {qDebug()<<temp_task+"-B-"+temp_board+"-C-"+temp_case<<"not in task map";}
                }
            }





            if ((task_list_tmp.at(i).indexOf("=Job Report=")>=0 || one_job_finished) && (! real_time_update) )
            {
                if ( task_list_tmp.at(i).indexOf("=Job Report=")>=0 )
                {
                QStringList out_put_result=task_list_tmp.at(i).split("=Job Report=").at(1).split("\n");
                //QStringList case_result_list;
                case_result_list.clear();
                out_put_result.removeFirst();
                out_put_result.removeLast();
                for (int b=0;b<out_put_result.count();b++)
                {
                    if (out_put_result.at(b).indexOf("TC_") >= 0 )
                    {
                        QString temp=out_put_result.at(b);
                        case_result_list<<temp.replace(" ","");
                    }
                    if (out_put_result.at(b).indexOf("Result:") >= 0 )
                    {
                        job_result=out_put_result.at(b).split(": ").at(1);
                    }
                    if (out_put_result.at(b).indexOf("Total Cost") >= 0 )
                    {
                        execution_time=out_put_result.at(b).split(": ").at(1);
                    }
                }
                }


                qDebug()<<"update status############### "<<job_name;

                staf_task_map[temp_task+"-B-"+temp_board]=temp_status;
                staf_task_map[temp_task]=staf_task_map[temp_task]+temp_status;



                for (int c=0;c<case_result_list.count();c++)
                {
                    //qDebug()<<case_result_list.at(c);
                    temp_case=case_result_list.at(c).split(":").at(0);
                    case_temp_status=case_result_list.at(c).split(":").at(1);
                    staf_task_map[temp_task+"-B-"+temp_board+"-C-"+temp_case]=case_temp_status;

                    //qDebug()<<temp_task+"-B-"+temp_board+"-C-"+temp_case+"AAA"+case_temp_status+temp_case;


                    if (task_map.find(temp_task+"-B-"+temp_board+"-C-"+temp_case) != task_map.end())
                    {
                        qDebug()<<"find table1";
                        QTableWidget* table_for_case=staf_task_case_map_tab[temp_task+"-B-"+temp_board+"-C-"+temp_case]->tableWidget();
                        qDebug()<<"find table2";
                        task_map[temp_task+"-B-"+temp_board+"-C-"+temp_case]->setText(1,case_temp_status);

                        int table_row=staf_task_case_map_tab[temp_task+"-B-"+temp_board+"-C-"+temp_case]->row();
                        table_for_case->setItem(table_row,5,new QTableWidgetItem(start_time));
                        table_for_case->setItem(table_row,6,new QTableWidgetItem(end_time));

                        qDebug()<<"find version1"<<table_for_case->columnCount();
                        QString image_for_case=table_for_case->item(table_row,7)->text();
                        qDebug()<<"find version2";
                        QString ltk_for_case=table_for_case->item(table_row,8)->text();
                        QString log_for_case="smb://"+test_package_server_ip+"/daily_build/Cloud_Test/"+task_date+"/"+temp_task+"/"+job_name;

                        qDebug()<<"find version3";
                        staf_task_log_map_tab[temp_task+"-B-"+temp_board+"-C-"+temp_case]->setText(log_for_case);

                        qDebug()<<"update sql";

                        //Task  | Board  | Case_ID   | Status | Log_Link | Start_Time | End_Time | Image_Version | LTK_Version
                        QString update_str="UPDATE TestResult_Cloud SET ";
                        update_str+="Status='"+case_temp_status+"',";
                        update_str+="Log_Link='"+log_for_case+"',";
                        update_str+="Start_Time='"+start_time+"',";
                        update_str+="Image_Version='"+image_for_case+"',";
                        update_str+="LTK_Version='"+ltk_for_case+"' ";
                        update_str+="where ";
                        update_str+="Task='"+temp_task+"' and ";
                        update_str+="Board='"+temp_board+"' and ";
                        update_str+="Case_ID='"+temp_case+"';";

                        //insert into TestResult_Cloud values('task1','board1','TC_LCD_01','Pass','smb','2014','2014','beta1','20140520');

                        QString insert_str="insert into TestResult_Cloud values(";
                        insert_str+="'"+temp_task+"',";
                        insert_str+="'"+temp_board+"',";
                        insert_str+="'"+temp_case+"',";
                        insert_str+="'"+case_temp_status+"',";
                        insert_str+="'"+log_for_case+"',";
                        insert_str+="'"+start_time+"',";
                        insert_str+="'"+end_time+"',";
                        insert_str+="'"+image_for_case+"',";
                        insert_str+="'"+ltk_for_case+"',";
                        insert_str+="'"+job_name+"',";
                        insert_str+="'"+job_result+"',";
                        insert_str+="'"+execution_time+"');";
                        /*
                        QString staf_execution_time_str;
                        int staf_execution_time_int;
                        staf_execution_time_int=end_time.replace(" ","").toInt()-start_time.replace(" ","").toInt();
                        staf_execution_time_str=QString::number(staf_execution_time_int,10);
                        insert_str+="'"+staf_execution_time_str+"');";
                        */
                        /*
                        mysql> describe Cloud_Broad_Info
                        | Field               | Type         | Null | Key | Default | Extra |
                        +---------------------+--------------+------+-----+---------+-------+
                        | Job_Name            | varchar(256) | YES  |     | NULL    |       |
                        | board_id            | varchar(64)  | YES  |     | NULL    |       |
                        | board_type          | varchar(64)  | YES  |     | NULL    |       |
                        | chip_name           | varchar(64)  | YES  |     | NULL    |       |
                        | chip_stepping       | varchar(64)  | YES  |     | NULL    |       |
                        | board_register_date | varchar(64)  | YES  |     | NULL    |       |
                        | board_eco           | varchar(64)  | YES  |     | NULL    |       |
                        | lcd_resolution      | varchar(64)  | YES  |     | NULL    |       |
                        | lcd_screensize      | varchar(64)  | YES  |     | NULL    |       |
                        | ddr_type            | varchar(64)  | YES  |     | NULL    |       |
                        | ddr_size            | varchar(64)  | YES  |     | NULL    |       |
                        | emmc_type           | varchar(64)  | YES  |     | NULL    |       |
                        | emmc_size           | varchar(64)  | YES  |     | NULL    |       |
                        | rf_name             | varchar(64)  | YES  |     | NULL    |       |
                        | rf_type             | varchar(64)  | YES  |     | NULL    |       |
                        | machine             | varchar(64)  | YES  |     | NULL    |       |
                        +---------------------+--------------+------+-----+---------+-------+
                        16 rows in set (0.00 sec)
                        */


                        qDebug()<<"job_name"<<job_name;
                        QString insert_job_info_str="insert into Cloud_Broad_Info values(";
                        insert_job_info_str+="'"+job_name+"',";
                        QString board_job_id=job_name.replace(" ","");
                        qDebug()<<job_name<<board_job_id;
                        QStringList temp_board_job=board_job_id.split("-");
                        temp_board_job.removeLast();temp_board_job.removeLast();
                        if (temp_board_job.count()>1)
                        {board_job_id=temp_board_job.join("-");}
                        else
                        {board_job_id=temp_board_job.at(0); }
                        insert_job_info_str+="'"+staf_board_map[board_job_id].board_id+"',";
                        insert_job_info_str+="'"+staf_board_map[board_job_id].board_type+"',";
                        insert_job_info_str+="'"+staf_board_map[board_job_id].chip_name+"',";
                        insert_job_info_str+="'"+staf_board_map[board_job_id].chip_stepping +"',";
                        insert_job_info_str+="'"+staf_board_map[board_job_id].board_register_date+"',";
                        insert_job_info_str+="'"+staf_board_map[board_job_id].board_eco+"',";
                        insert_job_info_str+="'"+staf_board_map[board_job_id].lcd_resolution+"',";
                        insert_job_info_str+="'"+staf_board_map[board_job_id].lcd_screensize+"',";
                        insert_job_info_str+="'"+staf_board_map[board_job_id].ddr_type+"',";
                        insert_job_info_str+="'"+staf_board_map[board_job_id].ddr_size+"',";
                        insert_job_info_str+="'"+staf_board_map[board_job_id].emmc_type+"',";
                        insert_job_info_str+="'"+staf_board_map[board_job_id].emmc_size+"',";
                        insert_job_info_str+="'"+staf_board_map[board_job_id].rf_name+"',";
                        insert_job_info_str+="'"+staf_board_map[board_job_id].rf_type+"',";
                        insert_job_info_str+="'"+staf_board_map[board_job_id].dro+"',";
                        insert_job_info_str+="'"+staf_board_map[board_job_id].chip_type+"',";
                        insert_job_info_str+="'"+staf_board_map[board_job_id].machine+"');";

                        QString mysql_job_check="select * from Cloud_Broad_Info where Job_Name=\""+job_name+"\";";


                        QString latest_case_result=staf_task_case_map_tab[temp_task+"-B-"+temp_board+"-C-"+temp_case]->text();
                        qDebug()<<"check whether update sql"<<latest_case_result.indexOf(case_temp_status)<<use_mysql_database;

                        if ( task_list_tmp.at(i).indexOf("=Job Report=")>=0 )
                        {
                            QString job_key=temp_task+"-B-"+temp_board+"-C-"+temp_case;
                            qDebug()<<"connect status"<<dbMySQL.isOpen()<<"MY_SQL_JOB_Result_exit_check:"<<job_key<<"  in:  "<<finished_job_history;
                            if (finished_job_history.indexOf(job_key) < 0 )
                            {
                            qDebug()<<"MY_SQL_JOB_Result_ADD"<<insert_str;
                            //QSqlQuery query;
                            //query.exec(insert_str);
                            finished_job_history+=job_key+";";
                            }
                        }
                        if (latest_case_result.indexOf(case_temp_status) < 0 && use_mysql_database )
                        {
                            qDebug()<<"MY_SQL_JOB_EXIST_CHECK"<<mysql_job_check;
                            QSqlQuery query_check_job_updated;                           
                            query_check_job_updated.exec(mysql_job_check);
                            if ( query_check_job_updated.numRowsAffected() == 0 )
                            {
                            qDebug()<<"connect status"<<dbMySQL.isOpen()<<"MY_SQL_JOB_ADD"<<insert_job_info_str;
                            //QSqlQuery query_update_job_info;
                            //query_update_job_info.exec(insert_job_info_str);
                            }
                            qDebug()<<"SQL:"<<insert_job_info_str<<query_check_job_updated.numRowsAffected();
                        }


                        staf_task_case_map_tab[temp_task+"-B-"+temp_board+"-C-"+temp_case]->setText(case_temp_status);

                        if (case_temp_status.indexOf("Pass") >= 0 )
                        {
                            task_map[temp_task+"-B-"+temp_board+"-C-"+temp_case]->setForeground(1,QBrush(QColor(Qt::green)));
                            staf_task_case_map_tab[temp_task+"-B-"+temp_board+"-C-"+temp_case]->setTextColor("green");
                            //staf_task_log_map_tab[temp_task+"-B-"+temp_board+"-C-"+temp_case]->setTextColor("green");

                        }
                        if (case_temp_status.indexOf("Fail") >= 0 )
                        {
                            task_map[temp_task+"-B-"+temp_board+"-C-"+temp_case]->setText(1,case_temp_status);
                            staf_task_case_map_tab[temp_task+"-B-"+temp_board+"-C-"+temp_case]->setText(case_temp_status);
                            task_map[temp_task+"-B-"+temp_board+"-C-"+temp_case]->setForeground(1,QBrush(QColor(Qt::red)));
                            staf_task_case_map_tab[temp_task+"-B-"+temp_board+"-C-"+temp_case]->setTextColor("red");
                        }
                        //task_map.remove(temp_task+"-B-"+temp_board+"-C-"+temp_case);
                    }else
                    {qDebug()<<"not in task map";}
                }
            }

            qDebug()<<"update task status";
            if (task_map.find(temp_task+"-B-"+temp_board) != task_map.end())
            {
                temp_status=temp_status.replace(")","");
                temp_status=temp_status.replace("(",":");
                task_map[temp_task+"-B-"+temp_board]->setText(1,temp_status);

                //staf_task_case_map_tab[temp_task+"-B-"+temp_board+"-C-"+temp_case]->setText(case_temp_status);
                if (temp_status == "Pass" )
                { task_map[temp_task+"-B-"+temp_board]->setForeground(1,QBrush(QColor(Qt::green)));}
                if (temp_status.indexOf("Fail") >= 0 )
                {
                    task_map[temp_task+"-B-"+temp_board]->setText(1,temp_status+":"+job_result);
                    task_map[temp_task+"-B-"+temp_board]->setForeground(1,QBrush(QColor(Qt::red)));
                }
            }else
            {qDebug()<<"not in task map";}












            if (task_map.find(temp_task) != task_map.end())
            {
                staf_task_result[temp_task]=staf_task_result[temp_task]+temp_status;
                task_result=staf_task_result[temp_task];
                qDebug()<<task_result<<task_result.indexOf("Pass")<<task_result.indexOf("Fail");
                //if (task_result.indexOf("Pass") >= 0 &&  task_result.indexOf("Fail") >= 0)
                //{ task_map[temp_task]->setText(1,"Part Pass"); task_map[temp_task]->setForeground(1,QBrush(QColor(Qt::blue)));}
                if (task_result.indexOf("Pass") >= 0 &&  task_result.indexOf("Fail") < 0)
                {
                    //"Pass         : Fail" 0 15

                  task_map[temp_task]->setText(1,"Finished");
                  task_map[temp_task]->setForeground(1,QBrush(QColor(Qt::green)));
                }
                if ( task_result.indexOf("Fail") >= 0)
                {
                    task_map[temp_task]->setText(1,"Finished");
                    task_map[temp_task]->setForeground(1,QBrush(QColor(Qt::red)));
                }
                if (task_list_tmp.at(i).indexOf("=Job Report=")<0 || staf_task_result[temp_task].indexOf("not_finished") >= 0 )
                { task_map[temp_task]->setText(1,"Running");task_map[temp_task]->setForeground(1,QBrush(QColor(Qt::black)));
                    staf_task_result[temp_task]=staf_task_result[temp_task]+temp_status+"not_finished";}
            }else
            {qDebug()<<temp_task<<"not in task map";}

        }
        ui->pushButton_update->setDisabled(false);
    }
}


void MainWindow::start_eable(bool eable)
{
    if (eable)
    {
        ui->pushButton_confirm->setDisabled(true);
        ui->pushButton_start->setDisabled(false);
    }else
    {
        ui->pushButton_confirm->setDisabled(false);
        ui->pushButton_start->setDisabled(false);
    }
}

void MainWindow::reset_config()
{
    for (int i=1;i<=toolBox->count();i++)
    {
        toolBox->removeTab(1);
        //qDebug()<<i<<" "<<toolBox->count();
    }
    start_config->clear();
    start_eable(false);

}

void MainWindow::show_tree()
{
    ui->treeWidget->clear();
    QStringList tree_title;
    tree_title<<"Case ID"<<"Time Out"<<"Description";
    ui->treeWidget->setColumnCount(3);
    ui->treeWidget->setHeaderLabels(tree_title);

    QMap<QString,QStringList>::Iterator it;

    QString mod_name="NULL";

    //QTreeWidgetItem* root;
    //QTreeWidgetItem* root= new QTreeWidgetItem(ui->treeWidget);
    QTreeWidgetItem* top= new QTreeWidgetItem(ui->treeWidget);
    top->setText(0,platform.replace(":",""));

    QTreeWidgetItem* root;

    for (it=case_map.begin();it !=case_map.end();++it)
    {
        if (it.value().at(2).indexOf(platform)>=0)
        {
            //qDebug()<<it.key();
            QStringList mod_id=it.key().split(":");
            //qDebug()<<mod_id.at(0);
            if (mod_name.indexOf(mod_id.at(0)) < 0 )
            {
                //root= new QTreeWidgetItem(ui->treeWidget);
                root= new QTreeWidgetItem;
                root->setText(0,mod_id.at(0));
                mod_name=mod_id.at(0);
                root->setCheckState(0,Qt::Unchecked);
                top->addChild(root);
                mod_map[mod_id.at(0)]=root;
                root->setFlags(Qt::ItemIsSelectable|Qt::ItemIsDragEnabled|Qt::ItemIsUserCheckable|Qt::ItemIsEnabled|Qt::ItemIsTristate);
            }
            QTreeWidgetItem* leaf= new QTreeWidgetItem;
            leaf->setText(0,mod_id.at(1));
            leaf->setText(1,it.value().at(0));
            leaf->setText(2,it.value().at(1));
            leaf->setFlags(Qt::ItemIsEditable|Qt::ItemIsUserCheckable|Qt::ItemIsEnabled|Qt::ItemIsTristate);
            QString auto_flag=it.value().at(3);
            if ( auto_flag.indexOf("Y") < 0 )
            {
                leaf->setForeground(0,QBrush(QColor(Qt::gray)));
            }
            //leaf->setText(2,it.value().at(2));
            leaf->setCheckState(0,Qt::Unchecked);
            //leaf->isSelected();
            root->addChild(leaf);
            //ui->treeWidget->addTopLevelItem(root);
            //root->child(1)->setCheckState(0,Qt::Checked);
            //root->child(1)->setText(0,"dd");
        }
    }
    ui->treeWidget->addTopLevelItem(top);
    top->setExpanded(true);


}




void MainWindow::show_board_detail(int row, int col)
{
    qDebug()<<"show board detal";
    if (row != 0)
    {
    board_detail_id=ui->tableWidget_board->item(row,0)->text();
    qDebug()<<board_detail_id;
    //toolBox->setTabText(0,"Board Detail: "+board_detail_id);
    start_config->setText(staf_board_map[board_detail_id].allinfo);
    if (!toolBox->currentIndex()==0)
    {toolBox->setCurrentIndex(0);}
    }

}


void MainWindow::check_board_list()
{
    qDebug()<<"check_board"<<ui->tableWidget_board->rowCount();
    board_list.clear();
    for (int i=0+filter_row_num;i< (ui->tableWidget_board->rowCount());i++)
    {
        //qDebug()<<"board"<<ui->tableWidget_board->item(i,0)->text();

        if (ui->tableWidget_board->item(i,0)->checkState() == Qt::Checked)
        {
            qDebug()<<ui->tableWidget_board->item(i,0)->text();
            board_list<<ui->tableWidget_board->item(i,0)->text();
        }

    }
}



void MainWindow::check_tree()
{
    case_list.clear();
    case_list_loop.clear();
    qDebug()<<"check_tree";
    QTreeWidgetItemIterator check_tree(ui->treeWidget);
    while (*check_tree)
    {
        //qDebug()<<""<<check_tree.value();
        //qDebug()<<(*check_tree)->text(0);
        if ( (*check_tree)->checkState(0) && (!(*check_tree)->text(1).isEmpty()) )
        {
            qDebug()<<"SEL:"<<(*check_tree)->text(0);
            QString case_id_for_running=(*check_tree)->text(0);
            case_list<<case_id_for_running;
            if ((*check_tree)->text(1).indexOf("*") >=0 )
            {
                case_id_for_running+="-loop-"+(*check_tree)->text(1).split("*").at(1);
            }
            case_list_loop<<case_id_for_running;
        }
        ++check_tree;
    }
    qDebug()<<"end"<<check_tree.Checked;

}


void MainWindow::show_board()
{
    if (  progress_lock )
    { qDebug()<<"under showing board, skip";}
    else{
        qDebug()<<"Begin to list board";
        ui->pushButton_confirm->setDisabled(true);
        filter_board_id_list.clear();filter_platform_list.clear();filter_stepping_list.clear();
        filter_task_list.clear();filter_online_list.clear();filter_board_list.clear();
        filter_mcu_list.clear();filter_serial_list.clear();filter_usb_list.clear();
        filter_user_list.clear();filter_ip_list.clear();
        filter_ddr_size_list.clear();filter_dro_list.clear();
        filter_ddr_type_list.clear();filter_chip_type_list.clear();filter_owner_list.clear();
        platform=ui->comboBox_platform->currentText();



        check_board_list();
        ui->tableWidget_board->setDisabled(true);
        ui->tableWidget_board->clear();
        //ui->tableWidget_board->setShowGrid(false);
        pxa1908_sum=pxa1928_sum=pxa1936_sum=0;


        staf_board_map.clear();



        Gcmd = new QProcess();
        qDebug()<<__LINE__<<__FILE__<<Gcmd->isSequential();
        qDebug()<<"list pending board";
        waitProcess(work_dir+"/STAFEnv.sh staf "+staf_ip+" LTKTEST QUERY",100);
        process_retry=0;
        while(process_retry<10 && !progress_result)
        {
            qDebug()<<"process exit fail :query pending ,retry"<<process_retry;
            process_retry++;
            waitProcess(work_dir+"/STAFEnv.sh staf "+staf_ip+" LTKTEST LIST ",100);
        }
        qDebug()<<"workdir"<<work_dir;
        //waitProcess("cat pend.log",100);
        QString read_qurey=Gcmd->readAll();
        bool check_pending_string_format=false;
        if (progress_result)
        {
            QStringList pend_string_list=read_qurey.split("[");
            if (pend_string_list.count()>1)
            {
                check_pending_string_format=true;
            }
        }
        if (check_pending_string_format)
        {
            QString pend_string=read_qurey.split("[").at(1).split("]").at(0);
            qDebug()<<"check_pending_board";
            QStringList pend_string_item=pend_string.split("{");
            pend_string_item.removeFirst();
            qDebug()<<pend_string_item.count();
            qDebug()<<pend_string;
            for (int i=0;i<pend_string_item.count();i++)
            {
                QStringList pend_string_line=pend_string_item.at(i).split("\n");
                pend_string_line.removeLast();
                pend_string_line.removeLast();
                pend_string_line.removeFirst();
                for (int a=0;a<pend_string_line.count();a++)
                {
                    if ( pend_string_line.at(a).indexOf("Requested Entry") >= 0 && pend_string_line.at(a).indexOf("board_id") >= 0)
                    {
                        //QString pend_board=pend_string_line.at(a).split(":").at(1);
                        //pend_board=pend_board.replace(" ","");
                        QString pend_board=pend_string_line.at(a).split("board_id=").at(1);
                        pend_board=pend_board.split("&&").at(0);
                        qDebug()<<"pending board"<<pend_board<<staf_board_map.count(pend_board);

                        if ( staf_board_map.find(pend_board) == staf_board_map.end() )
                        {
                            staf_board_map[pend_board].task_num=0;
                        }
                        staf_board_map[pend_board].task_num++;
                        qDebug()<<"pend num:"<<staf_board_map[pend_board].task_num;
                    }
                }
            }

            //QStringList resource_list=read_qurey.split("[").at(2).split("]").at(0).split("Entry");
            QStringList resource_list=read_qurey.split("Ready Requests").last().split("Resources").at(1).split("Entry");

            resource_list.removeFirst();
            for (int a=0;a<resource_list.count();a++)
            {
                QString board_id_ori=resource_list.at(a).split("Owner").at(0);
                QString board_id_source=board_id_ori.replace(" ","").replace("\n","").replace(":","");
                QString board_task_source=resource_list.at(a).split("Owner").at(1).split("PreTasks").at(0);
                qDebug()<<"under_task_board :"<<board_id_ori<<board_id_source<<board_task_source;
                if (board_task_source.indexOf("<None>") < 0 )
                {
                    if ( staf_board_map.find(board_id_source) == staf_board_map.end() )
                    {
                        staf_board_map[board_id_source].task_num=0;
                    }
                    staf_board_map[board_id_source].task_num++;
                }

            }

        }else {qDebug()<<"pending string abormal , skip check_pending_board ";}
        delete Gcmd;Gcmd=NULL;

        Gcmd = new QProcess();
        qDebug()<<__LINE__<<__FILE__<<Gcmd->isSequential();
        qDebug()<<"list board";
        waitProcess(work_dir+"/STAFEnv.sh staf "+staf_ip+" LTKTEST LIST ",100);
        process_retry=0;
        while(process_retry<10 && !progress_result)
        {
            qDebug()<<"process exit fail :list ,retry"<<process_retry;
            process_retry++;
            waitProcess(work_dir+"/STAFEnv.sh staf "+staf_ip+" LTKTEST LIST ",100);
        }
        tempString=Gcmd->readAll();
        tempString=tempString.remove("]\n");
        QStringList staf_board_list=tempString.split("{");

        staf_board_list.removeFirst();
        qDebug()<<"genetate detail";
        int board_by_platform=1;
        ui->tableWidget_board->setRowCount(staf_board_list.count()+board_by_platform);


        for (int i=0;i<staf_board_list.count();i++)
        {

            QStringList staf_board_detail=staf_board_list.at(i).split("\n");
            staf_board_detail.removeFirst();
            staf_board_detail.removeLast();
            staf_board_detail.removeLast();

            QString board_id=staf_board_detail.at(0).split(": ").at(1);


            //staf_board_map[board_id].board_status="free";

            //QListWidgetItem* board= new QListWidgetItem(board_id,ui->listWidget);
            QTableWidgetItem* board= new QTableWidgetItem(board_id);
            ui->tableWidget_board->setItem(board_by_platform,0,board);
            if (filter_board_id_list.indexOf(board_id,0) < 0 )
            {filter_board_id_list<<board_id;}

            //board->setFlags(Qt::NoItemFlags);
            board->setCheckState(Qt::Unchecked);
            if (board_list.indexOf(board_id,0) >=0 )
            {
                board->setCheckState(Qt::Checked);
            }
            //ui->listWidget->setc
            qDebug()<<"i="<<i<<"for"<<staf_board_list.count()<<board_id<<staf_board_detail.count();

            QString item_name_style="<tr><td style=\"font-size:9px;font-weight:bold;\" width=\"130\">";
            QString item_value_style="<td style=\"font-size:9px;\">";
            QString item_group_style="<tr><td style=\"font-size:10px;font-weight:bold;\" colspan=\"2\" bgcolor=\"#99CCFF\">";
            QString table_string="<table border=\"1\" width=\"270\" >";

            table_string+=item_group_style;
            table_string+="BOARD INFO";
            table_string+="</td>";
            table_string+="</tr>";

            while (staf_board_detail.count()>32)
            {
                staf_board_detail.removeLast();
            }

            while (staf_board_detail.count()<32)
            {
                staf_board_detail<<"(AUTO_DETECT)task_lock: 1";
            }

            QString board_lock="Un-know_";


            //work around for v1.5 , it will removed after v1.6 released

            //ui->tableWidget_board->setItem(board_by_platform,14,new QTableWidgetItem("Un-know"));
            //ui->tableWidget_board->setItem(board_by_platform,15,new QTableWidgetItem("Un-know"));
            staf_board_map[board_id].dro="un-know";
            staf_board_map[board_id].chip_type="un-know";
            QTableWidgetItem* dro_item=new QTableWidgetItem("unknow");
            QTableWidgetItem* chip_type_item=new QTableWidgetItem("unknow");
            ui->tableWidget_board->setItem(board_by_platform,14,dro_item);
            ui->tableWidget_board->setItem(board_by_platform,15,chip_type_item);
            //work around for v1.5 , it will removed after v1.6 released

            bool platform_filter=false;
            bool user_name_filter=true;
            for (int a=0;a<staf_board_detail.count();a++)
            {

                qDebug()<<"check lock task"<<staf_board_detail.at(a);

                QString temp_item,temp_value,temp_str;
                temp_str=staf_board_detail.at(a);
                QStringList temp_list;

                //qDebug()<<"a="<<a<<"for"<<staf_board_detail.count()<<board_id<<temp_str.replace(" ","");
                temp_list=temp_str.replace(" ","").split(")").at(1).split(":");
                temp_item=temp_list.at(0);
                if ( temp_list.count()<2)
                {temp_value="Unknow";}else
                {
                    temp_value=temp_list.at(1);
                }
                if (  temp_item=="ddr_type" )
                {
                    temp_value=ddr_type_lp_transform(temp_value);
                }
                table_string+=item_name_style;
                table_string+=temp_item;
                table_string+="</td>";
                table_string+=item_value_style;
                table_string+=temp_value;
                table_string+="</td>";
                table_string+="</tr>";

                if (login_name!="global_user")
                {
                    if (temp_item.indexOf("username")>=0 && temp_value == login_name )
                    {
                        qDebug()<<"username_filter"<<login_name<<temp_value;
                        //board->setCheckState(Qt::Unchecked);
                        user_name_filter=false;
                        //board->setFlags(Qt::NoItemFlags);
                    }


                    if (temp_item.indexOf("userteam")>=0 && !platform_filter )
                    {
                        if (temp_value != login_name)
                        {
                            if (user_name_filter)
                            {
                            board->setCheckState(Qt::Unchecked);
                            board->setFlags(Qt::NoItemFlags);
                            }
                        }else
                        {qDebug()<<"userteam_matched"<<login_name<<temp_value;}
                    }
                }

                if (temp_item.indexOf("chip_name")>=0 && temp_value.toUpper() != platform_map[platform].toUpper()  )
                {
                    if ( (temp_value.indexOf("HELAN2") >=0 && platform== "HELAN2") || (temp_value.toLower().indexOf("pxa1926") >=0 && platform== "EDEN") )
                    {qDebug()<<"helan2";}else
                    {
                        qDebug()<<temp_value.toUpper()<<platform_map[platform].toUpper()<<"HHHHHHHH";
                        board->setCheckState(Qt::Unchecked);
                        board->setFlags(Qt::NoItemFlags);
                        platform_filter=true;
                    }
                }








                // ui->tableWidget_board->setItem(board_by_platform,0,board);

                if (temp_item.indexOf("task_lock")>=0)
                {
                    if ( temp_value.toInt() == 0 )
                    {
                        board_lock="Un-lock_";
                    }else
                    {
                        board_lock="Lock_";
                    }

                }


                QString task_value;
                if (staf_board_map.find(board_id) == staf_board_map.end())
                {
                    task_value=board_lock+"0";
                    ui->tableWidget_board->setItem(board_by_platform,4,new QTableWidgetItem(task_value));
                    if (filter_task_list.indexOf(task_value,0) < 0 )
                    {filter_task_list<<task_value;}
                }
                else if ( temp_item.indexOf("task_lock")>=0 )
                {
                    task_value=board_lock+QString::number(staf_board_map[board_id].task_num,10);
                    ui->tableWidget_board->setItem(board_by_platform,4,new QTableWidgetItem(task_value));

                    if (filter_task_list.indexOf(task_value,0) < 0 )
                    {filter_task_list<<task_value;}
                }
                qDebug()<<"Board fff"<<a<<staf_board_detail.count()<<board_id<<temp_item<<temp_value;
                staf_board_map[board_id].board_id=board_id;

                if (temp_value.isEmpty())
                {temp_value="Unknow";}
                qDebug()<<"add_board_table";
                if (temp_item.indexOf("chip_stepping")>=0)
                {
                    staf_board_map[board_id].chip_stepping=temp_value;ui->tableWidget_board->setItem(board_by_platform,2,new QTableWidgetItem(temp_value));
                    if (filter_stepping_list.indexOf(temp_value,0) < 0 )
                    {filter_stepping_list<<temp_value;}
                }
                if (temp_item.indexOf("ddr_type")>=0)
                {
                    //staf_board_map[board_id].chip_stepping=temp_value;
                    ui->tableWidget_board->setItem(board_by_platform,3,new QTableWidgetItem(temp_value));
                    if (filter_ddr_type_list.indexOf(temp_value,0) < 0 )
                    {filter_ddr_type_list<<temp_value;}
                }
                if (temp_item.indexOf("username")>=0)
                {
                    /*
                    QRegExp rx("[a-zA-Z]*");
                    if (!rx.exactMatch(temp_value))
                    {temp_value="Unknow";}else
                    {temp_value="Unknow1";}
                    */
                    ui->tableWidget_board->setItem(board_by_platform,10,new QTableWidgetItem(temp_value));
                    if (filter_owner_list.indexOf(temp_value,0) < 0 )
                    {filter_owner_list<<temp_value;}
                }

                if (temp_item.indexOf("serial")>=0)
                {
                    staf_board_map[board_id].connect_serial=temp_value;ui->tableWidget_board->setItem(board_by_platform,8,new QTableWidgetItem(temp_value));
                    if (filter_serial_list.indexOf(temp_value,0) < 0 )
                    {filter_serial_list<<temp_value;}
                }
                if (temp_item.indexOf("mcu")>=0)
                {
                    staf_board_map[board_id].connect_serial=temp_value;ui->tableWidget_board->setItem(board_by_platform,7,new QTableWidgetItem(temp_value));
                    if (filter_mcu_list.indexOf(temp_value,0) < 0 )
                    {filter_mcu_list<<temp_value;}
                }
                if (temp_item.indexOf("usb")>=0)
                {
                    staf_board_map[board_id].connect_usb=temp_value;ui->tableWidget_board->setItem(board_by_platform,9,new QTableWidgetItem(temp_value));
                    if (filter_usb_list.indexOf(temp_value,0) < 0 )
                    {filter_usb_list<<temp_value;}
                }
                if (temp_item.indexOf("StafConnection")>=0)
                {
                    staf_board_map[board_id].connect_staf=temp_value;ui->tableWidget_board->setItem(board_by_platform,5,new QTableWidgetItem(temp_value));
                    if (filter_online_list.indexOf(temp_value,0) < 0 )
                    {filter_online_list<<temp_value;}
                }
                if (temp_item.indexOf("BoardConnection")>=0)
                {
                    staf_board_map[board_id].connect_board=temp_value;ui->tableWidget_board->setItem(board_by_platform,6,new QTableWidgetItem(temp_value));
                    if (filter_board_list.indexOf(temp_value,0) < 0 )
                    {filter_board_list<<temp_value;}
                }

                if (temp_item.indexOf("current_user")>=0)
                {
                    staf_board_map[board_id].board=temp_value;ui->tableWidget_board->setItem(board_by_platform,11,new QTableWidgetItem(temp_value));
                    if (filter_user_list.indexOf(temp_value,0) < 0 )
                    {filter_user_list<<temp_value;}
                }

                if (temp_item.indexOf("userteam")>=0)
                {
                    staf_board_map[board_id].board=temp_value;
                    //ui->tableWidget_board->setItem(board_by_platform,11,new QTableWidgetItem(temp_value));
                    ui->tableWidget_board->item(board_by_platform,11)->setText(temp_value);
                    if (filter_user_list.indexOf(temp_value,0) < 0 )
                    {filter_user_list<<temp_value;}
                }
                if (temp_item.indexOf("physicalInterfaceID")>=0)
                {
                    staf_board_map[board_id].board=temp_value;ui->tableWidget_board->setItem(board_by_platform,12,new QTableWidgetItem(temp_value));
                    if (filter_ip_list.indexOf(temp_value,0) < 0 )
                    {filter_ip_list<<temp_value;}
                }
                if (temp_item.indexOf("ddr_size")>=0)
                {
                    staf_board_map[board_id].ddr_size=temp_value;ui->tableWidget_board->setItem(board_by_platform,13,new QTableWidgetItem(temp_value));
                    if (filter_ddr_size_list.indexOf(temp_value,0) < 0 )
                    {filter_ddr_size_list<<temp_value;}
                }
                if (temp_item.indexOf("emmc_size")>=0)
                {
                    staf_board_map[board_id].emmc_size=temp_value;
                }

                if (temp_item.indexOf("DRO")>=0)
                {
                    staf_board_map[board_id].dro=temp_value;
                    //ui->tableWidget_board->setItem(board_by_platform,14,new QTableWidgetItem(temp_value));
                    dro_item->setText(temp_value);
                    if (filter_dro_list.indexOf(temp_value,0) < 0 )
                    {filter_dro_list<<temp_value;}
                }
                if (temp_item.indexOf("CHIP_TYPE")>=0)
                {
                    staf_board_map[board_id].chip_type=temp_value;
                    //ui->tableWidget_board->setItem(board_by_platform,15,new QTableWidgetItem(temp_value));
                    chip_type_item->setText(temp_value);
                    if (filter_chip_type_list.indexOf(temp_value,0) < 0 )
                    {filter_chip_type_list<<temp_value;}
                }
                if (temp_item.indexOf("EXTRA_INFO")>=0)
                {
                    staf_board_map[board_id].chip_type=temp_value;
                    //ui->tableWidget_board->setItem(board_by_platform,15,new QTableWidgetItem(temp_value));
                    chip_type_item->setText(temp_value);
                    if (filter_chip_type_list.indexOf(temp_value,0) < 0 )
                    {filter_chip_type_list<<temp_value;}
                }
                qDebug()<<"fill_the_staf_board_map";
                if (temp_item.indexOf("board_id")>=0)
                {staf_board_map[board_id].board_id=temp_value;}
                if (temp_item.indexOf("board_type")>=0)
                {staf_board_map[board_id].board_type=temp_value;}
                if (temp_item.indexOf("board_register_date")>=0)
                {staf_board_map[board_id].board_register_date=temp_value;}
                if (temp_item.indexOf("board_eco")>=0)
                {staf_board_map[board_id].board_eco=temp_value;}
                if (temp_item.indexOf("lcd_resolution")>=0)
                {staf_board_map[board_id].lcd_resolution=temp_value;}
                if (temp_item.indexOf("lcd_screensize")>=0)
                {staf_board_map[board_id].lcd_screensize=temp_value;}
                if (temp_item.indexOf("ddr_type")>=0)
                {staf_board_map[board_id].ddr_type=temp_value;}
                if (temp_item.indexOf("emmc_type")>=0)
                {staf_board_map[board_id].emmc_type=temp_value;}
                if (temp_item.indexOf("rf_name")>=0)
                {staf_board_map[board_id].rf_name=temp_value;}
                if (temp_item.indexOf("rf_type")>=0)
                {staf_board_map[board_id].rf_type=temp_value;}
                if (temp_item.indexOf("machine")>=0)
                {staf_board_map[board_id].machine=temp_value;}

                qDebug()<<"sum the board";
                if (temp_item.indexOf("chip_name")>=0)
                {
                    staf_board_map[board_id].chip_name=temp_value; ui->tableWidget_board->setItem(board_by_platform,1,new QTableWidgetItem(temp_value));
                    if (filter_platform_list.indexOf(temp_value,0) < 0 )
                    {filter_platform_list<<temp_value;}
                    if (temp_value.indexOf("1908")>=0)
                    {pxa1908_sum++;}
                    if (temp_value.indexOf("1936")>=0)
                    {pxa1936_sum++;}
                    if (temp_value.indexOf("1928")>=0)
                    {pxa1928_sum++;}
                    platform=ui->comboBox_platform->currentText();
                    qDebug()<<"comparee"<<temp_value.toLower()<<platform_map[platform].toLower();
                }
                qDebug()<<board_id<<"check findished";
            }

            //ui->label_board_list->setText("Board List: HELAN-"+QString::number(pxa1088_sum,10)+";HELAN_LTE-"+QString::number(pxa1L88_sum,10)+" ;EDEN-"+QString::number(pxa1928_sum,10)+" ;HELAN3-"+QString::number(pxa1U88_sum,10)+";Unknow-"+QString::number(staf_board_list.count()-pxa988_sum-pxa1088_sum-pxa1L88_sum-pxa1928_sum-pxa1U88_sum,10)+";SUM-"+QString::number(staf_board_list.count(),10));
            ui->statusBar->showMessage("Board List: EDEN-"+QString::number(pxa1928_sum,10)+" ;HELAN3-"+QString::number(pxa1936_sum,10)+" ;ULC1-"+QString::number(pxa1908_sum,10)+";Unknow-"+QString::number(staf_board_list.count()-pxa1908_sum-pxa1928_sum-pxa1936_sum,10)+";SUM-"+QString::number(staf_board_list.count(),10));
            qDebug()<<"add_color";
            table_string+="</table>";
            staf_board_map[board_id].allinfo=table_string;

            if ( board_lock.indexOf("Un-lock") < 0 )
            {
                board->setIcon(lock_icon);
            }


            if (staf_board_map[board_id].task_num < 1 && board_lock.indexOf("Un-lock") >= 0 )
            {
                board->setTextColor(QColor("green"));
            }else
            {
                board->setTextColor(QColor("blue"));
            }
            if ((staf_board_map[board_id].connect_board.indexOf("Online") < 0 || staf_board_map[board_id].connect_staf.indexOf("online") < 0 ) && staf_board_map[board_id].task_num < 1 )
            {
                board->setTextColor(QColor("gray"));
            }
            board_by_platform++;
            qDebug()<<board_by_platform<<"board_by_platform";


        }
        /*
    QStringList board_list=tempString.split("\n");
    for (int i=0;(i<board_list.count()-1);i++)
    {
        //ui->listWidget->addItem(board_list.at(i));
        QListWidgetItem* board= new QListWidgetItem(board_list.at(i),ui->listWidget);
        board->setFlags(Qt::ItemIsEnabled|Qt::ItemIsUserCheckable);
        board->setCheckState(Qt::Unchecked);
    }
    //ui->listWidget->addItems(board_list);
    */
        qDebug()<<"resize table";
        ui->tableWidget_board->setSelectionBehavior(QAbstractItemView::SelectRows);

        ui->tableWidget_board->setColumnWidth(0,210);

    ui->tableWidget_board->setColumnWidth(1,70);
    ui->tableWidget_board->setColumnWidth(2,70);
    ui->tableWidget_board->setColumnWidth(3,120);
    ui->tableWidget_board->setColumnWidth(4,70);
    ui->tableWidget_board->setColumnWidth(5,90);
    ui->tableWidget_board->setColumnWidth(6,100);
    ui->tableWidget_board->setColumnWidth(7,55);
    ui->tableWidget_board->setColumnWidth(8,60);
    ui->tableWidget_board->setColumnWidth(9,70);

        ui->tableWidget_board->verticalHeader()->setHidden(true);
        QStringList table_head_name;
        table_head_name<<"Board ID        "<<"Name"<<"Stepping"<<"DDR_Type"<<"Task"<<"Staf_online"<<"Board_online"<<"Mcu"<<"Serial"<<"USB"<<"User"<<"UserTeam"<<"IP"<<"DDR Size"<<"DRO"<<"Chip Type";
        ui->tableWidget_board->setHorizontalHeaderLabels(table_head_name);



        for (int e=0;e<ui->tableWidget_board->rowCount();e++)
        {
            ui->tableWidget_board->setRowHeight(e,20);
        }

        //ui->tableWidget_board->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
        qDebug()<<"add filter menu";

show_hidden_table();

create_filter_menu();

board_list_reflesh_count++;

if ( filter_table_enable )
{
    filter_table_enable=false;
     qDebug()<<"add filter menu2";
    show_filter_menu();
}else
{
filter_table_enable=true;
qDebug()<<"add filter menu3";
show_filter_menu();
}
qDebug()<<"add filter menu4";
if (hide_unavailable_board)
{
    combobox_filter_online->setCurrentIndex(combobox_filter_online->findText("online"));
    combobox_filter_board->setCurrentIndex(combobox_filter_board->findText("Online"));
}
qDebug()<<"add filter menu5";
ui->tableWidget_board->sortItems(0,Qt::DescendingOrder);
ui->tableWidget_board->setDisabled(false);
delete Gcmd;Gcmd=NULL;
ui->pushButton_confirm->setDisabled(false);
qDebug()<<"show board end";
    }

}


void MainWindow::show_hide_unavailable_board()
{
    if (hide_unavailable_board)
    {
        hide_unavailable_board=false;
        show_hide_board->setIcon(QIcon("./image/un_filter_big.png"));
    }else
    {
        hide_unavailable_board=true;
        show_hide_board->setIcon(QIcon("./image/filter_big.png"));
    }
    show_board();
    //show_filter_menu();

}

void MainWindow::create_filter_menu()
{


    qDebug()<<"create filter menu";

    combobox_filter_board_id= new QComboBox;
    combobox_filter_platform= new QComboBox;
    combobox_filter_stepping= new QComboBox;
    combobox_filter_task= new QComboBox;
    combobox_filter_online= new QComboBox;
    combobox_filter_board= new QComboBox;
    combobox_filter_mcu= new QComboBox;
    combobox_filter_serial= new QComboBox;
    combobox_filter_usb= new QComboBox;
    combobox_filter_user= new QComboBox;
    combobox_filter_ip= new QComboBox;
    combobox_filter_ddr_size= new QComboBox;
    combobox_filter_ddr_type= new QComboBox;
    combobox_filter_dro= new QComboBox;
    combobox_filter_chip_type= new QComboBox;
    combobox_filter_owner= new QComboBox;

    combobox_filter_board_id->clear();
    combobox_filter_platform->clear();
    combobox_filter_stepping->clear();
    combobox_filter_task->clear();
    combobox_filter_online->clear();
    combobox_filter_board->clear();
    combobox_filter_mcu->clear();
    combobox_filter_serial->clear();
    combobox_filter_usb->clear();
    combobox_filter_user->clear();
    combobox_filter_ip->clear();
    combobox_filter_ddr_size->clear();
    combobox_filter_dro->clear();
    combobox_filter_ddr_type->clear();
    combobox_filter_chip_type->clear();
    combobox_filter_owner->clear();


    combobox_filter_board_id->addItem("Un-Filter");
    combobox_filter_platform->addItem("Un-Filter");
    combobox_filter_stepping->addItem("Un-Filter");
    combobox_filter_task->addItem("Un-Filter");
    combobox_filter_online->addItem("Un-Filter");
    combobox_filter_board->addItem("Un-Filter");
    combobox_filter_mcu->addItem("Un-Filter");
    combobox_filter_serial->addItem("Un-Filter");
    combobox_filter_usb->addItem("Un-Filter");
    combobox_filter_user->addItem("Un-Filter");
    combobox_filter_ip->addItem("Un-Filter");
    combobox_filter_ddr_size->addItem("Un-Filter");
    combobox_filter_dro->addItem("Un-Filter");
    combobox_filter_ddr_type->addItem("Un-Filter");
    combobox_filter_chip_type->addItem("Un-Filter");
    combobox_filter_owner->addItem("Un-Filter");

    connect(combobox_filter_board_id,SIGNAL(currentIndexChanged(QString)),this,SLOT(filter_item(QString)));
    connect(combobox_filter_platform,SIGNAL(currentIndexChanged(QString)),this,SLOT(filter_item(QString)));
    connect(combobox_filter_stepping,SIGNAL(currentIndexChanged(QString)),this,SLOT(filter_item(QString)));
    connect(combobox_filter_task,SIGNAL(currentIndexChanged(QString)),this,SLOT(filter_item(QString)));
    connect(combobox_filter_online,SIGNAL(currentIndexChanged(QString)),this,SLOT(filter_item(QString)));
    connect(combobox_filter_board,SIGNAL(currentIndexChanged(QString)),this,SLOT(filter_item(QString)));
    connect(combobox_filter_mcu,SIGNAL(currentIndexChanged(QString)),this,SLOT(filter_item(QString)));
    connect(combobox_filter_serial,SIGNAL(currentIndexChanged(QString)),this,SLOT(filter_item(QString)));
    connect(combobox_filter_usb,SIGNAL(currentIndexChanged(QString)),this,SLOT(filter_item(QString)));
    connect(combobox_filter_user,SIGNAL(currentIndexChanged(QString)),this,SLOT(filter_item(QString)));
    connect(combobox_filter_ip,SIGNAL(currentIndexChanged(QString)),this,SLOT(filter_item(QString)));
    connect(combobox_filter_ddr_size,SIGNAL(currentIndexChanged(QString)),this,SLOT(filter_item(QString)));
    connect(combobox_filter_dro,SIGNAL(currentIndexChanged(QString)),this,SLOT(filter_item(QString)));
    connect(combobox_filter_owner,SIGNAL(currentIndexChanged(QString)),this,SLOT(filter_item(QString)));
    connect(combobox_filter_ddr_type,SIGNAL(currentIndexChanged(QString)),this,SLOT(filter_item(QString)));
    connect(combobox_filter_chip_type,SIGNAL(currentIndexChanged(QString)),this,SLOT(filter_item(QString)));

    ui->tableWidget_board->setCellWidget(0,0,combobox_filter_board_id);
    combobox_filter_board_id->addItems(filter_board_id_list);
    ui->tableWidget_board->setItem(0,0,new QTableWidgetItem("zzz"));

    ui->tableWidget_board->setCellWidget(0,1,combobox_filter_platform);
    combobox_filter_platform->addItems(filter_platform_list);
    ui->tableWidget_board->setItem(0,1,new QTableWidgetItem("zzz"));

    ui->tableWidget_board->setCellWidget(0,2,combobox_filter_stepping);
    combobox_filter_stepping->addItems(filter_stepping_list);
    ui->tableWidget_board->setItem(0,2,new QTableWidgetItem("zzz"));

    ui->tableWidget_board->setCellWidget(0,4,combobox_filter_task);
    combobox_filter_task->addItems(filter_task_list);
    ui->tableWidget_board->setItem(0,4,new QTableWidgetItem("zzz"));

    ui->tableWidget_board->setCellWidget(0,5,combobox_filter_online);
    combobox_filter_online->addItems(filter_online_list);
    ui->tableWidget_board->setItem(0,5,new QTableWidgetItem("zzz"));

    ui->tableWidget_board->setCellWidget(0,6,combobox_filter_board);
    combobox_filter_board->addItems(filter_board_list);
    ui->tableWidget_board->setItem(0,6,new QTableWidgetItem("zzz"));

    ui->tableWidget_board->setCellWidget(0,7,combobox_filter_mcu);
    combobox_filter_mcu->addItems(filter_mcu_list);
    ui->tableWidget_board->setItem(0,7,new QTableWidgetItem("zzz"));

    ui->tableWidget_board->setCellWidget(0,8,combobox_filter_serial);
    combobox_filter_serial->addItems(filter_serial_list);
    ui->tableWidget_board->setItem(0,8,new QTableWidgetItem("zzz"));

    ui->tableWidget_board->setCellWidget(0,9,combobox_filter_usb);
    combobox_filter_usb->addItems(filter_usb_list);
    ui->tableWidget_board->setItem(0,9,new QTableWidgetItem("zzz"));

    ui->tableWidget_board->setCellWidget(0,11,combobox_filter_user);
    combobox_filter_user->addItems(filter_user_list);
    ui->tableWidget_board->setItem(0,11,new QTableWidgetItem("zzz"));

    ui->tableWidget_board->setCellWidget(0,12,combobox_filter_ip);
    combobox_filter_ip->addItems(filter_ip_list);
    ui->tableWidget_board->setItem(0,12,new QTableWidgetItem("zzz"));

    ui->tableWidget_board->setCellWidget(0,13,combobox_filter_ddr_size);
    combobox_filter_ddr_size->addItems(filter_ddr_size_list);
    ui->tableWidget_board->setItem(0,13,new QTableWidgetItem("zzz"));

    ui->tableWidget_board->setCellWidget(0,14,combobox_filter_dro);
    combobox_filter_dro->addItems(filter_dro_list);
    ui->tableWidget_board->setItem(0,14,new QTableWidgetItem("zzz"));

    ui->tableWidget_board->setCellWidget(0,3,combobox_filter_ddr_type);
    combobox_filter_ddr_type->addItems(filter_ddr_type_list);
    ui->tableWidget_board->setItem(0,3,new QTableWidgetItem("zzz"));

    ui->tableWidget_board->setCellWidget(0,10,combobox_filter_owner);
    combobox_filter_owner->addItems(filter_owner_list);
    ui->tableWidget_board->setItem(0,10,new QTableWidgetItem("zzz"));

    ui->tableWidget_board->setCellWidget(0,15,combobox_filter_chip_type);
    combobox_filter_chip_type->addItems(filter_chip_type_list);
    ui->tableWidget_board->setItem(0,15,new QTableWidgetItem("zzz"));

    /*
    combobox_filter_board_id->setItemIcon(0,unfilter_icon);
    for (int f=1;f<combobox_filter_board_id->count();f++)
    {
        //combobox_filter_board_id->setItemIcon(f,filter_icon);
        //QSize ss;ss.setWidth(11);ss.setHeight(11);
       //combobox_filter_board_id->setIconSize(ss);
       //combobox_filter_board_id->setStyleSheet("QComboBox::drop-down{border-style: none;}");
       combobox_filter_board_id->setStyleSheet("QComboBox::drop-down{image:url(./image/filter.png);}"
                                               "QComboBox::drop-down{border:0px;}");
    }
    */

    //combobox_filter_board_id->setStyleSheet("QComboBox::drop-down{image:url(reboot.png);}""QComboBox{background-color:white};}");
    //combobox_filter_board_id->setStyleSheet("QComboBox::drop-down{border:0px;}""QComboBox::drop-down{border-image:url(./image/filter.png) 4 4 4 4 stretch stretch;}");
    combobox_filter_board_id->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/un_filter.png)}");
    combobox_filter_platform->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/un_filter.png)}");
    combobox_filter_stepping->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/un_filter.png)}");
    combobox_filter_task->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/un_filter.png)}");
    combobox_filter_online->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/un_filter.png)}");
    combobox_filter_board->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/un_filter.png)}");
    combobox_filter_mcu->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/un_filter.png)}");
    combobox_filter_serial->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/un_filter.png)}");
    combobox_filter_usb->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/un_filter.png)}");
    combobox_filter_user->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/un_filter.png)}");
    combobox_filter_ip->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/un_filter.png)}");
    combobox_filter_ddr_size->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/un_filter.png)}");
    combobox_filter_dro->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/un_filter.png)}");
    combobox_filter_ddr_type->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/un_filter.png)}");
    combobox_filter_owner->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/un_filter.png)}");
    combobox_filter_chip_type->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/un_filter.png)}");
}

void MainWindow::filter_item(QString filter_item_name)
{

    qDebug()<<"filter item@col";
    qDebug()<<"filter item@col"<<ui->tableWidget_board->currentColumn();
    if (filter_item_name == "Un-Filter" )
    {
            show_hidden_table();
            combobox_filter_board_id->setCurrentIndex(0);
            combobox_filter_platform->setCurrentIndex(0);
            combobox_filter_stepping->setCurrentIndex(0);
            combobox_filter_task->setCurrentIndex(0);
            combobox_filter_online->setCurrentIndex(0);
            combobox_filter_board->setCurrentIndex(0);
            combobox_filter_mcu->setCurrentIndex(0);
            combobox_filter_serial->setCurrentIndex(0);
            combobox_filter_usb->setCurrentIndex(0);
            combobox_filter_user->setCurrentIndex(0);
            combobox_filter_ip->setCurrentIndex(0);
            combobox_filter_ddr_size->setCurrentIndex(0);
            combobox_filter_dro->setCurrentIndex(0);
            combobox_filter_owner->setCurrentIndex(0);
            combobox_filter_ddr_type->setCurrentIndex(0);
            combobox_filter_chip_type->setCurrentIndex(0);

            combobox_filter_board_id->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/un_filter.png)}");
            combobox_filter_platform->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/un_filter.png)}");
            combobox_filter_stepping->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/un_filter.png)}");
            combobox_filter_task->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/un_filter.png)}");
            combobox_filter_online->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/un_filter.png)}");
            combobox_filter_board->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/un_filter.png)}");
            combobox_filter_mcu->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/un_filter.png)}");
            combobox_filter_serial->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/un_filter.png)}");
            combobox_filter_usb->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/un_filter.png)}");
            combobox_filter_user->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/un_filter.png)}");
            combobox_filter_ip->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/un_filter.png)}");
            combobox_filter_ddr_size->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/un_filter.png)}");
            combobox_filter_dro->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/un_filter.png)}");
            combobox_filter_owner->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/un_filter.png)}");
            combobox_filter_ddr_type->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/un_filter.png)}");
            combobox_filter_chip_type->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/un_filter.png)}");
    }else
    {
        show_hidden_table();



        if ( combobox_filter_board_id->currentText() != "Un-Filter" )
            {combobox_filter_board_id->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/filter.png)}");}
        if ( combobox_filter_platform->currentText()  != "Un-Filter" )
            {combobox_filter_platform->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/filter.png)}");}
        if ( combobox_filter_stepping->currentText() != "Un-Filter" )
            {combobox_filter_stepping->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/filter.png)}");}

        if ( combobox_filter_task->currentText() != "Un-Filter" )
            {combobox_filter_task->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/filter.png)}");}

        if ( combobox_filter_online->currentText() != "Un-Filter" )
            {combobox_filter_online->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/filter.png)}");}

        if ( combobox_filter_board->currentText() != "Un-Filter" )
            {combobox_filter_board->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/filter.png)}");}

        if ( combobox_filter_mcu->currentText() != "Un-Filter" )
            {combobox_filter_mcu->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/filter.png)}");}
        if ( combobox_filter_serial->currentText() != "Un-Filter" )
            {combobox_filter_serial->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/filter.png)}");}
        if ( combobox_filter_usb->currentText() != "Un-Filter" )
            {combobox_filter_usb->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/filter.png)}");}

        if ( combobox_filter_user->currentText() != "Un-Filter" )
            {combobox_filter_user->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/filter.png)}");}
        if ( combobox_filter_ip->currentText() != "Un-Filter" )
            {combobox_filter_ip->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/filter.png)}");}
        if ( combobox_filter_ddr_size->currentText() != "Un-Filter" )
            {combobox_filter_ddr_size->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/filter.png)}");}
        if ( combobox_filter_dro->currentText() != "Un-Filter" )
            {combobox_filter_dro->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/filter.png)}");}

        if ( combobox_filter_owner->currentText() != "Un-Filter" )
            {combobox_filter_owner->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/filter.png)}");}
        if ( combobox_filter_ddr_type->currentText() != "Un-Filter" )
            {combobox_filter_ddr_type->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/filter.png)}");}
        if ( combobox_filter_chip_type->currentText() != "Un-Filter" )
            {combobox_filter_chip_type->setStyleSheet("QComboBox::drop-down{border:2px;}""QComboBox::drop-down{image:url(./image/filter.png)}");}

    for (int i=1;i< (ui->tableWidget_board->rowCount());i++)
    {

       if (ui->tableWidget_board->item(i,0)->text() != combobox_filter_board_id->currentText() && combobox_filter_board_id->currentText() != "Un-Filter" )
           {ui->tableWidget_board->hideRow(i);continue;}
       if (ui->tableWidget_board->item(i,1)->text() != combobox_filter_platform->currentText() && combobox_filter_platform->currentText()  != "Un-Filter" )
           {ui->tableWidget_board->hideRow(i);continue;}

       if (ui->tableWidget_board->item(i,2)->text() != combobox_filter_stepping->currentText() && combobox_filter_stepping->currentText() != "Un-Filter" )
           {ui->tableWidget_board->hideRow(i);continue;}

       if (ui->tableWidget_board->item(i,4)->text() != combobox_filter_task->currentText() && combobox_filter_task->currentText() != "Un-Filter" )
           {ui->tableWidget_board->hideRow(i);continue;}

       if (ui->tableWidget_board->item(i,5)->text() != combobox_filter_online->currentText() && combobox_filter_online->currentText() != "Un-Filter" )
           {ui->tableWidget_board->hideRow(i);continue;}

       if (ui->tableWidget_board->item(i,6)->text() != combobox_filter_board->currentText() && combobox_filter_board->currentText() != "Un-Filter" )
           {ui->tableWidget_board->hideRow(i);continue;}

       if (ui->tableWidget_board->item(i,7)->text() != combobox_filter_mcu->currentText() && combobox_filter_mcu->currentText() != "Un-Filter" )
           {ui->tableWidget_board->hideRow(i);continue;}
       if (ui->tableWidget_board->item(i,8)->text() != combobox_filter_serial->currentText() && combobox_filter_serial->currentText() != "Un-Filter" )
           {ui->tableWidget_board->hideRow(i);continue;}
       if (ui->tableWidget_board->item(i,9)->text() != combobox_filter_usb->currentText() && combobox_filter_usb->currentText() != "Un-Filter" )
           {ui->tableWidget_board->hideRow(i);continue;}

       if (ui->tableWidget_board->item(i,11)->text() != combobox_filter_user->currentText() && combobox_filter_user->currentText() != "Un-Filter" )
           {ui->tableWidget_board->hideRow(i);continue;}
       if (ui->tableWidget_board->item(i,12)->text() != combobox_filter_ip->currentText() && combobox_filter_ip->currentText() != "Un-Filter" )
           {ui->tableWidget_board->hideRow(i);continue;}
       if (ui->tableWidget_board->item(i,13)->text() != combobox_filter_ddr_size->currentText() && combobox_filter_ddr_size->currentText() != "Un-Filter" )
           {ui->tableWidget_board->hideRow(i);continue;}
       if (ui->tableWidget_board->item(i,14)->text() != combobox_filter_dro->currentText() && combobox_filter_dro->currentText() != "Un-Filter" )
           {ui->tableWidget_board->hideRow(i);continue;}



       if (ui->tableWidget_board->item(i,3)->text() != combobox_filter_ddr_type->currentText() && combobox_filter_ddr_type->currentText() != "Un-Filter" )
           {ui->tableWidget_board->hideRow(i);continue;}
       if (ui->tableWidget_board->item(i,10)->text() != combobox_filter_owner->currentText() && combobox_filter_owner->currentText() != "Un-Filter" )
           {ui->tableWidget_board->hideRow(i);continue;}
       if (ui->tableWidget_board->item(i,15)->text() != combobox_filter_chip_type->currentText() && combobox_filter_chip_type->currentText() != "Un-Filter" )
           {ui->tableWidget_board->hideRow(i);continue;}

    }







/*

    for (int i=1;i< (ui->tableWidget_board->rowCount());i++)
    {

       if (  ui->tableWidget_board->isRowHidden(i) )
       {
           qDebug()<<"remove"<<ui->tableWidget_board->item(i,1)->text()<<combobox_filter_platform->findText(ui->tableWidget_board->item(i,1)->text());
           combobox_filter_platform->removeItem(combobox_filter_platform->findText(ui->tableWidget_board->item(i,1)->text()));
       }

    }


    for (int i=1;i< (ui->tableWidget_board->rowCount());i++)
    {

       if (  ! ui->tableWidget_board->isRowHidden(i) )
       {
               if (combobox_filter_platform->findText(ui->tableWidget_board->item(i,1)->text()) < 0 )
           {
           combobox_filter_platform->addItem(ui->tableWidget_board->item(i,1)->text());
           }
       }

    }
qDebug()<<combobox_filter_platform->currentText();
combobox_filter_platform->addItem("---------");
//combobox_filter_platform->addItems(filter_platform_list);
*/

    }

}


void MainWindow::show_filter_menu()
{
    qDebug()<<"show filter menu";
    if ( filter_table_enable)
    {
    show_hidden_table();
    ui->tableWidget_board->hideRow(0);
    filter_table_enable=false;
    }else
    {
        show_hidden_table();
        //ui->tableWidget_board->sortByColumn(1);
        qDebug()<<"sortItems(1,Qt::DescendingOrder)";
        ui->tableWidget_board->sortItems(1,Qt::DescendingOrder);
        filter_table_enable=true;
    }
    qDebug()<<"show menu end";
}




void MainWindow::sort_col(int col)
{
    /*
    QMenu* item_menu= new QMenu("table_menu");
    QAction *order = new QAction(tr("Sort A -> Z"),this);
    QAction *desorder = new QAction(tr("Sort Z -> A"),this);

    item_menu->addAction(order);
    item_menu->addAction(desorder);

    QActionGroup* action_group = new QActionGroup(this);
    action_group->addAction(order);
    action_group->addAction(desorder);
    order->setChecked(true);
    //desorder->setChecked(true);
    action_group->setExclusive(false);

    //item_menu.addAction(action_group->addAction(order));
    //item_menu.addAction(action_group->addAction(desorder));
    QWidgetAction *action1 = new QWidgetAction(item_menu);
    action1->setDefaultWidget(new QLabel("test"));



    order_col=col;
    connect(order,SIGNAL(triggered()),this,SLOT(sort_table_order()));
    connect(desorder,SIGNAL(triggered()),this,SLOT(sort_table_desorder()));
    item_menu->setStyleSheet("font:9pt;text-align:center;");
    //item_menu.setFixedSize(100,20);
    item_menu->exec(QCursor::pos());

    */



    QMenu* item_menu= new QMenu("table_menu");

    QAction *order = new QAction(tr("Sort A -> Z"),this);
    QAction *desorder = new QAction(tr("Sort Z -> A"),this);


    item_menu->addAction(order);
    item_menu->addAction(desorder);


    QAction *filter1 = new QAction(tr("Filter"),this);
    item_menu->addAction(filter1);
    connect(filter1,SIGNAL(triggered()),this,SLOT(show_filter_menu()));

    if ( filter_table_enable )
    {
    order->setDisabled(true);
    desorder->setDisabled(true);
    }
    order_col=col;
    connect(order,SIGNAL(triggered()),this,SLOT(sort_table_order()));
    connect(desorder,SIGNAL(triggered()),this,SLOT(sort_table_desorder()));
    item_menu->setStyleSheet("font:9pt;text-align:center;");
    //item_menu.setFixedSize(100,20);
    item_menu->exec(QCursor::pos());


}




void MainWindow::show_hidden_table()
{
    qDebug()<<"show hidden table start";
    for (int i=0;i< (ui->tableWidget_board->rowCount());i++)
    {
            ui->tableWidget_board->showRow(i);
    }
    qDebug()<<"show hidden table end";
}


void MainWindow::sort_table_desorder()
{
    ui->tableWidget_board->sortItems(order_col,Qt::DescendingOrder);
    ui->tableWidget_board->hideRow(0);

}

void MainWindow::sort_table_order()
{
    ui->tableWidget_board->sortItems(order_col,Qt::AscendingOrder);
    ui->tableWidget_board->hideRow(ui->tableWidget_board->rowCount()-1);
}

void MainWindow::change_list()
{
    platform=":"+ui->comboBox_platform->currentText()+":";
    show_tree();
    show_board();
    show_package_tree();
    show_image_tree();

}

void MainWindow::clear_config()
{
    //ui->pushButton_confirm->setDisabled(true);
    platform=":"+ui->comboBox_platform->currentText()+":";
    //show_tree();
    show_board();
    //ui->pushButton_confirm->setDisabled(false);
    qDebug()<<"clear config end";
}


void MainWindow::xml_read()
{
    QFile file("Case_information.xml");
    QDomDocument doc;
    QString errStr;
    int errLine, errCol;
    if(!doc.setContent(&file, false, &errStr, &errLine, &errCol))
    {
        qDebug()<<"Error： "+errStr;
    }
    QDomElement root3=doc.documentElement();
    //qDebug()<<root3.nodeName();
    QDomNode root2 = root3.firstChild();


    QDomNode root1 = root2.firstChild();
    //qDebug()<<root1.toElement().nodeName()<<"\n"<<root1.toElement().text();
    //qDebug()<<root1.toElement().text();
    while (!root1.isNull())
    {

        QDomNode case_info = root1.firstChild();
        //qDebug()<<case_info.toElement().tagName();

        QDomNode case_info_detail = case_info.firstChild();
        QString map_id,map_mod,time_out,case_name,auto_flag;
        QStringList map_id_detail;
        while (!case_info_detail.isNull())
        {
            //qDebug()<<child.toElement().tagName()<<":"<<child.toElement().text()<<child.toElement().attribute("size");
            QString item_name;

            item_name=case_info_detail.toElement().tagName();
            //string_size=child.toElement().attribute("size");
            if (item_name.indexOf("casepath") >= 0 )
            {
                //qDebug()<<"moudel:"<<case_info_detail.toElement().text();
                map_mod=case_info_detail.toElement().text();
            }
            if (item_name.indexOf("case_id") >= 0 )
            {
                //qDebug()<<"ID:"<<case_info_detail.toElement().text();
                map_id=case_info_detail.toElement().text();
            }
            if (item_name.indexOf("timeout") >= 0 )
            {
               // qDebug()<<"timeout:"<<case_info_detail.toElement().text();
                time_out=case_info_detail.toElement().text();
            }
            if (item_name.indexOf("case_name") >= 0 )
            {
               // qDebug()<<"timeout:"<<case_info_detail.toElement().text();
                case_name=case_info_detail.toElement().text();
            }
            if (item_name.indexOf("autoflag") >= 0 )
            {
                //qDebug()<<"timeout:"<<case_info_detail.toElement().text();
                auto_flag=case_info_detail.toElement().text();
            }
            case_info_detail=case_info_detail.nextSibling();
        }

        QDomNode case_info_platform=case_info.nextSibling();
        QDomNode case_info_platform_detail=case_info_platform.firstChild();
        QString platform_name;
        while (!case_info_platform_detail.isNull())
        {
            platform_name=platform_name+":"+case_info_platform_detail.toElement().text()+":";
            case_info_platform_detail=case_info_platform_detail.nextSibling();
            //qDebug()<<platform<<"\n";
        }
        map_id=map_mod+":"+map_id;
        map_id_detail<<time_out<<case_name<<platform_name<<auto_flag;
        //if (!map_mod.isNull() && (auto_flag.indexOf("Y")>=0))
        if (!map_mod.isNull())
        {
            case_map[map_id]=map_id_detail;
        }
        root1=root1.nextSibling();
    }

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
        //progress_lock=false;
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
        }
        progress_lock=false;
        qDebug()<<__LINE__<<__FILE__<<"the result is"<<"exitStatus"<<Gcmd->exitStatus()<<"exitCode"<<Gcmd->exitCode()<<"state"<<Gcmd->state();
        progress_return=Gcmd->exitCode();
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
    qDebug()<<"progress_lock"<<progress_lock;
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
