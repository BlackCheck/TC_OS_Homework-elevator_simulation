#include "building.h"
#include "ui_building.h"

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
        QLabel *eleNo = new QLabel(ui->groupBox_eles);
			eleNo->setGeometry(20 + 40 * i, 30, 20, 20);
            eleNo->setAlignment(Qt::AlignHCenter);
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

bool building::send_request(bool up, int floor, elevator *ele){
    if(ele->flag != 0 && ele->no == 0)
        return false;
    else
        return ele->recive_request(up, floor);
}

int building::ele_rate(bool reqUp, int reqFloor, int eleFloor, int eleStatus){
    if(reqFloor == eleFloor) return 10000;
    double distanceRating = double(abs(eleFloor - reqFloor)) / double(20);
    if(eleStatus == 0) distanceRating *= 3;
	double statusRating = eleStatus == 0 ? 1.0 : reqUp ? eleStatus == 1 ? eleFloor < reqFloor ? 1.0 : 0.2
																		: eleFloor < reqFloor ? 0.6 : 0.4
													   : eleStatus == 2 ? eleFloor > reqFloor ? 1.0 : 0.2
																		: eleFloor > reqFloor ? 0.6 : 0.4;

    if(eleStatus == 3||eleStatus == 4){distanceRating = statusRating = 0;}
    return int(100.0 * (distanceRating * 0.6 + statusRating * 0.4));
}

void building::ele_select_send(bool up, int floor){
    eleRatings.clear();
    for(int i = 0; i < 6;i++){
        eleRatings.push_back({i, ele_rate(up, floor, eles[unsigned(i)]->currentFloor, eles[unsigned(i)]->status)});
        std::sort(eleRatings.begin(), eleRatings.end(),[](std::pair<int, int> &a, std::pair<int, int> &b){
           return a.second < b.second;});
    }    
    if(eles[0]->flag == 0){
        for(int i = 0 ;i < 6 ;i++){
          if(send_request(up,floor,eles[unsigned(eleRatings[i].first)]))
              return ;
          else
              continue;
        }
    }
    else if(eles[0]->flag != 0 )
        for(int i = 1 ;i < 6 ;i++){
          if(send_request(up,floor,eles[unsigned(eleRatings[i].first)]))
              return ;
          else
              continue;
        }
    }

void building::timer_building_tick(){
    for(int i = 0; i < 6; i++){
        renew_label(i);
        if(eles[i]->status == 0 || eles[i]->status == 3){
            for(auto j : eles)  j->cancel_request(eles[i]->currentFloor);
            floorBtnsUp[unsigned(eles[i]->currentFloor)]->setEnabled(true);
            floorBtnsDown[unsigned(eles[i]->currentFloor)]->setEnabled(true);
        }
      }

    if(eles[0]->status == 5 || eles[1]->status == 5 || eles[0]->flag_1 == 1 || eles[1]->flag_1 == 1){
        for(int i = 0; i < 56 ;i++){
            floorBtnsUp[i]->setEnabled(true);
            floorBtnsDown[i]->setEnabled(true);
           }
        for(int i = 0; i < 6 ;i++){
            eles[i]->flag_1 = 0;
           }
    }
}


