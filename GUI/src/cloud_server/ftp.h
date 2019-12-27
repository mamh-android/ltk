#ifndef FTP_H
#define FTP_H
#include <QDialog>
#include <QHash>
#include <QFtp>

QT_BEGIN_NAMESPACE
class QDialogButtonBox;
class QFile;
class QLabel;
class QLineEdit;
class QTreeWidget;
class QTreeWidgetItem;
class QProgressDialog;
class QPushButton;
class QUrlInfo;
class QNetworkSession;
QT_END_NAMESPACE

class Ftp: QWidget
{
    Q_OBJECT

public:
    Ftp(QWidget *parent = 0);
    ~Ftp();

    static QString FtpServerIP;
    static void setFtpServerIp(const QString &temp) {FtpServerIP=temp;}
    void uploadFile();
    void setGfileName(const QString &temp) {GfileName=temp;}
    QString getGfileName(){return GfileName;}
    void setcdDirname(const QStringList &temp){cdDirname=temp;}
    void connectOrDisconnect();
    QFtp::State state();
    void closeFtp();
//![0]
private slots:
    void cancelDownload();
    void addToList(const QUrlInfo &urlInfo);

    void ftpCommandFinished(int commandId, bool error);
    void updateDataTransferProgress(qint64 readBytes,
                                    qint64 totalBytes);
//![0]

private:
//    void closeFtp();
    QProgressDialog *progressDialog;
    QString GfileName; //the whole file name locally including temp/Date/time/case.Goutput
    QStringList cdDirname;//the rest need cd dir name, value is upload/ProjectIndex/temp/Date/time
    QString mkdirFilename;//the new maked dir name
//![1]
//    QHash<QString, bool> isDirectory;
//    QString currentPath;
    QFtp *ftp;
    QFile *file;
    QFile *uploadfileName;

//    QNetworkSession *networkSession;
    QHash<QString, bool> isDirectory;
    int commandid;//0 means done.
////![1]
};
#endif // FTP_H

