#include "controller.h"
#include "ui_controller.h"

controller::controller(QWidget *parent, std::vector<elevator*> _eles, int _FLOOR_NUM, int _ELE_SELECT_MODE) : QWidget(parent), ui(new Ui::controller){
	ui->setupUi(this);
	eles = _eles;
    FLOOR_NUM = _FLOOR_NUM;
    ELE_NUM = int(eles.size());//当前电梯数量
	ELE_SELECT_MODE = _ELE_SELECT_MODE;
	srand(unsigned (time(nullptr)) );
    this->setWindowTitle("电梯控制视图");

	//set suitable box and window size
    ui->groupBox_btns->setGeometry(20, 20, 621, 421);
    this->resize(830,460);
	//every 100ms, refresh sliders
	QTimer *timer = new QTimer(this);
	timer->start(100);

    connect(timer, &QTimer::timeout, this, &controller::timer_building_tick);
    connect(ui->ele_stop_1,&QPushButton::clicked,this,[=]{stop_ele(0);});
    connect(ui->ele_stop_2,&QPushButton::clicked,this,[=]{stop_ele(1);});
    connect(ui->ele_stop_3,&QPushButton::clicked,this,[=]{stop_ele(2);});
    connect(ui->ele_stop_4,&QPushButton::clicked,this,[=]{stop_ele(3);});
    connect(ui->ele_stop_5,&QPushButton::clicked,this,[=]{stop_ele(4);});
    connect(ui->ele_stop_6,&QPushButton::clicked,this,[=]{stop_ele(5);});
    connect(ui->pushButton_7,&QPushButton::clicked,this,[=]{Emergency();});
    connect(ui->ele_reset_1,&QPushButton::clicked,this,[=]{reset(0);});
    connect(ui->ele_reset_2,&QPushButton::clicked,this,[=]{reset(1);});
    connect(ui->ele_reset_3,&QPushButton::clicked,this,[=]{reset(2);});
    connect(ui->ele_reset_4,&QPushButton::clicked,this,[=]{reset(3);});
    connect(ui->ele_reset_5,&QPushButton::clicked,this,[=]{reset(4);});
    connect(ui->ele_reset_6,&QPushButton::clicked,this,[=]{reset(5);});
    connect(ui->pushButton_8,&QPushButton::clicked,this,[=]{resume();});
    connect(ui->pushButton,&QPushButton::clicked,this,[=]{Full_load();});
    connect(ui->pushButton_2,&QPushButton::clicked,this,[=]{Overload();});
}



controller::~controller(){
	delete ui;
}


bool controller::receive_request(int ele_no){
    display_alert(ele_no);
}

void controller::timer_building_tick(){
	for(unsigned int i = 0; i < unsigned(ELE_NUM); i++){
        renew_label();
		if(eles[i]->status == 0 || eles[i]->status == 3){
			if(ELE_SELECT_MODE == 3){
				for(auto j : eles) j->cancel_request(eles[i]->currentFloor);
			}
		}
	}
}

void controller::display_alert(int ele_no){
    QMessageBox::about(nullptr, "Alert!", "电梯：" + QString::number(ele_no) + "已发出警报！");
    eles[ele_no]->destsInsider.clear();
    eles[ele_no]->destsOutside.clear();
    eles[ele_no]->recive_request(false,1,true);
    reset(ele_no);
}

void controller::renew_label(){
        ui->label_26->setText(eles[0]->statusStr[eles[0]->status]);
        ui->label_19->setText(QString::number(eles[0]->trueCurrentFloor));

        ui->label_27->setText(eles[1]->statusStr[eles[1]->status]);
        ui->label_20->setText(QString::number(eles[1]->trueCurrentFloor));

        ui->label_25->setText(eles[2]->statusStr[eles[2]->status]);
        ui->label_18->setText(QString::number(eles[2]->trueCurrentFloor));

        ui->label_24->setText(eles[3]->statusStr[eles[3]->status]);
        ui->label_17->setText(QString::number(eles[3]->trueCurrentFloor));

        ui->label_28->setText(eles[4]->statusStr[eles[4]->status]);
        ui->label_21->setText(QString::number(eles[4]->trueCurrentFloor));

        ui->label_22->setText(eles[5]->statusStr[eles[5]->status]);
        ui->label_15->setText(QString::number(eles[5]->trueCurrentFloor));

}

void controller::stop_ele(int ele){
    eles[ele]->status = 3;
    eles[ele]->door = 1;//eles是一个向量组  eles[i]才是一个对象指针
    eles[ele]->destsInsider.clear();
    eles[ele]->destsOutside.clear();
}

void controller::Emergency(){
    for(int i = 0;i<6;i++){
        stop_ele(i);
    }
}

void controller::reset(int ele){
    eles[ele]->status = 0;
    eles[ele]->door = 0;
    eles[ele]->Qbtns[eles[ele]->currentFloor]->setEnabled(true);

}

void controller::resume(){
    for(int i = 0;i<6;i++){
        reset(i);
    }
}

void controller::Full_load(){
    eles[1]->status = 6;
    eles[1]->door = 1;//eles是一个向量组  eles[i]才是一个对象指针
}

void controller::Overload(){
    eles[1]->status = 3;
    eles[1]->door = 1;//eles是一个向量组  eles[i]才是一个对象指针
}
