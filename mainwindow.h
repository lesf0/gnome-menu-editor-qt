#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "editdfile.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QMap<QString,QMap<QString,QPair<QString, bool> > >* cats;
    EditDFile* dfile;
private slots:
    void switchcat(QString cat_id);
    void switchelem(QString name);
    void execcurelem();
    void additem();
    void hideitem();
    void visitem();
    void switchhiddenshow(bool flag);
    void showaboutmb();
};

#endif // MAINWINDOW_H
