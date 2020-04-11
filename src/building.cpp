#include "building.h"
#include "ui_building.h"
//elevator 是Qweight 的子类  此时传入的是一个对象指针动态数组 每一个变量都指向这个类
building::building(QWidget *parent, std::vector<elevator*> _eles, int _FLOOR_NUM, int _ELE_SELECT_MODE) : QWidget(parent), ui(new Ui::building){
	ui->setupUi(this);
	eles = _eles;
	FLOOR_NUM = _FLOOR_NUM;
	ELE_NUM = int(eles.size());
	ELE_SELECT_MODE = _ELE_SELECT_MODE;
	srand(unsigned (time(nullptr)) );
	this->setWindowTitle("电梯外部视图");

	//draw elevators
	for(int i = 0; i < ELE_NUM; i++){
        QLabel *eleNo = new QLabel(ui->groupBox_eles);// 层数
			eleNo->setGeometry(20 + 40 * i, 30, 20, 20);
            eleNo->setAlignment(Qt::AlignHCenter);// 消除间隙
			eleNo->setText(QString::number(i + 1, 10));
			eleNo->show();
		QSlider *eleSlider = new QSlider(ui->groupBox_eles);
			eleSlider->setGeometry(20 + 40 * i, 50, 20, 220);
			eleSlider->setMinimum(1);
			eleSlider->setMaximum(FLOOR_NUM);
			eleSlider->setSingleStep(1);
			eleSlider->setPageStep(1);
			eleSlider->show();
			eleSliders.push_back(eleSlider);
		QLabel *eleCurrent = new QLabel(ui->groupBox_eles);
			eleCurrent->setGeometry(20 + 40 * i, 270, 20, 20);
			eleCurrent->setAlignment(Qt::AlignHCenter);
			eleCurrent->setText("1");
			eleCurrent->show();
			eleCurrents.push_back(eleCurrent);
	}

	//draw btns
	for(int i = 0; i < FLOOR_NUM; i++){
		QLabel *floorNo = new QLabel(ui->groupBox_btns);
			floorNo->setGeometry(20 + 40 * (i % 10), 30 + 120 * (i / 10), 30, 30);
			floorNo->setAlignment(Qt::AlignHCenter);
			floorNo->setText(QString::number(i + 1, 10));
			floorNo->show();
		QPushButton *floorBtnUp = new QPushButton(ui->groupBox_btns);
			floorBtnUp->setGeometry(20 + 40 * (i % 10), 60  + 120 * (i / 10), 30, 30);
			floorBtnUp->setText("↑");
			floorBtnUp->show();
			floorBtnsUp.push_back(floorBtnUp);
			connect(floorBtnsUp[unsigned(i)], &QPushButton::clicked, this, [=]{floorBtnsUp[unsigned(i)]->setEnabled(false);});
			connect(floorBtnsUp[unsigned(i)], &QPushButton::clicked, this, [=]{ele_select_send(true, i);});
		QPushButton *floorBtnDown = new QPushButton(ui->groupBox_btns);
			floorBtnDown->setGeometry(20 + 40 * (i % 10), 100 + 120 * (i / 10), 30, 30);
			floorBtnDown->setText("↓");
			floorBtnDown->show();
			floorBtnsDown.push_back(floorBtnDown);
			connect(floorBtnsDown[unsigned(i)], &QPushButton::clicked, this, [=]{floorBtnsDown[unsigned(i)]->setEnabled(false);});
			connect(floorBtnsDown[unsigned(i)], &QPushButton::clicked, this, [=]{ele_select_send(false, i);});
	}
	floorBtnsDown[0]->hide();
	floorBtnsUp[unsigned(FLOOR_NUM) - 1]->hide();

	//set suitable box and window size
	ui->groupBox_eles->setGeometry(10, 10, ELE_NUM > 10 ? 20 + 40 * ELE_NUM : 430, 300);
	ui->groupBox_btns->setGeometry(10, 320, 430, FLOOR_NUM > 20 ? 20 + 120 * ((FLOOR_NUM) / 10 + 1) : 280);
	ui->label_bar->setGeometry(10, FLOOR_NUM > 20 ? 340 + 120 * ((FLOOR_NUM) / 10 + 1) : 600, ELE_NUM > 10 ? 20 + 40 * ELE_NUM : 430, 20);
	this->resize(ELE_NUM > 10 ? 40 + 40 * ELE_NUM : 450, FLOOR_NUM > 20 ? 360 + 120 * ((FLOOR_NUM) / 10 + 1) : 620);

	//every 100ms, refresh sliders
	QTimer *timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, &building::timer_building_tick);
	timer->start(100);
}


