#include "elevator.h"
#include "controller.h"
#include "ui_elevator.h"
//上升下降请求在接受后 都存放在dests中（内外） 然后进行上升下降判断
//要想STOP只能在判断时候就不存放
elevator::elevator(QWidget *parent, int _no, int _FLOOR_NUM) : QWidget(parent), ui(new Ui::elevator){//派生类继承基类函数  新对象
	ui->setupUi(this);
    no = _no;//电梯号
    FLOOR_NUM = _FLOOR_NUM; //层数
    QGroupBox *box = ui->groupBox_destination;//框子
    ctrl = nullptr;//避免内存溢出 初始化

	// resize the window and box's size to include all the buttons.
	if(FLOOR_NUM > 20){
		box ->setGeometry(60, 10, 20 + 210 * (FLOOR_NUM / 21 + 1), 200);
		this->setGeometry(0, 0, 210 * (FLOOR_NUM / 21 + 1) + 100, 380);
	}

    // Draw btns.//一排按钮
	for(int i = 0; i < FLOOR_NUM; i++){
        QPushButton *btn = new QPushButton(box);//Qpushbutton参数？
			btn->setGeometry(20+40*(i%5)+210*(i/20), 30+40*(i%20/5), 30, 30);
			btn->setText(QString::number(i+1, 10));
			btn->show();
			connect(btn, &QPushButton::clicked, this, [=] {
                if(status != 3 && status != 4 && status != 5) {
                destsInsider.push_back(i);}//当按下的时候就把内部i按钮push上去
            });
        Qbtns.push_back(btn);//将电梯内部按钮push上去
	}

	// Set title , label, slider on the window.
    this->setWindowTitle("电梯内部: " + QString::number(no+1, 10));//字符串直接相加就可以，10指间距
	ui->label_NUM_FLOOR->setText(QString::number(FLOOR_NUM, 10));
    ui->verticalSlider_currentFloor->setMaximum(FLOOR_NUM);//滚条

	// todo: ?
    // Setup the timer. Run timer_elevator_tick() every ELEVATOR_TIMER_TICK ms.//控制时间？？
	QTimer *timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, &elevator::timer_elevator_tick);
    timer->start(ELEVATOR_TIMER_TICK);//800毫秒 0.8秒控制  0.8秒更新标签  即0.8秒上升 下降

	// Setup buttons: ["开门", "关门", "报警"].
    connect(ui->pushButton_opendoor, &QPushButton::clicked, this, [=]{  //0 pause 1 up 2 down
        if(status == 0) { if(door == 0 || door == 2) open_door(); }//0closed 2closing
		else QMessageBox::about(nullptr, "Error!", "运行中无法开门.");
	});
    connect(ui->pushButton_closedoor, &QPushButton::clicked, this, [=]{//关门？
        if(door == 0 || door == 2) QMessageBox::about(nullptr, "Error!", "门已经关上了.");//QMessageBox::about(0,"标题文本","内容文本")
    });
    connect(ui->pushButton_alert, &QPushButton::clicked, this, [=]{ctrl->display_alert(no+1);});//connect 最后可以用一个匿名函数代替

}

elevator::~elevator(){
	delete ui;
}

void elevator::open_door(){
    door = 3; renew_label();//Opening: 800ms.opening 开门过程状态维持0.8秒
    QElapsedTimer t1;//计数器
    t1.start();//计时器开始
    while(t1.elapsed() < 800)//两个操作间时间流逝小于0.8S
    QCoreApplication::processEvents();//避免程序死机

    door = 1; renew_label();// Opened: 1000ms.opened 开门 开门时间1秒
	QElapsedTimer t2;
	t2.start();
	while(t2.elapsed() < 1000) QCoreApplication::processEvents();
    QCoreApplication::processEvents();//避免程序死机

    door = 2;  renew_label(); //Closing: 800ms.closing  关门0.8秒
	QElapsedTimer t3;
	t3.start();
	while(t3.elapsed() < 800) QCoreApplication::processEvents();
    QCoreApplication::processEvents();//避免程序死机

    door = 0;  renew_label(); //Closed.closed
    QCoreApplication::processEvents();//避免程序死机
}

void elevator::renew_label(){
	ui->label_status->setText(statusStr[status]);
	ui->label_door->setText(doorStr[door]);
	ui->label_current->setText(QString::number(currentFloor+1, 10));
	ui->label_current_2->setText(QString::number(currentFloor+1, 10));
	ui->verticalSlider_currentFloor->setValue(currentFloor + 1);
}

