#ifndef DIALOG_H
#define DIALOG_H
enum BlfType{EmuBlfFileName,EmuBlfImgFilePath};
#include <QDialog>
#include <QMap>
#include <QTemporaryFile>
namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();
    bool ParseFile(const QString &fileName);
    void closeEvent(QCloseEvent *event);

    static QString tempDirDateTime;
    static QString blf_folder;
    //QString tempDirDateTime;
    static QString AllBlfTextPreImage;//stores all the blf context before image loading
    static QString AllBlfTextFromImage;//stores all the blf context from image loading to end
    QMap<QString, QString> getImageConfig2() {return ImageConfig2;}
private:
    QMap<QString, QString> ImageConfig2;//for location2 and path2
    bool SubParseFile(QFile &fileTemp);

public slots:
    void SlotupdateCellDoubleClicked(int row, int column);
    void autoupload();
    void blfFileUpdate();
    void appendImageText(bool temp);
private:
    QTemporaryFile FileTemp;
    Ui::Dialog *ui;
    bool blfFileChange;
    QString blfFileName;
};

#endif // DIALOG_H
