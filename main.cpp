#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
<<<<<<< HEAD

    if(argc==1){
        MainWindow w;
        w.show();

        return a.exec();
    }else{
        EditDFile d(argv[1]);
        if(d.exec()==QDialog::Accepted){
            d.writeToFile();
        }
    }
=======
    MainWindow w;
    w.show();

    return a.exec();
>>>>>>> 3d29f2021102e022c37d1b9087950df263b71a42
}
