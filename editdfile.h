#ifndef EDITDFILE_H
#define EDITDFILE_H

#include <QDialog>
#include <QFile>

namespace Ui {
class EditDFile;
}

class EditDFile : public QDialog
{
    Q_OBJECT

public:
    explicit EditDFile(QString filename, QString name="", QString cats="", QWidget *parent = 0);
    ~EditDFile();
    const QString getProp(const QString propName);
    void writeToFile();
    void makeHidden();
    void makeVisible();
private:
    Ui::EditDFile *ui;
    QMap<QString,QObject*>* tf_map;
    QMap<QString, QMap <QString, QMap<QString,QString> > >* e_map;
    QFile* file;
private slots:
    void showhidemore(bool flag);
    void switchlocale(QString locale_id);
    void cbChangesHandler(bool flag);
    void leChangesHandler(QString str);
    void teChangesHandler();
    void browseButtonHandler();
    void delLocale();
    void addLocale();
    void switchentry(QString entry_id);
    void addentry();
    void delentry();
};

#endif // EDITDFILE_H