building::~building(){
	delete ui;
}

void building::renew_label(unsigned int i){
	eleSliders[i]->setValue(eles[i]->currentFloor + 1);
	eleCurrents[i]->setText(QString::number(eles[i]->currentFloor + 1));
}

bool building::send_request(bool up, int floor, elevator *ele, bool forceRecive){  //给哪个楼梯  //内外通信
    return(ele->recive_request(up, floor, forceRecive));  //ele函数
}


// 优先级调度算法  不好改  复杂算法  权重没法改
int building::ele_rate(bool reqUp, int reqFloor, int eleFloor, int eleStatus){//elestatus电梯状态
    if(reqFloor == eleFloor) return 10000;  //要求层数等于当前层数
	double distanceRating = double(abs(eleFloor - reqFloor)) / double(FLOOR_NUM);
    if(eleStatus == 0) distanceRating *= 3;//停住
	double statusRating = eleStatus == 0 ? 1.0 : reqUp ? eleStatus == 1 ? eleFloor < reqFloor ? 1.0 : 0.2
																		: eleFloor < reqFloor ? 0.6 : 0.4
													   : eleStatus == 2 ? eleFloor > reqFloor ? 1.0 : 0.2
																		: eleFloor > reqFloor ? 0.6 : 0.4;
    if(eleStatus == 3||eleStatus == 4){
        distanceRating = 0;
        statusRating = 0;
    }

    return int(100.0 * (distanceRating * 0.6 + statusRating * 0.4));
}

// 控制端发送调用电梯的请求 true = up false = down  外部控制
void building::ele_select_send(bool up, int floor){
	ui->label_bar->setText("开始处理来自"+ QString::number(floor + 1, 10) +"层的电梯调度请求...");
	if(ELE_SELECT_MODE == 1){
		eleRatings.clear();


        for(int i = 0; i < ELE_NUM; i++)//此floor是要求的楼层数
			eleRatings.push_back({i, ele_rate(up, floor, eles[unsigned(i)]->currentFloor, eles[unsigned(i)]->status)});
		    std::sort(eleRatings.begin(), eleRatings.end(), [](std::pair<int, int> &a, std::pair<int, int> &b){
                        return a.second < b.second;//对每层电梯进行权重排序 由大到小
		    });

		bool successSend = false;
        for(auto i : eleRatings) {//给第几个发
            if(send_request(up, floor, eles[unsigned(i.first)])){//i.first 是权重最大的电梯  但是语法不懂
				successSend = true;
				ui->label_bar->setText("已为来自"+ QString::number(floor + 1, 10) +"层的请求调度" + QString::number(i.first + 1, 10) + "号电梯.");
				return;
			}else{
				ui->label_bar->setText("为来自"+ QString::number(floor + 1, 10) +"层调度" + QString::number(i.first + 1, 10) + "号电梯的请求被拒绝.");
				continue;
			}
		}

		// 强制调度
		if(successSend == false) {
            send_request(up, floor, eles[unsigned(eleRatings.begin()->first)], true);//？
			ui->label_bar->setText("已为来自"+ QString::number(floor + 1, 10) +"层的请求强制调度" + QString::number(eleRatings.begin()->first + 1, 10) + "号电梯.");
		}


	}

	// 优化算法1，舍弃其余
	else if(ELE_SELECT_MODE == 2) {
		int temp = rand() % (ELE_NUM);
		send_request(up, floor, eles[unsigned(temp)], true);
		ui->label_bar->setText("已为来自"+ QString::number(floor + 1, 10) +"层的请求选择" + QString::number(temp + 1, 10) + "号电梯.");
	}
	else {
		for(auto i : eles){
			send_request(up, floor, i, true);
			ui->label_bar->setText("已将来自"+ QString::number(floor + 1, 10) +"层的请求发送至所有电梯.");
		}
	}
}


void building::judge_stop(int ele,elevator *ele_num){
    if(eles[ele]->status == 3 || eles[ele]->status == 4){
        ele_num->cancel_request(eles[ele]->currentFloor);
    }
}


void building::timer_building_tick(){
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
       judge_stop(i,eles[i]);
    }
}


