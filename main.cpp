#include "mainwindow.h"
#include <QApplication>
#include "env.h"

#ifdef time_test
#include <time.h>
#include <QDebug>
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if(argc==1){ //simply launch a program
#ifdef time_test
        auto t1=clock();
#endif
        MainWindow w;
#ifdef time_test
        auto t2=clock();
        qDebug()<<((((float)t2 - (float)t1) / 1000000.0F ) * 1000);
#endif
        w.show();

        return a.exec();
    }else{ //just open specified .desktop file for editing, save if necessary and exit
        EditDFile d(argv[1]);
        if(d.exec()==QDialog::Accepted){
            d.writeToFile();
        }
    }
}
