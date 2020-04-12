#ifndef ELEVATOR_H
#define ELEVATOR_H

//#include "controller.h"
#include <vector>
#include <algorithm>
#include <QTimer>
#include <QWidget>
#include <QGroupBox>
#include <QMessageBox>
#include <QPushButton>
#include <QElapsedTimer>

// This class is the panel inside the elevators

namespace Ui { class elevator; }

//extern
class controller;

class elevator : public QWidget{
	Q_OBJECT
	public:
        explicit elevator(QWidget *parent = nullptr, int _no = 0, int _FLOOR_NUM = 20);//构造函数
		~elevator();
	private:
		Ui::elevator *ui;
	public:
		int no = 0; // This elevator's id.
        int door = 0; // The status of door ∈ [0: closed, 1: opened, 2: closing, 3:opening].
        int status = 0; //  The status of this elevator ∈ [0: pause暂停,1: up, 2: down,3: stop停用,4: broken,5:reset].
        int currentFloor = 0;//floor
        int trueCurrentFloor = 0; // true floor number
        int FLOOR_NUM = 20;//无用

		const int ELEVATOR_TIMER_TICK = 800; // Frequency, unit: ms;

        QString statusStr[5] = {"P", "↑", "↓","Stop","Broken"};
		QString doorStr[4]   = {"Closed", "Opened", "Closing", "Opening"};

		std::vector<int> dests; // This elevator's destations <- destsInsider + destsOutside.
		std::vector<int> destsInsider; // This elevator's destations from inside.
		std::vector<int> destsOutside; // This elevator's destations from outside. Add by "recive_request()".

		std::vector<QPushButton*> Qbtns; // To store buttons on the window.

		controller *ctrl;
	public:
		void open_door();
		void renew_label();
		void check_when_run();
		void check_when_pause();
		void timer_elevator_tick(); // Run every ELEVATOR_TIMER_TICK ms.
		void cancel_request(int floor);
		// Recive task request form outside(building), and add it to destO. .
		// See "send_request()" in class "building".
		bool recive_request(bool up = true, int floor = 1, bool forceRecive = false);

		// set controller
        void setController(controller *_ctrl);
        bool reset();

//	signals:
//        void send_ele_no(int);


};

#endif // ELEVATOR_H
