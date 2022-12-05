#include <iostream>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "role.h"
#include <QPainter>
#include <QRandomGenerator>
#include <QKeyEvent>
#include <algorithm>
#include <vector>
#include <QTimer>
#include <QApplication>
#include <QProgressBar>
#include <QLCDNumber>
#include <QString>
#include <QPushButton>
#include <QPoint>
#include <QFile>
#include <QDataStream>
#include <QTextStream>
#include <QObject>
#include <qapplication.h>
#include <QtSvg/QSvgRenderer>
using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    resize(WINDOW_WIDTH, WINDOW_HEIGHT);
    this->setFocusPolicy(Qt::StrongFocus);
    if(onePlayer || twoPlayer) initWindow();

    //initialize role
    role1 = new Role();
    role2 = new Role();
    role1->generateRoleXY();
    role2->generateRoleXY();
    while(role1->roleX == role2->roleX && role1->roleY == role2->roleY){
        role2->generateRoleXY();
    }

    //start menu
    onePlayerBox = new QPushButton(this);
    twoPlayerBox = new QPushButton(this);
    startGame = new QPushButton(this);
    exitGame = new QPushButton(this);
    startGame->setText("Start");
    startGame->setGeometry(400, 400, 200, 80);
    connect(startGame, SIGNAL(clicked()), this, SLOT(playModelChoice()));
    connect(exitGame, SIGNAL(clicked()), this, SLOT(backToMenu()));
    connect(onePlayerBox, SIGNAL(clicked()), this, SLOT(onePlayerModel()));
    connect(twoPlayerBox, SIGNAL(clicked()), this, SLOT(twoPlayerModel()));

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timeOutSlots()));
    connect(timer, SIGNAL(timeout()), this, SLOT(showTime()));

    //time screen
    timeScreen = new QLCDNumber(this);

    //time extend
    QTimer* extendTimer = new QTimer(this);
    extendTimer->start(250);
    connect(extendTimer, SIGNAL(timeout()), this, SLOT(isExtendTime()));

    //shuffle
    QTimer* shuffleTimer = new QTimer(this);
    shuffleTimer->start(250);
    connect(shuffleTimer, SIGNAL(timeout()), this, SLOT(shuffle()));

    //flash
    QTimer* flashTimer = new QTimer(this);
    flashTimer->start(250);
    connect(flashTimer, SIGNAL(timeout()), this, SLOT(isFlash()));

    //pause button
    pauseButton = new QPushButton(this);
    connect(pauseButton, SIGNAL(clicked()), this, SLOT(pauseGame()));
    //save button
    saveButton = new QPushButton(this);
    connect(saveButton, SIGNAL(clicked()), this, SLOT(saveData()));
    //load button
    loadButton = new QPushButton(this);
    connect(loadButton, SIGNAL(clicked()), this, SLOT(loadData()));
    //score screen
    score = new QLCDNumber(this);
    score2 = new QLCDNumber(this);
    //end button
    endButton = new QPushButton(this);
    connect(endButton, SIGNAL(clicked()), this, SLOT(initMenu()));

    connect(this, SIGNAL(change()), this, SLOT(update()));
    refresh();
    update();
}

MainWindow::~MainWindow()
{
    delete ui;
}

//start menu
void MainWindow::playModelChoice(){
    startGame->hide();
    onePlayerBox->show();
    twoPlayerBox->show();
    onePlayerBox->setText("One Player\n\n ⚫ W/S/A/D");
    twoPlayerBox->setText("Two Player\n\n Player1⚫: W/S/A/D \n Player2🔷: ↑/↓/←/→");
    onePlayerBox->setGeometry(400, 380, 200, 90);
    twoPlayerBox->setGeometry(400, 500, 200, 90);
}

void MainWindow::initMenu(){
    refresh();
    if(!onePlayer && !twoPlayer){
        startGame->setGeometry(zeroPoint.x()+singleWidth*(Role::COLUMN-5)/2, zeroPoint.y()+singleHeight*(Role::LINE-5)/2 + singleHeight*4, singleWidth*5, singleHeight*2);
        startGame->show();
    }
}

void MainWindow::backToMenu(){
    onePlayer = false;
    twoPlayer = false;
    pauseFlag = true;
    timer->stop();
    initMenu();
}

void MainWindow::refresh(){
    pauseButton->hide();
    saveButton->hide();
    loadButton->hide();
    timeScreen->hide();
    score->hide();
    score2->hide();
    endButton->hide();
    onePlayerBox->hide();
    twoPlayerBox->hide();
    exitGame->hide();
    isTimeExist = false;
    isShuffleExist = false;
    isFlashExist = false;
    timeFlag = false;
    shuffleFlag = false;
    flashFlag = false;

    isEndFlag = false;
    pauseFlag = 0;
    role1->clickedBox.clear();
    role2->clickedBox.clear();

    for(int i = 0; i <= MAX_MAP_SIZE; ++i){
        for(int j = 0; j <= MAX_MAP_SIZE; ++j){
            Role::map[i][j] = 0;
        }
    }
}

