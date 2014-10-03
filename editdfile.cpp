#include "editdfile.h"
#include "ui_editdfile.h"
#include <QMap>
#include <QMessageBox>
#include <QTextStream>
#include <QFileDialog>
#include <QInputDialog>
#include <QDir>
#include <QFile>
#include "env.h"

EditDFile::EditDFile(QString filename, QString name, QString cats, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditDFile)
{
    ui->setupUi(this);

    connect(ui->scbShowMore,SIGNAL(toggled(bool)),this,SLOT(showhidemore(bool)));
    ui->scbShowMore->setChecked(false);

    QList<QLineEdit*> text_fields=this->findChildren<QLineEdit*>(QRegExp("^text*")); //connect all text fields and checkboxes to handlers
    QList<QCheckBox*> cb_fields=this->findChildren<QCheckBox*>(QRegExp("^cb*"));
    QList<QTextEdit*> ltext_fields=this->findChildren<QTextEdit*>(QRegExp("^ltext*"));
    tf_map=new QMap<QString,QObject*>;

    for(QList<QLineEdit*>::iterator it=text_fields.begin();it!=text_fields.end();++it){
        (*tf_map)[(*it)->objectName().mid(4)]=*it;
        connect(*it,SIGNAL(textChanged(QString)),this,SLOT(leChangesHandler(QString)));
    }

    for(QList<QCheckBox*>::iterator it=cb_fields.begin();it!=cb_fields.end();++it){
        (*tf_map)[(*it)->objectName().mid(2)]=*it;
        connect(*it,SIGNAL(toggled(bool)),this,SLOT(cbChangesHandler(bool)));
    }

    for(QList<QTextEdit*>::iterator it=ltext_fields.begin();it!=ltext_fields.end();++it){
        (*tf_map)[(*it)->objectName().mid(5)]=*it;
        connect(*it,SIGNAL(textChanged()),this,SLOT(teChangesHandler()));
    }

    e_map=new QMap<QString, QMap <QString, QMap<QString,QString> > >; //contents of .desktop file
    QString entry=default_entry;
    (*e_map)[entry][default_locale]["Type"]="Application"; //default initialisation for empty files

    if(QFile::exists(filename)){
        if(filename.left(sizeof(default_path)-1)==default_path){ //if file is in /usr save new file in ~/
            QString filename_new=QDir::homePath()+"/.local"+filename.mid(4);
            QFile::copy(filename,filename_new);
            filename=filename_new;
        }
    }else{
        if(filename.left(sizeof(default_path)-1)==default_path){ //if file not exists create it in ~/ anyway
            filename=QDir::homePath()+"/.local"+filename.mid(4);
        }
    }

    file=new QFile(filename);
    if(file->exists()){
        if(!file->open(QIODevice::ReadOnly)) {
            QMessageBox::information(0, "error", file->errorString());
        }else{
            QTextStream in(file);
            while(!in.atEnd()){
                QString line=in.readLine();
                if(line.size()==0 || line[0]=='#'){
                    //comment, do nothing
                }else if(line[0]=='['){
                    entry=line; //remember entry and set default locale
                    (*e_map)[entry][default_locale];
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

                    if(value.trimmed()!=""){
                        (*e_map)[entry][locale_id][param]=value; //add value to proper place in array
                    }
                }
            }
        }
        file->close();
    }else{
        (*e_map)[entry][default_locale]["Name"]=name; //set name and category from args if file is empty
        (*e_map)[entry][default_locale]["Categories"]=cats;
    }

    connect(ui->listLocale,SIGNAL(currentTextChanged(QString)),this,SLOT(switchlocale(QString)));

    for(QMap<QString, QMap <QString, QMap<QString,QString> > >::iterator it=e_map->begin();it!=e_map->end();++it){ //add entrys
        ui->comboEntry->addItem(it.key());
    }

    connect(ui->comboEntry,SIGNAL(currentIndexChanged(QString)),this,SLOT(switchentry(QString)));
    ui->comboEntry->setCurrentIndex(-1);
    ui->comboEntry->setCurrentIndex(ui->comboEntry->count()-1); //select entry and fill everything from emitted signal

    connect(ui->buttonExec,SIGNAL(clicked()),this,SLOT(browseButtonHandler()));
    connect(ui->buttonTryExec,SIGNAL(clicked()),this,SLOT(browseButtonHandler()));
    connect(ui->buttonIcon,SIGNAL(clicked()),this,SLOT(browseButtonHandler()));
    connect(ui->buttonPath,SIGNAL(clicked()),this,SLOT(browseButtonHandler()));

    connect(ui->buttonLocaleDel,SIGNAL(clicked()),this,SLOT(delLocale()));
    connect(ui->buttonLocaleAdd,SIGNAL(clicked()),this,SLOT(addLocale()));

    connect(ui->buttonAddEntry,SIGNAL(clicked()),this,SLOT(addentry()));
    connect(ui->buttonDelEntry,SIGNAL(clicked()),this,SLOT(delentry()));
}

