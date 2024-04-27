#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMouseEvent>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //设置窗口属性为没有边框的窗口
    setWindowFlag(Qt::FramelessWindowHint);
    //固定窗口大小
    setFixedSize(width(), height());
    exitMenu = new QMenu(this);
    exitAct = new QAction;
    exitAct->setText(u8"退出");
    exitAct->setIcon(QIcon(":/weatherIco/close.ico"));
    exitMenu->addAction(exitAct);
    connect(exitAct, SIGNAL(triggered(bool)), this, SLOT(slot_exitApp()));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::contextMenuEvent(QContextMenuEvent* event){
    exitMenu->exec(QCursor::pos());
    event->accept();
}

void MainWindow::slot_exitApp(){
    qApp->exit(0);
}

void MainWindow::mouseMoveEvent(QMouseEvent* event){
    move(event->globalPos() - mPos);
}

void MainWindow::mousePressEvent(QMouseEvent* event){
    mPos = event->globalPos() - this->pos();
}