//if end
bool MainWindow::isEnd(){
    for(int i = 0; i < Role::COLUMN*Role::LINE; ++i){
        for(int j = i + 1; j < 420; ++j){
            int x1 = i / Role::COLUMN+1;
            int y1 = i % Role::COLUMN+1;
            int x2 = j / Role::COLUMN+1;
            int y2 = j % Role::COLUMN+1;
            if(Role::map[x1][y1] > 0 && Role::map[x1][y1] <= TYPE && Role::map[x2][y2] > 0 && Role::map[x2][y2] <= TYPE){
                if(isLink(x1, y1, x2, y2, froze, false, role1)) return false;
            }
        }
    }
    return true;
}

void MainWindow::playStatus(){
    onePlayerBox->hide();
    twoPlayerBox->hide();
    pauseButton->show();
    saveButton->show();
    loadButton->show();
    timeScreen->show();
    score->show();
    score2->show();
    role1->generateRoleXY();
    role1->scoreNum = 0;
    role2->generateRoleXY();
    role2->scoreNum = 0;
    while(role1->roleX == role2->roleX && role1->roleY == role2->roleY){
        role2->generateRoleXY();
    }
    initWindow();
}

//two player model
void MainWindow::twoPlayerModel(){
    twoPlayer = true;
    onePlayer = false;
    playStatus();
}

//one player model
void MainWindow::onePlayerModel(){
    onePlayer = true;
    twoPlayer = false;
    playStatus();
}

void MainWindow::saveData(){
    QFile file(fileName);
    QTextStream out(&file);
    out.setCodec("UTF-8");
    if(file.open(QIODevice::WriteOnly | QIODevice::Text)){
        //play model
        out << onePlayer << endl; out << twoPlayer << endl;
        //time bar value
        out << time << endl;
        //role1坐标
        out << (role1->roleX) << endl;
        out << (role1->roleY) << endl;
        out << (role2->roleX) << endl;
        out << (role2->roleY) << endl;
        //map data
        for(int i = 0; i < MAP_LINE; ++i){
            for(int j = 0; j < MAP_COLUMN; ++j){
                out << Role::map[i][j] << endl;
            }
        }
        for(int i = 1; i <= TYPE; ++i){
            out << color[i].red() << endl;
            out << color[i].green() << endl;
            out << color[i].blue() << endl;
        }
        //role color
        for(int i = 0; i < 2; ++i){
            out << roleColors[i].red() << endl;
            out << roleColors[i].green() << endl;
            out << roleColors[i].blue() << endl;
        }
        //tool
        out << timeFlag << endl; out << isTimeExist << endl; out << extendTimeX << endl; out << extendTimeY << endl;
        out << shuffleFlag << endl; out << isShuffleExist << endl; out << shuffleX << endl; out << shuffleY << endl;
        out << flashFlag << endl; out << isFlashExist << endl; out << flashX << endl; out << flashY << endl;
        //score data
        out << role1->scoreNum << endl;
        out << role2->scoreNum << endl;
        file.close();
    }
}

void MainWindow::loadData(){
    QFile file(fileName);
    QTextStream in(&file);
    in.setCodec("UTF-8");
    if(file.open(QIODevice::ReadOnly)){
        //player model
        int one, two;
        in >> one; onePlayer = one;  in >> two; twoPlayer = two;
        //time bar value
        in >> time;
        //role 坐标
        in >> role1->roleX;
        in >> role1->roleY;
        in >> role2->roleX;
        in >> role2->roleY;
        //map data
        for(int i = 0; i < MAP_LINE; ++i){
            for(int j = 0; j < MAP_COLUMN; ++j){
                in >> Role::map[i][j];
            }
        }
        for(int i = 1; i <= TYPE; ++i){
            int red, green, blue;
            in >> red;
            in >> green;
            in >> blue;
            color[i] = QColor(red, green, blue);
        }
        //role color
        for(int i = 0; i < 2; ++i){
            int red, green, blue;
            in >> red;
            in >> green;
            in >> blue;
            roleColors[i] = QColor(red, green, blue);
        }
        role1->roleColor = roleColors[0];
        role2->roleColor = roleColors[1];
        //tool
        int t, t_, s, s_, f, f_;
        in >> t; timeFlag = t; in >> t_; isTimeExist = t_; in >> extendTimeX; in >> extendTimeY;
        in >> s; shuffleFlag = s; in >> s_; isShuffleExist = s_; in >> shuffleX; in >> shuffleY;
        in >> f; flashFlag = f; in >> f_; isFlashExist = f_; in >> flashX; in >> flashY;
        //score
        in >> role1->scoreNum;
        in >> role2->scoreNum;
        update();
        file.close();
    }
}