void EditDFile::switchentry(QString entry_id){
    if(entry_id!=""){
        ui->listLocale->clear();
        QMap<QString,QMap<QString,QString> >* locales = &(*e_map)[entry_id];
        for(QMap<QString,QMap<QString,QString> >::iterator it=locales->begin();it!=locales->end();++it){
            ui->listLocale->addItem(it.key());
        }
        ui->listLocale->setCurrentRow(0);
    }
}

void EditDFile::addentry(){
    QString entry_id=QInputDialog::getText(this,"New entry","Entry name (with [])").trimmed();
    if(entry_id!=""){
        (*e_map)[entry_id][default_locale];
        ui->comboEntry->addItem(entry_id);
        ui->comboEntry->setCurrentIndex(-1);
        ui->comboEntry->setCurrentIndex(ui->comboEntry->count()-1);
    }
}

void EditDFile::delentry(){
    QString entry_id=ui->comboEntry->currentText();
    if(entry_id!=default_entry){
        e_map->erase(e_map->find(entry_id));
        ui->comboEntry->removeItem(ui->comboEntry->currentIndex());
    }
}

void EditDFile::writeToFile(){
    if(!file->open(QIODevice::WriteOnly)) {
        QMessageBox::information(0, "error", file->errorString());
    }else{
        QTextStream out(file);
        for(QMap<QString, QMap <QString, QMap<QString,QString> > >::iterator it3=e_map->begin();it3!=e_map->end();++it3){
            out<<it3.key()<<'\n';
            for(QMap<QString, QMap<QString,QString> >::iterator it2=it3.value().begin();it2!=it3.value().end();++it2){
                if(it2.key()==default_locale){
                    for(QMap<QString,QString>::iterator it=it2.value().begin();it!=it2.value().end();++it){
                        out<<it.key()<<'='<<it.value()<<'\n';
                    }
                }else{
                    for(QMap<QString,QString>::iterator it=it2.value().begin();it!=it2.value().end();++it){
                        out<<it.key()<<'['<<it2.key()<<"]="<<it.value()<<'\n';
                    }
                }
            }
        }
        out.flush();
    }
    file->close();
}

EditDFile::~EditDFile()
{
    delete tf_map;
    delete e_map;
    delete file;

    delete ui;
}

void EditDFile::cbChangesHandler(bool flag){
    if(flag){
        (*e_map)[ui->comboEntry->currentText()][ui->listLocale->currentItem()->text()][sender()->objectName().mid(2)]="true";
    }else if((*e_map)[ui->comboEntry->currentText()][ui->listLocale->currentItem()->text()].count(sender()->objectName().mid(2))){
        (*e_map)[ui->comboEntry->currentText()][ui->listLocale->currentItem()->text()].erase((*e_map)[ui->comboEntry->currentText()][ui->listLocale->currentItem()->text()].find(sender()->objectName().mid(2)));
    }else{

    }
}

void EditDFile::leChangesHandler(QString str){
    if(str.trimmed()!=""){
        (*e_map)[ui->comboEntry->currentText()][ui->listLocale->currentItem()->text()][sender()->objectName().mid(4)]=str;
    }else if((*e_map)[ui->comboEntry->currentText()][ui->listLocale->currentItem()->text()].count(sender()->objectName().mid(4))){
        (*e_map)[ui->comboEntry->currentText()][ui->listLocale->currentItem()->text()].erase((*e_map)[ui->comboEntry->currentText()][ui->listLocale->currentItem()->text()].find(sender()->objectName().mid(4)));
    }else{

    }
}

void EditDFile::teChangesHandler(){
    QTextEdit* qte=qobject_cast<QTextEdit*>(sender());
    if(qte){
        if(qte->toPlainText().trimmed()!=""){
            (*e_map)[ui->comboEntry->currentText()][ui->listLocale->currentItem()->text()][qte->objectName().mid(5)]=qte->toPlainText();
        }else if((*e_map)[ui->comboEntry->currentText()][ui->listLocale->currentItem()->text()].count(sender()->objectName().mid(5))){
            (*e_map)[ui->comboEntry->currentText()][ui->listLocale->currentItem()->text()].erase((*e_map)[ui->comboEntry->currentText()][ui->listLocale->currentItem()->text()].find(sender()->objectName().mid(5)));
        }else{

        }
    }
}

