#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

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
}