//pause tool
void MainWindow::pauseGame(){
    pauseFlag = 1 - pauseFlag;
    if(pauseFlag){
        timer->stop();
    }else{
        timer->start();
    }
}

//倒计时
void MainWindow::timeOutSlots(){
    if(time == 0){
        timer->stop();
        endButton->show();
        isEndFlag = true;
        afterEnd();
        role1->clickedBox.clear();
        role2->clickedBox.clear();
        onePlayer = false;
        twoPlayer = false;
        return;
    }else{
        time -= 1000;
    }
    minute = QString::number((time/1000)/60);
    second = QString::number((time/1000)%60);

    if(minute.length() == 1) minute = "0" + minute;
    if(second.length() == 1) second = "0" + second;
    display = minute + ":" + second;
    if(display == "02:30" && !timeFlag && !pauseFlag){
        extendTime();
    }else if(display == "03:00" && !shuffleFlag && !pauseFlag){ //totaltime/2
        generateShuffle();
    }else if(display == "02:40" && !flashFlag && !twoPlayer && !pauseFlag){ //totaltime/4*3
        generateFlash();
    }
}

void MainWindow::showTime(){
    timeScreen->display(display);
}

//Flash tool
void MainWindow::generateFlash(){
    if(twoPlayer) return;
    if(!isFlashExist){
        int i = 0, j = 0;
        i = rand() % (Role::LINE + 2);
        j = rand() % (Role::COLUMN + 2);
        while((Role::map[i][j] > 0 && Role::map[i][j] <= TYPE + 3)){
            i = rand() % (Role::LINE + 2);
            j = rand() % (Role::COLUMN + 2);
        }

        flashX = j;
        flashY = i;

        Role::map[i][j] = TYPE + 3;
        isFlashExist = true;
        update();
    }
}

bool MainWindow::isFlash(){
    if((role1->roleX == flashX && role1->roleY == flashY) && !flashFlag && isFlashExist){
        flashFlag = true;
        Role::map[flashY][flashX] = 0;
        nextTime = time - 5000;
        curTime = time;
        update();
        return true;
    }
    return false;
}

void MainWindow::mousePressEvent(QMouseEvent *event){
    NowDragPos = ViewToWinTrans.map(event->localPos());
    bool timeLimit = true;
    if(flashFlag && timeLimit){
        curTime = time;
        if(nextTime >= curTime){
            timeLimit = false;;
        }
        if(event->button() == Qt::LeftButton && timeLimit){
            int j = NowDragPos.x() + 1;
            int i = NowDragPos.y() + 1;
            if(j > Role::COLUMN + 1 || i > Role::LINE + 1 || j < 0 || i < 0) return;
            if(Role::map[i][j] > 0 && Role::map[i][j] <= TYPE){
                int surroundPoints[4][2] = {{i, j-1}, {i-1, j}, {i, j+1}, {i+1, j}};
                for(int k = 0; k < 4; ++k){
                    if(Role::map[surroundPoints[k][0]][surroundPoints[k][1]] == 0 && isLink(role1->roleY, role1->roleX, surroundPoints[k][0], surroundPoints[k][1], true, false, role1)){
                        role1->roleX = surroundPoints[k][1];
                        role1->roleY = surroundPoints[k][0];
                        update();
                        return;
                    }
                }
            }
            if(isLink(role1->roleY, role1->roleX, i, j, true, false, role1)){
                role1->roleX = j;
                role1->roleY = i;
                update();
            }
        }
    }
}

//延时道具
void MainWindow::extendTime(){
    if(!isTimeExist){
        //随机生成道具
        int i = rand() % (Role::LINE + 2);
        int j = rand() % (Role::COLUMN + 2);
        while((Role::map[i][j] > 0 && Role::map[i][j] <= TYPE + 3)){
            i = rand() % (Role::LINE + 2);
            j = rand() % (Role::COLUMN + 2);
        }

        extendTimeX = j;
        extendTimeY = i;

        Role::map[i][j] = TYPE + 1;
        isTimeExist = true;
        update();
    }
}

void MainWindow::isExtendTime(){
    if(((role1->roleX == extendTimeX && role1->roleY == extendTimeY)||(role2->roleX == extendTimeX && role2->roleY == extendTimeY)) && !timeFlag && isTimeExist){
        //加时5s
        time += 1000*30;
        showTime();
        timeFlag = true;
        Role::map[extendTimeY][extendTimeX] = 0;
        update();
    }
}

