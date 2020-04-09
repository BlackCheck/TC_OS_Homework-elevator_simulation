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
    // 设置层数
//	int FLOOR_NUM = ui->spinBox_floor->value();
	int FLOOR_NUM = 56;
	int ELE_NUM = 6;

	for(int i = 0; i < ELE_NUM; i++){
		elevator *ele = new elevator(nullptr, i, FLOOR_NUM);

		// windows
//		ele->move( 5 + i % (GetSystemMetrics(SM_CXSCREEN) / ele->width()) * (ele->width() + 5),
//				   5 + ((i + 1) * ele->width() / GetSystemMetrics(SM_CXSCREEN)) * (ele->height() + 35)
//				 );

		ele->show();
		eles.push_back(ele);
	}
	building *bld = new building(nullptr, eles, FLOOR_NUM, 1);
	bld->move(100,100);
	bld->show();
	a_building = bld;

    controller *ctrl = new controller(nullptr, eles, FLOOR_NUM, 1);
    ctrl->move(100,100);
    ctrl->show();
    a_controller = ctrl;

    for(int i = 0; i < ELE_NUM; i++){
        eles[i]->setController(ctrl);
    }

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


