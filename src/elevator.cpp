#include "elevator.h"
#include "controller.h"
#include "ui_elevator.h"

elevator::elevator(QWidget *parent, int _no, int _FLOOR_NUM) : QWidget(parent), ui(new Ui::elevator){
	ui->setupUi(this);
    no = _no;
    FLOOR_NUM = _FLOOR_NUM;
    QGroupBox *box = ui->groupBox_destination;
    ctrl = nullptr;
    flag = 0;
    flag_1 = 0;

	// resize the window and box's size to include all the buttons.
	if(FLOOR_NUM > 20){
		box ->setGeometry(60, 10, 20 + 210 * (FLOOR_NUM / 21 + 1), 200);
		this->setGeometry(0, 0, 210 * (FLOOR_NUM / 21 + 1) + 100, 380);
	}

    // Draw btns.
	for(int i = 0; i < FLOOR_NUM; i++){
        QPushButton *btn = new QPushButton(box);
			btn->setGeometry(20+40*(i%5)+210*(i/20), 30+40*(i%20/5), 30, 30);
			btn->setText(QString::number(i+1, 10));
			btn->show();
			connect(btn, &QPushButton::clicked, this, [=] {
                if(status != 3 && status != 4 && status != 5) {
                   Qbtns[unsigned(i)]->setEnabled(false);
                   destsInsider.push_back(i);}
            });
        Qbtns.push_back(btn);
	}

	// Set title , label, slider on the window.
    this->setWindowTitle("电梯内部: " + QString::number(no+1, 10));
	ui->label_NUM_FLOOR->setText(QString::number(FLOOR_NUM, 10));
    ui->verticalSlider_currentFloor->setMaximum(FLOOR_NUM);

    // Setup the timer. Run timer_elevator_tick() every ELEVATOR_TIMER_TICK ms.
	QTimer *timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, &elevator::timer_elevator_tick);
    timer->start(ELEVATOR_TIMER_TICK);

	// Setup buttons: ["开门", "关门", "报警"].
    connect(ui->pushButton_opendoor, &QPushButton::clicked, this, [=]{
        if(status == 0) { if(door == 0 || door == 2) open_door(); }
        else if(status == 4) door = 1;
		else QMessageBox::about(nullptr, "Error!", "运行中无法开门.");
	});
    connect(ui->pushButton_closedoor, &QPushButton::clicked, this, [=]{
        if(door == 0 || door == 2) {QMessageBox::about(nullptr, "Error!", "门已经关上了.");close_door();}
        else close_door();
    });
    connect(ui->pushButton_alert, &QPushButton::clicked, this, [=]{ctrl->display_alert(no);});
}

elevator::~elevator(){
	delete ui;
}

void elevator::open_door(){

    door = 1; renew_label();// Opened: 9000ms.
	QElapsedTimer t2;
	t2.start();
    while(t2.elapsed() < 9000)
        QCoreApplication::processEvents();

    door = 0;  renew_label(); //Closed.
    QElapsedTimer t1;
    t1.start();
    while(t1.elapsed() < 2000)
     QCoreApplication::processEvents();
}

void elevator::close_door(){

    door = 0;  renew_label(); //Closed
    QElapsedTimer t2;
    t2.start();
    while(t2.elapsed() < 9000) QCoreApplication::processEvents();
}

void elevator::renew_label(){
	ui->label_status->setText(statusStr[status]);
	ui->label_door->setText(doorStr[door]);
	ui->label_current->setText(QString::number(currentFloor+1, 10));
	ui->label_current_2->setText(QString::number(currentFloor+1, 10));
	ui->verticalSlider_currentFloor->setValue(currentFloor + 1);
}

bool elevator::recive_request(bool up, int floor){

    if( (up && status == 2 && currentFloor > floor)|| ( !up && status == 1 && currentFloor < floor )||status == 3||status == 4||status == 5) return false;
    else if(floor == currentFloor) open_door();
    bool hasIn = false;
    for(auto i : destsOutside) if(i == floor) hasIn = true;
    if(!hasIn) destsOutside.push_back(floor);

    if(status == 0||status == 5||status == 3)
        check_when_pause();
     else
        check_when_run();

	return true;
}