//shuffle tool
void MainWindow::generateShuffle(){
    if(!isShuffleExist){
        //随机生成道具
        int i = rand() % (Role::LINE + 2);
        int j = rand() % (Role::COLUMN + 2);
        while((Role::map[i][j] > 0 && Role::map[i][j] <= TYPE + 3)){
            i = rand() % (Role::LINE + 2);
            j = rand() % (Role::COLUMN + 2);
        }
        shuffleX = j;
        shuffleY = i;

        Role::map[i][j] = TYPE + 2;
        isShuffleExist = true;
        update();
    }
}

bool MainWindow::isRoleTrapped(Role *role){
    if(role->roleX == 0 && role->roleY >= 0 && role->roleY <= Role::LINE + 1) return false;
    if(role->roleX == Role::COLUMN + 1 && role->roleY >= 0 && role->roleY <= Role::LINE + 1) return false;
    if(role->roleY == 0 && role->roleX >= 0 && role->roleX <= Role::COLUMN + 1) return false;
    if(role->roleY == Role::LINE + 1 && role->roleX >= 0 && role->roleX <= Role::COLUMN + 1) return false;

    int i = 0, j = 0;
    //上
    for(j = 1; j <= Role::COLUMN; ++j){
        i = 0;
        if(isLink(role->roleY, role->roleX, i, j, true, false, role)) return false;
    }
    //下
    for(j = 1; j <= Role::COLUMN; ++j){
        i = Role::LINE + 1;
        if(isLink(role->roleY, role->roleX, i, j, true, false, role)) return false;
    }
    //左
    for(i = 1; i <= Role::LINE; ++i){
        j = 0;
        if(isLink(role->roleY, role->roleX, i, j, true, false, role)) return false;
    }
    //右
    for(i = 1; i <= Role::LINE; ++i){
        j = Role::COLUMN + 1;
        if(isLink(role->roleY, role->roleX, i, j, true, false, role)) return false;
    }
    return true;
}

void MainWindow::shuffle(){
    while(((role1->roleX == shuffleX && role1->roleY == shuffleY)||(role2->roleX == shuffleX && role2->roleY == shuffleY)) && !shuffleFlag && isShuffleExist){
        Role::map[shuffleY][shuffleX] = 0;
        update();
        for(int i = 1; i <= Role::LINE; ++i){
            for(int j = 1; j <= Role::COLUMN; ++j){
                if(Role::map[i][j] > TYPE || (role1->roleX == j && role1->roleY == i) || (role2->roleX == j && role2->roleY == i)) continue;
                int x1 = 1 + rand()%Role::LINE;
                int y1 = 1 + rand()%Role::COLUMN;
                while(Role::map[x1][y1] > TYPE || (role1->roleX == y1 && role1->roleY == x1) || (role2->roleX == y1 && role2->roleY == x1)){
                    x1 = 1 + rand()%Role::LINE;
                    y1 = 1 + rand()%Role::COLUMN;
                }
                swap(Role::map[i][j], Role::map[x1][y1]);
            }
        }
        if(isRoleTrapped(role1) || isRoleTrapped(role2)) continue;

        shuffleFlag = true;
        update();
        break;
    }
}

void MainWindow::generateMap(){
    for(int i = 0; i <= MAX_MAP_SIZE; ++i){
        for(int j = 0; j <= MAX_MAP_SIZE; ++j){
            Role::map[i][j] = 0;
        }
    }

    for(int j = 1; j <= Role::COLUMN; ++j){
        for(int i = 1; i < Role::LINE; i += 2){
            int colorType = rand() % TYPE + 1;
            Role::map[i][j] = colorType;
            Role::map[i + 1][j] = colorType;
        }
    }

    //打乱box
    for(int i = 1; i <= Role::LINE; ++i){
        for(int j = 1; j <= Role::COLUMN; ++j){
            int x1 = 1 + rand()%Role::LINE;
            int y1 = 1 + rand()%Role::COLUMN;
            swap(Role::map[i][j], Role::map[x1][y1]);
        }
    }
}

void MainWindow::initWindow(){
    setWindowTitle(tr("QLink"));

    if(onePlayer || twoPlayer){
        //time clock
        time = TOTAL_TIME;
        timer->start(1000);
        minute = QString::number((time/1000)/60);
        second = QString::number((time/1000)%60);
        if(minute.length() == 1) minute = "0" + minute;
        if(second.length() == 1) second = "0" + second;
        display = minute + ":" + second;

        //generate map
        generateMap();
        randomColor();

        pauseButton->show();
        saveButton->show();
        loadButton->show();
        exitGame->show();
    }
    update();
}