void EditDFile::switchlocale(QString locale_id){
    if(locale_id!=""){
        for(QMap<QString,QObject*>::iterator it=tf_map->begin();it!=tf_map->end();++it){
            QLineEdit* qle=qobject_cast<QLineEdit*>(it.value());
            if(qle){
                qle->setText((*e_map)[ui->comboEntry->currentText()][locale_id].count(it.key())?(*e_map)[ui->comboEntry->currentText()][locale_id][it.key()]:"");
            }
            QTextEdit* qte=qobject_cast<QTextEdit*>(it.value());
            if(qte){
                qte->setText((*e_map)[ui->comboEntry->currentText()][locale_id].count(it.key())?(*e_map)[ui->comboEntry->currentText()][locale_id][it.key()]:"");
            }
            QCheckBox* qcb=qobject_cast<QCheckBox*>(it.value());
            if(qcb){
                qcb->setChecked((*e_map)[ui->comboEntry->currentText()][locale_id].count(it.key()) && (*e_map)[ui->comboEntry->currentText()][locale_id][it.key()]=="true");
            }
        }
    }
}

void EditDFile::browseButtonHandler(){
    QString target=sender()->objectName().mid(6);
    QString filename="";
    if(target=="Icon"){
        filename=QFileDialog::getOpenFileName(this,"Open file", "/usr/share/icons", "Icon Files (*.png *.jpg *.bmp *.svg *.ico)");
    }else if(target=="Path"){
        filename=QFileDialog::getExistingDirectory(this,"Open directory","/usr/bin");
    }else{
        filename=QFileDialog::getOpenFileName(this,"Open file", "/usr/bin");
    }
    if(filename!=""){
        QLineEdit* qle=qobject_cast<QLineEdit*>((*tf_map)[target]);
        qle->setText(filename);
    }
}

void EditDFile::delLocale(){
    QString locale_id=ui->listLocale->currentItem()->text();
    if(locale_id!=default_locale){
        (*e_map)[ui->comboEntry->currentText()].erase((*e_map)[ui->comboEntry->currentText()].find(locale_id));
        delete ui->listLocale->currentItem();
    }
}

void EditDFile::addLocale(){
    QString locale_id=QInputDialog::getText(this,"New locale","Locale ID").trimmed();
    if(locale_id!=""){
        ui->listLocale->addItem(locale_id);
        ui->listLocale->setCurrentRow(ui->listLocale->count()-1);
    }
}

void EditDFile::showhidemore(bool flag){
    if(flag){
        ui->labelActions->show();
        ui->labelKeywords->show();
        ui->labelMimeType->show();
        ui->labelNotShowIn->show();
        ui->labelOnlyShowIn->show();
        ui->labelStartupWMClass->show();
        ui->labelTryExec->show();

        ui->textActions->show();
        ui->textKeywords->show();
        ui->textMimeType->show();
        ui->ltextNotShowIn->show();
        ui->ltextOnlyShowIn->show();
        ui->textStartupWMClass->show();
        ui->textTryExec->show();

        ui->buttonTryExec->show();

        ui->cbDBUsActivatable->show();
        ui->cbHidden->show();
        ui->cbNoDisplay->show();
        ui->cbStartupNotify->show();
        ui->cbTerminal->show();

        ui->buttonAddEntry->show();
        ui->buttonDelEntry->show();
        ui->labelEntry->show();
        ui->comboEntry->show();
    }else{
        ui->labelActions->hide();
        ui->labelKeywords->hide();
        ui->labelMimeType->hide();
        ui->labelNotShowIn->hide();
        ui->labelOnlyShowIn->hide();
        ui->labelStartupWMClass->hide();
        ui->labelTryExec->hide();

        ui->textActions->hide();
        ui->textKeywords->hide();
        ui->textMimeType->hide();
        ui->ltextNotShowIn->hide();
        ui->ltextOnlyShowIn->hide();
        ui->textStartupWMClass->hide();
        ui->textTryExec->hide();

        ui->buttonTryExec->hide();

        ui->cbDBUsActivatable->hide();
        ui->cbHidden->hide();
        ui->cbNoDisplay->hide();
        ui->cbStartupNotify->hide();
        ui->cbTerminal->hide();

        ui->buttonAddEntry->hide();
        ui->buttonDelEntry->hide();
        ui->labelEntry->hide();
        ui->comboEntry->hide();
    }

    layout()->setSizeConstraint(QLayout::SetFixedSize); //workaround for size changing
}

void EditDFile::makeHidden(){
    ui->cbHidden->toggle();
    this->writeToFile();
}

void EditDFile::makeVisible(){
    ui->cbNoDisplay->toggle();
    this->writeToFile();
}

const QString EditDFile::getProp(const QString propName){
    return (*e_map)[default_entry][default_locale][propName];
}
