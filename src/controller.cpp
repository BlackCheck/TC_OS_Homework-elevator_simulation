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
    connect(ui->ele_stop_1, &QPushButton::clicked, this, [=]{send_stop_request(eles[0]);});
    connect(ui->ele_reset_1, &QPushButton::clicked, this, [=]{reset_ele(0);});
	//every 100ms, refresh sliders
	QTimer *timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, &controller::timer_building_tick);
	timer->start(100);
}


controller::~controller(){
	delete ui;
}


bool controller::send_request(bool up, int floor, elevator *ele, bool forceRecive){
    return(ele->recive_request(up, floor, forceRecive));
}

bool controller::receive_request(int ele_no){
    display_alert(ele_no);
}

// 优先级调度算法
int controller::ele_rate(bool reqUp, int reqFloor, int eleFloor, int eleStatus){
	if(reqFloor == eleFloor) return 10000;
	double distanceRating = double(abs(eleFloor - reqFloor)) / double(FLOOR_NUM);
	if(eleStatus == 0) distanceRating *= 3;
	double statusRating = eleStatus == 0 ? 1.0 : reqUp ? eleStatus == 1 ? eleFloor < reqFloor ? 1.0 : 0.2
																		: eleFloor < reqFloor ? 0.6 : 0.4
													   : eleStatus == 2 ? eleFloor > reqFloor ? 1.0 : 0.2
																		: eleFloor > reqFloor ? 0.6 : 0.4;
	return int(100.0 * (distanceRating * 0.6 + statusRating * 0.4));
}

// 控制端发送调用电梯的请求 true = up false = down
void controller::ele_select_send(bool up, int floor){
    //ui->label_bar->setText("开始处理来自"+ QString::number(floor + 1, 10) +"层的电梯调度请求...");
	if(ELE_SELECT_MODE == 1){
		eleRatings.clear();


		for(int i = 0; i < ELE_NUM; i++)
			eleRatings.push_back({i, ele_rate(up, floor, eles[unsigned(i)]->currentFloor, eles[unsigned(i)]->status)});
		    std::sort(eleRatings.begin(), eleRatings.end(), [](std::pair<int, int> &a, std::pair<int, int> &b){
						return a.second < b.second;
		    });

		bool successSend = false;
		for(auto i : eleRatings) {
			if(send_request(up, floor, eles[unsigned(i.first)])){
				successSend = true;
                //ui->label_bar->setText("已为来自"+ QString::number(floor + 1, 10) +"层的请求调度" + QString::number(i.first + 1, 10) + "号电梯.");
				return;
			}else{
                //ui->label_bar->setText("为来自"+ QString::number(floor + 1, 10) +"层调度" + QString::number(i.first + 1, 10) + "号电梯的请求被拒绝.");
				continue;
			}
		}

		// 强制调度
		if(successSend == false) {
			send_request(up, floor, eles[unsigned(eleRatings.begin()->first)], true);
            //ui->label_bar->setText("已为来自"+ QString::number(floor + 1, 10) +"层的请求强制调度" + QString::number(eleRatings.begin()->first + 1, 10) + "号电梯.");
		}



	}

	// 优化算法1，舍弃其余
	else if(ELE_SELECT_MODE == 2) {
		int temp = rand() % (ELE_NUM);
		send_request(up, floor, eles[unsigned(temp)], true);
        //ui->label_bar->setText("已为来自"+ QString::number(floor + 1, 10) +"层的请求选择" + QString::number(temp + 1, 10) + "号电梯.");
	}
	else {
		for(auto i : eles){
			send_request(up, floor, i, true);
            //ui->label_bar->setText("已将来自"+ QString::number(floor + 1, 10) +"层的请求发送至所有电梯.");
		}
	}
}

void controller::timer_building_tick(){
	for(unsigned int i = 0; i < unsigned(ELE_NUM); i++){
        renew_label();
		if(eles[i]->status == 0 || eles[i]->status == 3){
			if(ELE_SELECT_MODE == 3){
                //ui->label_bar->setText(QString::number(i + 1, 10) +"号电梯到达, 取消其他电梯请求.");
				for(auto j : eles) j->cancel_request(eles[i]->currentFloor);
			}
//			floorBtnsUp[unsigned(eles[i]->currentFloor)]->setEnabled(true);
//			floorBtnsDown[unsigned(eles[i]->currentFloor)]->setEnabled(true);
		}
	}
}

void controller::display_alert(int ele_no){
    QMessageBox::about(nullptr, "Alert!", "电梯：" + QString::number(ele_no) + "已发出警报！");
}

void controller::send_stop_request(elevator * ele){
    return(recive_stop_request(ele));
}

void controller::recive_stop_request(elevator * ele){
    if (ele->door) {
        ui->ele_stop_1->setEnabled(false);
		ele->status = 3;
        while(~ele->status) ele->recive_request(2, 1, true);
	}
	else {
        if (ele->status == 1){
            ele->recive_request(true, ele->currentFloor + 1, true);
		}
		else {
            ele->recive_request(false, ele->currentFloor - 1, true);
		}
        while(reset_ele(ele->no)) ele->recive_request(2, 1, true);
	}
}

void controller::reset_ele(int ele){
	eles[ele]->status = 0;
}

void controller::renew_label(){
        ui->label_26->setText(eles[0]->statusStr[eles[0]->status]);
        ui->label_19->setText(QString::number(eles[0]->currentFloor));

        ui->label_27->setText(eles[1]->statusStr[eles[1]->status]);
        ui->label_20->setText(QString::number(eles[1]->currentFloor));

        ui->label_25->setText(eles[2]->statusStr[eles[2]->status]);
        ui->label_18->setText(QString::number(eles[2]->currentFloor));

        ui->label_24->setText(eles[3]->statusStr[eles[3]->status]);
        ui->label_17->setText(QString::number(eles[3]->currentFloor));

        ui->label_28->setText(eles[4]->statusStr[eles[4]->status]);
        ui->label_21->setText(QString::number(eles[4]->currentFloor));

        ui->label_22->setText(eles[5]->statusStr[eles[5]->status]);
        ui->label_15->setText(QString::number(eles[5]->currentFloor));

}