void MainWindow::randomColor(){
    color = new QColor[TYPE + 1];
    for (int i = 1; i <= TYPE; ++i) {
        int green = (std::rand()) % 256;
        int red = (std::rand()) % 256;
        int blue = (std::rand()) % 256;
        color[i] = QColor(red, green, blue);
    }
    roleColors = new QColor[2];
    for(int i = 0; i < 2; ++i){
        int green = (std::rand()) % 256;
        int red = (std::rand()) % 256;
        int blue = (std::rand()) % 256;
        roleColors[i] = QColor(red, green, blue);
    }
    role1->roleColor = roleColors[0];
    role2->roleColor = roleColors[1];
}

void MainWindow::drawMap(QPainter* painter){
    drawRole(painter);
    for(int i = 0; i <= Role::LINE + 1; ++i){
        for(int j = 0; j <= Role::COLUMN + 1; ++j){
            if(Role::map[i][j] == 0) continue;
            if(Role::map[i][j] == TYPE + 1){
                drawExtendTimeBox(painter);
                continue;
            }
            if(Role::map[i][j] == TYPE + 2){
                drawShuffleBox(painter);
                continue;
            }
            if(Role::map[i][j] == TYPE + 3){
                drawFlashBox(painter);
                continue;
            }
            painter->setPen(Qt::NoPen);
            QLinearGradient boxGradient(j - 1, i - 1, j, i);
            boxGradient.setColorAt(0.0,Qt::white);
            boxGradient.setColorAt(0.8,color[Role::map[i][j]]);
            boxGradient.setColorAt(1.0,Qt::white);
            painter->setBrush(boxGradient);

            if(role1->isBoxClicked(i, j) && role2->isBoxClicked(i, j)){
                if(last == role1){
                    painter->setPen(QPen(Qt::red, 0.075, Qt::DotLine, Qt::RoundCap));
                    role1->deletePoint(i, j);
                }else{
                    painter->setPen(QPen(Qt::red, 0.075, Qt::SolidLine, Qt::RoundCap));
                    role2->deletePoint(i, j);
                }
            }else if(role1->isBoxClicked(i, j)){
                painter->setPen(QPen(Qt::red, 0.075, Qt::SolidLine, Qt::RoundCap));
            }else if(role2->isBoxClicked(i, j)){
                painter->setPen(QPen(Qt::red, 0.075, Qt::DotLine, Qt::RoundCap));
            }
            painter->drawRect(j - 1, i - 1, 1, 1);
        }
    }
}

void MainWindow::drawRole(QPainter* painter){
    if(!onePlayer && !twoPlayer && !isEndFlag) return;
    painter->setPen(Qt::NoPen);
    painter->setBrush(role1->roleColor);
    painter->drawEllipse(role1->roleX - 1, role1->roleY - 1, 1, 1);
    if(twoPlayer && !isEndFlag){
        painter->setBrush(role2->roleColor);
        QPointF points[4] = {QPointF(role2->roleX - 1, role2->roleY - 0.5), QPointF(role2->roleX - 0.5, role2->roleY - 1), QPointF(role2->roleX, role2->roleY - 0.5), QPointF(role2->roleX - 0.5, role2->roleY)};
        painter->drawPolygon(points, 4);
    }
}

void MainWindow::drawLine(QPainter* painter, Role* role){
    if(!role->cornerPoint.empty()){
        QPen pen(QColor(255, 0, 0));
        pen.setWidthF(0.08);
        painter->setPen(pen);
        painter->drawLine(role->cornerPoint[0], role->cornerPoint[1]);

        if(role->cornerPoint.size() == 3){
            painter->drawLine(role->cornerPoint[2], role->cornerPoint[0]);
        }else if(role->cornerPoint.size() == 4){
            painter->drawLine(role->cornerPoint[2], role->cornerPoint[1]);
            painter->drawLine(role->cornerPoint[0], role->cornerPoint[3]);
        }
        QTimer::singleShot(50, this, SLOT(update()));
        role->cornerPoint.clear();
    }
}

void MainWindow::drawExtendTimeBox(QPainter *painter){
    QPixmap pix = QPixmap(":/new/prefix1/image/clock-regular.png");
    painter->drawPixmap(extendTimeX - 1, extendTimeY - 1, 1, 1, pix);
}

void MainWindow::drawShuffleBox(QPainter *painter){
    QPixmap pix = QPixmap(":/new/prefix1/image/shuffle-solid.png");
    painter->drawPixmap(shuffleX - 1, shuffleY - 1, 1, 1, pix);
}

void MainWindow::drawFlashBox(QPainter *painter){
    if(flashFlag) return;
    QPixmap pix = QPixmap(":/new/prefix1/image/arrow-pointer-solid.png");
    painter->drawPixmap(flashX - 1, flashY - 1, 1, 1, pix);
}

