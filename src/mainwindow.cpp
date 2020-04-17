#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
	ui->setupUi(this);
    connect(ui->pushButton_exit, &QPushButton::clicked, this, [=]{exit(0);});
    connect(ui->pushButton_run, &QPushButton::clicked, this, &MainWindow::run);
    connect(ui->pushButton_stop, &QPushButton::clicked, this, &MainWindow::my_stop);
	ui->pushButton_stop->setEnabled(false);
}

MainWindow::~MainWindow(){
	delete ui;
}

void MainWindow::run(){
	int FLOOR_NUM = 56;
	int ELE_NUM = 6;
	for(int i = 0; i < ELE_NUM; i++){
        elevator *ele = new elevator(nullptr, i, FLOOR_NUM);
    // windows
		ele->show();
        eles.push_back(ele);
	}

	building *bld = new building(nullptr, eles, FLOOR_NUM, 1);
	bld->move(100,100);
	bld->show();
	a_building = bld;
    controller *ctl = new controller(nullptr, eles, FLOOR_NUM, 1);
    for(int i = 0;i < ELE_NUM; i++){
        eles[i]->setController(ctl);
    }

    for (int i = 0; i < ELE_NUM; ++i) {
        eles[i]->setController(ctl);
    }

    ctl->move(100,100);
    ctl->show();
    a_controller = ctl;

	ui->pushButton_run->setEnabled(false);
	ui->pushButton_stop->setEnabled(true);
}

void MainWindow::my_stop(){
	for(auto i : eles) i->close();
	eles.clear();
	a_building->close();
	a_building = nullptr;
	ui->pushButton_run->setEnabled(true);
	ui->pushButton_stop->setEnabled(false);
}


