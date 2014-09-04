#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if(argc==1){ //simply launch a program
        MainWindow w;
        w.show();

        return a.exec();
    }else{ //just open specified .desktop file for editing, save if necessary and exit
        EditDFile d(argv[1]);
        if(d.exec()==QDialog::Accepted){
            d.writeToFile();
        }
    }
}
