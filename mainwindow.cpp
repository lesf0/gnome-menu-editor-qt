#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDirIterator>
#include <QFileInfo>
#include <QMap>
#include <QStringList>
#include "env.h"
#include <QDir>
#include <QTextStream>
#include <QMessageBox>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->actionQuit,SIGNAL(triggered()),this,SLOT(close()));
    connect(ui->actionAbout,SIGNAL(triggered()),this,SLOT(showaboutmb()));

    cats=new QMap<QString,QMap<QString,QPair<QString, bool> > >; //contents of all .desktop files is stored here

    QStringList pathes; //search pathes of .desktop files
    pathes.append(default_path);
    pathes.append(QDir::homePath()+"/.local/share/applications");
    for(QStringList::Iterator it2=pathes.begin();it2!=pathes.end();++it2){
        for(QDirIterator it(*it2,QDirIterator::Subdirectories);it.hasNext();it.next()){
            if(QFileInfo(it.filePath()).isFile() && QFileInfo(it.filePath()).suffix()=="desktop"){ //.desktop file found
                QFile file(it.filePath());
                if(file.open(QIODevice::ReadOnly)) {
                    QTextStream in(&file);
                    QString entry=default_entry; //setting defaults to obtain errors on empty files
                    QString name=default_name;
                    QString cat=default_cat;
                    bool hidden=false;
                    while(!in.atEnd()){
                        QString line=in.readLine();
                        if(line.size()==0 || line[0]=='#'){
                            //just skip comment, nothing to do here
                        }else if(line[0]=='['){
                            entry=line; //remember current entry
                        }else{
                            QString locale_id=default_locale;
                            QString param;
                            int iob;
                            if((iob=line.indexOf('['))!=-1 && iob<line.indexOf('=')){
                                locale_id=line.mid(iob+1,line.indexOf(']')-iob-1);
                                param=line.left(iob);
                            }else{
                                param=line.left(line.indexOf('='));
                            }
                            QString value=line.mid(line.indexOf('=')+1);

                            if(locale_id==default_locale && entry==default_entry){ //remember default entry & default locale settings
                                if(param=="Name"){
                                    name=value;
                                }else if(param=="Categories"){
                                    cat+=value;
                                }else if(param=="Hidden"){
                                    hidden=(value=="true");
                                }else{
                                    //unnecessary params, skipping
                                }
                            }
                        }
                    }
                    int ioc; //get names of all categories, including default category; add filename to every category which includes it
                    while((ioc=cat.indexOf(';'))!=-1){
                        (*cats)[cat.left(ioc).trimmed()][name]=QPair<QString,bool>(it.filePath(),hidden);
                        cat=cat.mid(ioc+1);
                    }
                }
                file.close();
            }
        }
    }

    connect(ui->comboCat,SIGNAL(currentIndexChanged(QString)),this,SLOT(switchcat(QString)));

    dfile=0;

    connect(ui->listCat,SIGNAL(currentTextChanged(QString)),this,SLOT(switchelem(QString)));
    connect(ui->buttonEditItem,SIGNAL(clicked()),this,SLOT(execcurelem()));

    connect(ui->buttonAddItem,SIGNAL(clicked()),this,SLOT(additem()));

    connect(ui->buttonDelItem,SIGNAL(clicked()),this,SLOT(hideitem()));

    connect(ui->cbHidden,SIGNAL(toggled(bool)),this,SLOT(switchhiddenshow(bool)));

    for(QMap<QString,QMap<QString,QPair<QString, bool> > >::iterator it=cats->begin();it!=cats->end();++it){
        ui->comboCat->addItem(it.key());
    }
    ui->comboCat->setCurrentIndex(0); //workaround to emit signal and fill list properly
}

MainWindow::~MainWindow(){
    delete ui;
    delete dfile;
}

void MainWindow::switchhiddenshow(bool flag){
    for(int i=0;i<ui->listCat->count();++i){
        if((*cats)[ui->comboCat->currentText()][ui->listCat->item(i)->text()].second){
            ui->listCat->item(i)->setHidden(!flag);
        }
    }
}

void MainWindow::additem(){
    QString name=QInputDialog::getText(this,"File name","New file name").trimmed();
    if(!name.isEmpty()){
        QString filename=default_path+name+".desktop";
        QString cat="";
        if(ui->comboCat->currentIndex()){
            cat=ui->comboCat->currentText()+';';
        }
        if(dfile){
            delete dfile;
        }
        dfile=new EditDFile(filename,name,cat);
        dfile->writeToFile(); //create file with choosen name and default params, dfile will be recreated on list item adding/switching

        (*cats)[ui->comboCat->currentText()][name]=QPair<QString,bool>(filename,false); //add filename to current and default cats
        (*cats)[default_cat_name][name]=QPair<QString,bool>(filename,false);

        QListWidgetItem* item=new QListWidgetItem(name);
        ui->listCat->addItem(item);
        ui->listCat->setCurrentItem(item);
        ui->listCat->sortItems();
        ui->listCat->scrollToItem(item);
    }
}

