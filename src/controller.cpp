#include "controller.h"
#include "ui_controller.h"

controller::controller(QWidget *parent, std::vector<elevator*> _eles, int _FLOOR_NUM, int _ELE_SELECT_MODE) : QWidget(parent), ui(new Ui::controller){
	ui->setupUi(this);
	eles = _eles;
    FLOOR_NUM = _FLOOR_NUM;
    ELE_NUM = int(eles.size());
	ELE_SELECT_MODE = _ELE_SELECT_MODE;
	srand(unsigned (time(nullptr)) );
    this->setWindowTitle("电梯控制视图");

    //draw stop Btns
   /* for (int i= 0; i < 5; i++) {
        QPushButton *StopBtn  =  new QPushButton(ui->groupBox_btns);//参数为按钮父节点  一般为this
        StopBtn->setGeometry(480, 30 + i*60, 120, 40);
        StopBtn->setText("STOP");
        StopBtn->show();
        StopBtns.push_back(StopBtn);
    }

    //draw elevator number lables
    for(int i = 0; i < 5; i++){
        QLabel *ELE_NUM = new QLabel(ui->groupBox_btns);//放数字label
            ELE_NUM->setGeometry( 10, 10, 30, 30);//增加位置
            ELE_NUM->setAlignment(Qt::AlignHCenter);//消除空隙
            ELE_NUM->setText(QString::number(i + 1));//数字标记  将数字转化成字符串！！函数
            ELE_NUM->show();//显示
     }

    //draw elevator floor lables
    for(int i = 0; i < 5; i++){
        QLabel *ELE_NUM = new QLabel(ui->groupBox_btns);//放数字label
            ELE_NUM->setGeometry( 10, 10, 30, 30);//增加位置
            ELE_NUM->setAlignment(Qt::AlignHCenter);//消除空隙
            ELE_NUM->setText(QString::number(i + 1));//数字标记  将数字转化成字符串！！函数
            ELE_NUM->show();//显示
     }

    //draw elevator status lables
    for(int i = 0; i < 5; i++){
        QLabel *ELE_NUM = new QLabel(ui->groupBox_btns);//放数字label
            ELE_NUM->setGeometry( 10, 10, 30, 30);//增加位置
            ELE_NUM->setAlignment(Qt::AlignHCenter);//消除空隙
            ELE_NUM->setText(QString::number(i + 1));//数字标记  将数字转化成字符串！！函数
            ELE_NUM->show();//显示
     }*/


	//set suitable box and window size
    ui->groupBox_btns->setGeometry(20, 20, 621, 421);
    //ui->groupBox_btns->setGeometry(10, 320, 430, FLOOR_NUM > 20 ? 20 + 120 * ((FLOOR_NUM) / 10 + 1) : 280);
    //ui->label_bar->setGeometry(10, FLOOR_NUM > 20 ? 340 + 120 * ((FLOOR_NUM) / 10 + 1) : 600, ELE_NUM > 10 ? 20 + 40 * ELE_NUM : 430, 20);
    this->resize(830,460);

	//every 100ms, refresh sliders
	QTimer *timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, &controller::timer_building_tick);
	timer->start(100);
}


controller::~controller(){
	delete ui;
}

void controller::renew_label(unsigned int i){
	eleSliders[i]->setValue(eles[i]->currentFloor + 1);
	eleCurrents[i]->setText(QString::number(eles[i]->currentFloor + 1));
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
		renew_label(i);
		if(eles[i]->status == 0){
			if(ELE_SELECT_MODE == 3){
                //ui->label_bar->setText(QString::number(i + 1, 10) +"号电梯到达, 取消其他电梯请求.");
				for(auto j : eles) j->cancel_request(eles[i]->currentFloor);
			}
			floorBtnsUp[unsigned(eles[i]->currentFloor)]->setEnabled(true);
			floorBtnsDown[unsigned(eles[i]->currentFloor)]->setEnabled(true);
		}
	}
}

void controller::display_alert(int ele_no){
    QMessageBox::about(nullptr, "Alert!", "电梯：" + QString::number(ele_no) + "已发出警报！");
}