void MainWindow::drawName(QPainter* painter){
    if(!onePlayer && !twoPlayer){
        QPixmap name = QPixmap(":/new/prefix1/image/linkName.png");
        painter->drawPixmap(Role::COLUMN/3, 0, 6, 2, name);
    }
}

void MainWindow::paintEvent(QPaintEvent*){
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true); // 抗锯齿

    //设置视口比例，防止地图变形
    int heightSide, widthSide;
    if (((double)(width())/(double)(height())) > ((double)(wid) / (double)(heigh))) {
        heightSide = height();
        widthSide = wid * heightSide / heigh;
    } else {
        widthSide = width();
        heightSide = heigh * widthSide / wid;
    }
    painter.setViewport((width()-widthSide)/2,(height()-heightSide)/2,widthSide,heightSide);

    //设置painter的坐标，方便画图
    int widSpace, heiSpace;
    heiSpace = heigh / std::min(heigh/4, wid/4);
    widSpace = wid / std::min(heigh/4, wid/4);
    painter.setWindow(-widSpace, -heiSpace, wid+2*widSpace, heigh+2*heiSpace);

    ViewToWinTrans = painter.combinedTransform().inverted();
    WinToViewTrans = painter.combinedTransform();
    zeroPoint = WinToViewTrans.map(QPoint(0,0));
    firstPoint = WinToViewTrans.map(QPoint(1, 1));
    singleWidth = firstPoint.x() - zeroPoint.x();
    singleHeight = firstPoint.y() - zeroPoint.y();
    showElem();

    //draw game name
    drawName(&painter);
    //draw box & role
    drawMap(&painter);
    drawLine(&painter, role1);
    drawLine(&painter, role2);
}

void MainWindow::setScoreScreen(){
    score->setSegmentStyle(QLCDNumber::Flat);
    score->setMode(QLCDNumber::Dec);
    score->setDigitCount(2);
    score->display(role1->scoreNum);
    score2->setSegmentStyle(QLCDNumber::Flat);
    score2->setMode(QLCDNumber::Dec);
    score2->setDigitCount(2);
    score2->display(role2->scoreNum);
}

void MainWindow::showElem(){
    if(onePlayer || twoPlayer){
        //score
        setScoreScreen();
        score->setGeometry(zeroPoint.x()/2 - 0.5*singleWidth, zeroPoint.y(), singleWidth, singleHeight);
        score2->hide();
        //time screen
        timeScreen->setSegmentStyle(QLCDNumber::Flat);
        timeScreen->setMode(QLCDNumber::Dec);//十进制
        timeScreen->setDigitCount(5);
        timeScreen->setGeometry(zeroPoint.x() + Role::LINE/2*singleWidth, zeroPoint.y()/2 - 0.5*singleHeight, singleWidth*2, singleHeight);
        timeScreen->display(display);
        timeScreen->setStyleSheet("background:black;color:#00ccff;");
        //pause button
        pauseButton->setGeometry(zeroPoint.x(), zeroPoint.y()+(Role::LINE+3)*singleHeight, singleWidth*2, singleHeight);
        pauseButton->setText("Pause");
        //save button
        saveButton->setText("Save");
        saveButton->setGeometry(zeroPoint.x()+Role::LINE/3*singleWidth, zeroPoint.y()+(Role::LINE+3)*singleHeight, singleWidth*2, singleHeight);
        //load button
        loadButton->setText("Load");
        loadButton->setGeometry(zeroPoint.x() + Role::LINE/3*2.2*singleWidth, zeroPoint.y()+(Role::LINE+3)*singleHeight, singleWidth*2, singleHeight);
        //exit button
        exitGame->setText("Exit");
        exitGame->setGeometry(zeroPoint.x() + Role::LINE/3*3.5*singleWidth, zeroPoint.y()+(Role::LINE+3)*singleHeight, singleWidth*2, singleHeight);

        if(twoPlayer){
            score2->show();
            score2->setGeometry(1.5*zeroPoint.x() + singleWidth*Role::COLUMN, zeroPoint.y(), singleWidth, singleHeight);
        }
    }
}

void MainWindow::moveUp(Role* role){
    if(role->roleY == 0){
        role->roleY = Role::LINE + 1;
    }else if(Role::map[role->roleY - 1][role->roleX] > 0 && Role::map[role->roleY - 1][role->roleX] <= TYPE && role->clickedBox.size() <= 1){
        role->clickedBox.push_back({role->roleY - 1, role->roleX});
        last = role;
    }else{
        role->roleY -= 1;
    }
}

void MainWindow::moveLeft(Role* role){
    if(role->roleX == 0){
        role->roleX = Role::COLUMN + 1;
    }else if(Role::map[role->roleY][role->roleX - 1] > 0 && Role::map[role->roleY][role->roleX - 1] <= TYPE && role->clickedBox.size() <= 1){
        role->clickedBox.push_back({role->roleY, role->roleX - 1});
        last = role;
    }else{
        role->roleX -= 1;
    }
}