void MainWindow::hideitem(){
    if(dfile){
        dfile->makeHidden();
        bool flag=(*cats)[ui->comboCat->currentText()][ui->listCat->currentItem()->text()].second;

        QString cat=default_cat+dfile->getProp("Categories");
        int ioc; //switch status in every cat
        while((ioc=cat.indexOf(';'))!=-1){
            (*cats)[cat.left(ioc).trimmed()][ui->listCat->currentItem()->text()].second=!flag;
            cat=cat.mid(ioc+1);
        }

        if(flag){
            ui->listCat->currentItem()->setTextColor(Qt::black);
            ui->listCat->currentItem()->setHidden(false);
        }else{
            ui->listCat->currentItem()->setTextColor(Qt::gray);
            ui->listCat->currentItem()->setHidden(!ui->cbHidden->isChecked());
            ui->listCat->setCurrentRow(ui->listCat->currentRow()+!ui->cbHidden->isChecked()); //switch to next element if current is not aviable
        }
    }
}

void MainWindow::switchelem(QString name){
    if(dfile){
        delete dfile;
    }
    if(!name.isEmpty()){ //workaround for empty categories bug
        dfile=new EditDFile((*cats)[ui->comboCat->currentText()][name].first);
    }else{
        dfile=0;
    }
}

void MainWindow::execcurelem(){
    if(dfile){
        QString name=dfile->getProp("Name");
        QString cat=default_cat+dfile->getProp("Categories");
        QString filename=(*cats)[default_cat_name][name].first;
        if(dfile->exec()==QDialog::Accepted){
            dfile->writeToFile();

            int ioc; //after successfully execution, delete current file info from all cats, and add again
            while((ioc=cat.indexOf(';'))!=-1){
                (*cats)[cat.left(ioc).trimmed()].erase((*cats)[cat.left(ioc).trimmed()].find(name));
                if (!(*cats)[cat.left(ioc).trimmed()].size()){
                    cats->erase(cats->find(cat.left(ioc).trimmed()));
                }
                cat=cat.mid(ioc+1);
            }

            name=dfile->getProp("Name");
            if(name.isEmpty()){
                name=default_name;
            }
            cat=default_cat+dfile->getProp("Categories");
            bool hidden=(dfile->getProp("Hidden")=="true");

            while((ioc=cat.indexOf(';'))!=-1){
                (*cats)[cat.left(ioc).trimmed()][name]=QPair<QString,bool>(filename,hidden);
                cat=cat.mid(ioc+1);
            }

            int ci=ui->comboCat->currentIndex();

            ui->comboCat->clear();
            for(QMap<QString,QMap<QString,QPair<QString, bool> > >::iterator it=cats->begin();it!=cats->end();++it){
                ui->comboCat->addItem(it.key());
            }

            ui->comboCat->setCurrentIndex(std::min(ci,ui->comboCat->count()-1)); //workaround for deleting last cat

            int li=0;
            while(li<ui->listCat->count() && ui->listCat->item(li)->text()!=name){
                ++li;
            }
            if(li==ui->listCat->count()){
                li=0;
            }
            ui->listCat->scrollToItem(ui->listCat->item(li));
            ui->listCat->setCurrentRow(li);
        }
    }
}

void MainWindow::switchcat(QString cat_id){
    if(!cat_id.isEmpty()){
        ui->listCat->clear();
        for(QMap<QString,QPair<QString, bool> >::iterator it=(*cats)[cat_id].begin();it!=(*cats)[cat_id].end();++it){
            QListWidgetItem* item=new QListWidgetItem(it.key());
            if(it.value().second){
                item->setTextColor(Qt::gray);
                //item->setHidden(!ui->cbHidden->isChecked()); //TODO: Why this not working?
            }
            ui->listCat->addItem(item);
        }
        ui->listCat->sortItems();
        ui->listCat->setCurrentItem(ui->listCat->item(0));

        ui->cbHidden->setChecked(!ui->cbHidden->isChecked()); //workaround for not working setHidden
        ui->cbHidden->setChecked(!ui->cbHidden->isChecked());
    }
}

void MainWindow::showaboutmb(){
    QMessageBox::about(this,"About","Simple Gnome menu editor written on QT5\n\nby Denis Sheremet\nzaycakitayca@xaker.ru\n\nLicense: WTFPL\n\nDevelopment version\n\nIf you like what I do, you may support me\nYandex.money: 41001853988430\nWMR: R379723001067\nbtc: 1GnmDreXwhSPPnxmmkhimVXR8VGQrwiS2f");
}