//电梯运行逻辑判断
bool elevator::recive_request(bool up, int floor, bool forceRecive){//向上 和 要down c>f 50>40
    if(!forceRecive && ( (up && status == 2 && currentFloor > floor)|| ( !up && status == 1 && currentFloor < floor )||status == 3||status == 4)) return false;
    //不强迫接受 并且 （1.要上升  状态是下降  当前层>要去的楼层数 或者 2.要下降  状态是上升  当前层<要去的楼层数）  1.继续上升  2.继续下降  这两种都不接受请求
	bool hasIn = false;
    for(auto i : destsOutside) if(i == floor) hasIn = true;//当前楼梯数=从外部按键的命令楼梯数  destsoutside 是int 类型
    if(!hasIn) destsOutside.push_back(floor);//相当于 接受了请求  *核心

    if(status == 0||status == 5||status == 3||status == 4)
        check_when_pause();
    else
        check_when_run();

	return true;
}

void elevator::cancel_request(int floor){
    auto it = std::find(destsOutside.begin(), destsOutside.end(), floor);//寻找输入楼梯数  是不是包含在数组内
    if(it != destsOutside.end()){   //find返回容器指针 不等于 说明找到了
        destsOutside.erase(it);//擦除  相当于是实现取消功能 *核心
        Qbtns[unsigned(currentFloor)]->setEnabled(true);//本层楼梯按钮可以被重新激活
    }
}


void elevator::check_when_pause(){//先判断  上行下行  在让按钮使能
	dests.insert(dests.end(), destsInsider.begin(), destsInsider.end());
    dests.insert(dests.end(), destsOutside.begin(), destsOutside.end());//两者相加 int类型动态数组
    if(dests.size() == 0) return ;//返回空 结束函数

	bool upDest   = false; // If has tasks needing upstair.
	bool downDest = false; // If has tasks needing downstair.
    for(auto i : dests){//遍历
        if(i <  currentFloor) downDest = true;//要求<当前
		if(i >  currentFloor) upDest = true;
		if(i == currentFloor) open_door();
	}

    auto it = std::find(destsInsider.begin(), destsInsider.end(), currentFloor);//当前命令执行完  就擦除储存命令的地方
    if(it != destsInsider.end()){//找到了
		destsInsider.erase(it);
		Qbtns[unsigned(currentFloor)]->setEnabled(true);
	}
	it = std::find(destsOutside.begin(), destsOutside.end(), currentFloor);
	if(it != destsOutside.end()){
		destsOutside.erase(it);
		Qbtns[unsigned(currentFloor)]->setEnabled(true);
	}

    if(currentFloor >= FLOOR_NUM / 2 && (upDest))				  status = 1;//当前层>=总层数一半 并且  要上升  结果 上升
    else if(currentFloor >= FLOOR_NUM / 2 && !upDest && downDest) status = 2;//当前成>=总层数一半 并且 不上升  并且要下降  结果下降
    else if(currentFloor <= FLOOR_NUM / 2 && (downDest))		  status = 2;//当前层
	else if(currentFloor <= FLOOR_NUM / 2 && upDest && !downDest) status = 1;
    dests.clear();//清空容器
}

void elevator::check_when_run(){
	dests.insert(dests.end(), destsInsider.begin(), destsInsider.end());
	dests.insert(dests.end(), destsOutside.begin(), destsOutside.end());
	if(dests.size() == 0){
        status = 0; //没有需求就暂停
		return;
	}
	bool upDest   = false;
	bool downDest = false;
	for(auto i : dests){
		if(i < currentFloor){downDest = true;}
		if(i > currentFloor){upDest = true;}
		if(i == currentFloor){
            auto beforeStatus = status;//保持当前状态  继续UP OR DOWN
            status = 0;
			open_door();
			status = beforeStatus;
		}
	}
    auto it = std::find(destsInsider.begin(), destsInsider.end(), currentFloor);//返回当前一个指针 指向容器的一个值
	if(it != destsInsider.end()){
		destsInsider.erase(it);
		Qbtns[unsigned(currentFloor)]->setEnabled(true);
	}
    it = std::find(destsOutside.begin(), destsOutside.end(), currentFloor);//使能按钮
	if(it != destsOutside.end()){
		destsOutside.erase(it);
		Qbtns[unsigned(currentFloor)]->setEnabled(true);
	}

    if(status == 1 && !upDest && downDest)		status = 2;//down
    else if(status == 2 && upDest && !downDest) status = 1;//up
    else if(!upDest && !downDest)				status = 0;//pause
	dests.clear();
}

void elevator::timer_elevator_tick(){
	currentFloor += status == 1 ? 1 : status == 2 ? -1 : 0;
    trueCurrentFloor = currentFloor+1;

    if(status == 0 || status == 5) check_when_pause();
    else if(status == 3 || status == 4) return ;
    else check_when_run();

	renew_label();
}

void elevator::setController(controller *_ctrl){  //初始化一个控模板
    ctrl = _ctrl;
}
