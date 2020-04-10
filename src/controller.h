#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "elevator.h"
#include <cmath>
#include <time.h>
#include <stdlib.h>
#include <QLabel>
#include <QString>
#include <QSlider>
#include <QStatusBar>

// This class is buttons on the outside of the elevator on each floor

namespace Ui { class controller; }//Ui controller 是一个类指针

class controller : public QWidget{
	Q_OBJECT
	public:
		explicit controller( QWidget *parent = nullptr,
						   std::vector<elevator*> _eles = {},
						   int _FLOOR_NUM = 20, int _ELE_SELECT_MODE = 1
						 );
		~controller();
	private:
		Ui::controller *ui;
	public:
		int ELE_NUM = 5;
        int FLOOR_NUM = 20;//每个方块显示得数目
		int ELE_SELECT_MODE = 1;
		std::vector<elevator*> eles;
		std::vector<QSlider*>  eleSliders;
		std::vector<QLabel*>   eleCurrents;
		std::vector<QPushButton*> floorBtnsUp;
		std::vector<QPushButton*> floorBtnsDown;
        std::vector<QPushButton*> StopBtns;
		// Store every elevator's rating, for elevator selecting.
		// Only be used in the ELE_SELECT_MODE_1.
		std::vector<std::pair<int, int>> eleRatings;
	public:
		void timer_building_tick(); // Run every 0.1s.
        void renew_label();
		// Main func to select elevator(s) and send task request(s) after button clicked.
		void ele_select_send(bool up = true, int floor = 1);
		// Caculate every elevator's rating, for elevator selecting.
		// Only be used in the ELE_SELECT_MODE_1.
		int  ele_rate(bool reqUp, int reqFloor, int eleFloor, int eleStatus);
		// See "recive_request" in class "elevator".
		bool send_request(bool up = true, int floor = 1, elevator *ele = nullptr, bool forceRecive = false);

		bool receive_request(int ele_no);
		
		void send_stop_request(elevator * ele);
		void recive_stop_request(elevator * ele);
		void reset_ele(int ele);


//    public slots:
        void display_alert(int ele_no);

};

#endif // BUILDING_H