void elevator::cancel_request(int floor){
    auto it = std::find(destsOutside.begin(), destsOutside.end(), floor);
    if(it != destsOutside.end()){
        destsOutside.erase(it);
        Qbtns[unsigned(currentFloor)]->setEnabled(true);
    }
}

void elevator::check_when_pause(){
	dests.insert(dests.end(), destsInsider.begin(), destsInsider.end());
    dests.insert(dests.end(), destsOutside.begin(), destsOutside.end());
    if(dests.size() == 0) return ;

	bool upDest   = false; // If has tasks needing upstair.
	bool downDest = false; // If has tasks needing downstair.
    for(auto i : dests){
        if(i <  currentFloor) downDest = true;
        if(i >  currentFloor) upDest = true;
        if(i == currentFloor) open_door();
	}

    auto it = std::find(destsInsider.begin(), destsInsider.end(), currentFloor);
    if(it != destsInsider.end()){
		destsInsider.erase(it);
		Qbtns[unsigned(currentFloor)]->setEnabled(true);
	}
	it = std::find(destsOutside.begin(), destsOutside.end(), currentFloor);
	if(it != destsOutside.end()){
		destsOutside.erase(it);
		Qbtns[unsigned(currentFloor)]->setEnabled(true);
	}

    if(currentFloor >= FLOOR_NUM / 2 && (upDest))				  status = 1;
    else if(currentFloor >= FLOOR_NUM / 2 && !upDest && downDest) status = 2;
    else if(currentFloor <= FLOOR_NUM / 2 && (downDest))		  status = 2;
	else if(currentFloor <= FLOOR_NUM / 2 && upDest && !downDest) status = 1;
    dests.clear();
}

void elevator::check_when_run(){
	dests.insert(dests.end(), destsInsider.begin(), destsInsider.end());
	dests.insert(dests.end(), destsOutside.begin(), destsOutside.end());
	if(dests.size() == 0){
        status = 0;
		return;
	}
	bool upDest   = false;
	bool downDest = false;
	for(auto i : dests){
		if(i < currentFloor){downDest = true;}
		if(i > currentFloor){upDest = true;}
		if(i == currentFloor){
            status = 0;
            open_door();
		}
	}
    auto it = std::find(destsInsider.begin(), destsInsider.end(), currentFloor);
	if(it != destsInsider.end()){
		destsInsider.erase(it);
		Qbtns[unsigned(currentFloor)]->setEnabled(true);
	}

    it = std::find(destsOutside.begin(), destsOutside.end(), currentFloor);
	if(it != destsOutside.end()){
		destsOutside.erase(it);
		Qbtns[unsigned(currentFloor)]->setEnabled(true);
	}

    if(status == 1 && !upDest && downDest )		   status = 2;
    else if(status == 2 && upDest && !downDest)    status = 1;
    else if(!upDest && !downDest && status != 4)   status = 0;
    else if(status == 4 )                          status = 4;
	dests.clear();
}

void elevator::timer_elevator_tick(){

    if(status == 1)
        currentFloor += 1;
    else if(status == 2)
        currentFloor -= 1;
    else if(status == 0)
        currentFloor += 0;
    else if(status == 4 && currentFloor != 0)
        currentFloor -= 1;

    if(status == 4)
        for(int i = 0;i < 56;i++)
            Qbtns[i]->setEnabled(true);

    if(status == 4 && currentFloor == 0)
        door = 1;

    renew_label();

    trueCurrentFloor = currentFloor+1;

    if(status == 0 || status == 5) check_when_pause();
    else if(status == 3||status == 4) return ;
    else check_when_run();

}

void elevator::setController(controller *_ctrl){
    ctrl = _ctrl;
}
