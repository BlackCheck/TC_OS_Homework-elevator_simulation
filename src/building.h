#ifndef BUILDING_H
#define BUILDING_H

#include "elevator.h"
#include <cmath>
#include <time.h>
#include <stdlib.h>
#include <QLabel>
#include <QString>
#include <QSlider>
#include <QStatusBar>

// This class is buttons on the outside of the elevator on each floor

namespace Ui { class building; }

class building : public QWidget{
	Q_OBJECT
	public:
		explicit building( QWidget *parent = nullptr,
						   std::vector<elevator*> _eles = {},
                           int _FLOOR_NUM = 20, int _ELE_SELECT_MODE = 1
						 );
		~building();
	private:
		Ui::building *ui;
	public:
        int ELE_NUM = 6;
        int FLOOR_NUM = 56;
		int ELE_SELECT_MODE = 1;
		std::vector<elevator*> eles;
		std::vector<QSlider*>  eleSliders;
		std::vector<QLabel*>   eleCurrents;
		std::vector<QPushButton*> floorBtnsUp;
		std::vector<QPushButton*> floorBtnsDown;
		// Store every elevator's rating, for elevator selecting.
		// Only be used in the ELE_SELECT_MODE_1.
        std::vector<std::pair<int, int>> eleRatings;//分别保存电梯的楼层数  与权重
	public:
		void timer_building_tick(); // Run every 0.1s.
		void renew_label(unsigned int i);
		// Main func to select elevator(s) and send task request(s) after button clicked.
		void ele_select_send(bool up = true, int floor = 1);
		// Caculate every elevator's rating, for elevator selecting.
		// Only be used in the ELE_SELECT_MODE_1.
		int  ele_rate(bool reqUp, int reqFloor, int eleFloor, int eleStatus);
		// See "recive_request" in class "elevator".
        bool send_request(bool up = true, int floor = 1, elevator *ele = nullptr);
        void judge_stop(int ele,elevator *ele_num);
};

#endif // BUILDING_H