void MainWindow::moveDown(Role* role){
    if(role->roleY == Role::LINE + 1){
        role->roleY = 0;
    }else if(Role::map[role->roleY + 1][role->roleX] > 0 && Role::map[role->roleY + 1][role->roleX] <= TYPE && role->clickedBox.size() <= 1){
        role->clickedBox.push_back({role->roleY + 1, role->roleX});
        last = role;
    }else{
        role->roleY += 1;
    }
}

void MainWindow::moveRight(Role* role){
    if(role->roleX == Role::COLUMN + 1){
        role->roleX = 0;
    }else if(Role::map[role->roleY][role->roleX + 1] > 0 && Role::map[role->roleY][role->roleX + 1] <= TYPE && role->clickedBox.size() <= 1){
        role->clickedBox.push_back({role->roleY, role->roleX + 1});
        last = role;
    }else{
        role->roleX += 1;
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event){
    if(pauseFlag) return;

    if(event->key() == Qt::Key_W){ //向上
        moveUp(role1);
    }else if(event->key() == Qt::Key_S){ //向下
        moveDown(role1);
    }else if(event->key() == Qt::Key_A){  //向左
        moveLeft(role1);
    }else if(event->key() == Qt::Key_D){  //向右
        moveRight(role1);
    }

    if(role1->clickedBox.size() == 2){
        if(role2->isBoxClicked(role1->clickedBox[1][0], role1->clickedBox[1][1])){
            role1->deletePoint(role1->clickedBox[1][0], role1->clickedBox[1][1]);
        }else{
            bool flag = isLink(role1->clickedBox[0][0], role1->clickedBox[0][1], role1->clickedBox[1][0], role1->clickedBox[1][1], false, false, role1);
            if(flag) {
                role1->scoreNum++;
                role1->clickedBox.clear();
                if(isEnd()) afterEnd();
            }
        }
    }

    if(event->key() == Qt::Key_Up){ //向上
        moveUp(role2);
    }else if(event->key() == Qt::Key_Down){ //向下
        moveDown(role2);
    }else if(event->key() == Qt::Key_Left){  //向左
        moveLeft(role2);
    }else if(event->key() == Qt::Key_Right){  //向右
        moveRight(role2);
    }

    if(role2->clickedBox.size() == 2){
        if(role1->isBoxClicked(role2->clickedBox[1][0], role2->clickedBox[1][1])){
            role2->deletePoint(role2->clickedBox[1][0], role2->clickedBox[1][1]);
        }else{
            bool flag = isLink(role2->clickedBox[0][0], role2->clickedBox[0][1], role2->clickedBox[1][0], role2->clickedBox[1][1], false, false, role2);
            if(flag) {
                role2->scoreNum++;
                role2->clickedBox.clear();
                if(isEnd()) afterEnd();
            }
        }
    }
    emit change();
}

void MainWindow::afterEnd(){
    pauseFlag = true;
    timer->stop();
    endButton->show();
    if(twoPlayer){
        if(role1->scoreNum > role2->scoreNum){
            endButton->setText("Game Over! Player1 Win!");
        }else if(role1->scoreNum < role2->scoreNum){
            endButton->setText("Game Over! Player2 Win!");
        }else{
            endButton->setText("Game Over! No Winner.");
        }
        endButton->setGeometry(zeroPoint.x()+singleWidth*(Role::COLUMN-5)/2, zeroPoint.y()+singleHeight*(Role::LINE-5)/2, singleWidth*5, singleHeight*5);
    }else{
        endButton->setText("Game Over! \n Your score is: " + QString::number(role1->scoreNum));
        endButton->setGeometry(zeroPoint.x()+singleWidth*(Role::COLUMN-5)/2, zeroPoint.y()+singleHeight*(Role::LINE-5)/2, singleWidth*5, singleHeight*5);
    }
    onePlayer = false; twoPlayer = false;
    isEndFlag = true;
    return;
}

bool MainWindow::isLink(const int& x1, const int& y1, const int& x2, const int& y2, bool flag, bool isTwo, Role* role){ //flag --for froze & check
    if(Role::map[x1][y1] != Role::map[x2][y2] && !flag){ //两个box对应的图案不一样
        role->clickedBox.erase(role->clickedBox.begin());
        return false;
    }
    if(x1 == x2 && y1 == y2){ //对同一个box进行了两次选定操作
        role->clickedBox.clear();
        return false;
    }

    if(linkStraight(x1, y1, x2, y2, flag, isTwo)){
        if(!flag){
            Role::map[x1][y1] = 0;
            Role::map[x2][y2] = 0;
            role->cornerPoint.push_back(QPointF(y1 - 0.5, x1 - 0.5));
            role->cornerPoint.push_back(QPointF(y2 - 0.5, x2 - 0.5));
        }
        return true;
    }
    if(linkOneCorner(x1, y1, x2, y2, flag, isTwo, role)){
        if(!flag){
            Role::map[x1][y1] = 0;
            Role::map[x2][y2] = 0;
            role->cornerPoint.push_back(QPointF(y1 - 0.5, x1 - 0.5));
            role->cornerPoint.push_back(QPointF(y2 - 0.5, x2 - 0.5));
        }
        return true;
    }
    if(linkTwoCorner(x1, y1, x2, y2, flag, isTwo, role)){
        if(!flag){
            Role::map[x1][y1] = 0;
            Role::map[x2][y2] = 0;
            role->cornerPoint.push_back(QPointF(y1 - 0.5, x1 - 0.5));
            role->cornerPoint.push_back(QPointF(y2 - 0.5, x2 - 0.5));
        }
        return true;
    }
    if(!flag) role->clickedBox.erase(role->clickedBox.begin());
    return false;
}

bool MainWindow::linkStraight(const int& x1, const int& y1, const int& x2, const int& y2, bool flag, bool isTwo){
    //竖着连
    if(y1 == y2){
        int xmin = min(x1, x2);
        int xmax = max(x1, x2);
        for(int i = xmin + 1; i < xmax; ++i){
            if(Role::map[i][y1] > 0 && Role::map[i][y1] <= TYPE) return false;
        }
        return true;
    }

    //横着连
    if(x1 == x2){
        int ymin = min(y1, y2);
        int ymax = max(y1, y2);
        for(int i = ymin + 1; i < ymax; ++i){
            if(Role::map[x1][i] > 0 && Role::map[x1][i] <= TYPE) return false;
        }
        return true;
    }
    return false;
}

bool MainWindow::linkOneCorner(const int& x1, const int& y1, const int& x2, const int& y2, bool flag, bool isTwo, Role* role){
    if(!Role::map[x1][y2]){
        if(linkStraight(x1, y1, x1, y2, flag, isTwo) && linkStraight(x2, y2, x1, y2, flag, isTwo)) {
            if(!flag) role->cornerPoint.push_back(QPointF(y2 - 0.5, x1 - 0.5));
            return true;
        }
    }
    if(!Role::map[x2][y1]){
        if(linkStraight(x1, y1, x2, y1, flag, isTwo) && linkStraight(x2, y2, x2, y1, flag, isTwo)) {
            if(!flag) role->cornerPoint.push_back(QPointF(y1 - 0.5, x2 - 0.5));
            return true;
        }
    }
    return false;
}

bool MainWindow::linkTwoCorner(const int &x1, const int &y1, const int &x2, const int &y2, bool flag, bool isTwo, Role* role){
    //向左寻找
    for(int i = y1 - 1; i >= 0; --i){
        if((Role::map[x1][i] == 0 || Role::map[x1][i] > TYPE) && linkStraight(x1, i, x1, y1, flag, isTwo) && linkOneCorner(x2, y2, x1, i, flag, isTwo, role)){
            if(!flag) role->cornerPoint.push_back(QPointF(i - 0.5, x1 - 0.5));
            return true;
        }
    }
    //向右寻找
    for(int i = y1 + 1; i <= Role::COLUMN + 1; ++i){
        if((Role::map[x1][i] == 0 || Role::map[x1][i] > TYPE) && linkStraight(x1, i, x1, y1, flag, isTwo) && linkOneCorner(x2, y2, x1, i, flag, isTwo, role)){
            if(!flag) role->cornerPoint.push_back(QPointF(i - 0.5, x1 - 0.5));
            return true;
        }
    }
    //向上寻找
    for(int i = x1 - 1; i >= 0; --i){
        if((Role::map[i][y1] == 0 || Role::map[i][y1] > TYPE) && linkStraight(i, y1, x1, y1, flag, isTwo) && linkOneCorner(x2, y2, i, y1, flag, isTwo, role)){
            if(!flag) role->cornerPoint.push_back(QPointF(y1 - 0.5, i - 0.5));
            return true;
        }
    }
    //向下寻找
    for(int i = x1 + 1; i <= Role::LINE + 1; ++i){
        if((Role::map[i][y1] == 0 || Role::map[i][y1] > TYPE) && linkStraight(i, y1, x1, y1, flag, isTwo) && linkOneCorner(x2, y2, i, y1, flag, isTwo, role)){
            if(!flag) role->cornerPoint.push_back(QPointF(y1 - 0.5, i - 0.5));
            return true;
        }
    }
    return false;
}
