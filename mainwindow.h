#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "role.h"
#include <QColor>
#include <QEvent>
#include <vector>
#include <QTimer>
#include <QProgressBar>
#include <QLCDNumber>
#include <QPushButton>
#include <QMouseEvent>
#include <QPointF>
using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow;}
QT_END_NAMESPACE

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 840
#define PLAY_AREA_WIDTH 600
#define PLAY_AREA_HEIGHT 440
#define LENGTH 40
#define MAP_LINE 21
#define MAP_COLUMN 25
#define TOTAL_TIME 4*60*1000
#define TYPE 15
#define MAX_MAP_SIZE 16

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    Role* role1, *role2;

private:
    Ui::MainWindow *ui;

protected:
    void paintEvent(QPaintEvent *event) override;
    void randomColor();
    void generateMap();
    QColor *color;
    QColor *roleColors;
    void drawMap(QPainter* painter);
    void drawLine(QPainter* painter, Role* role);
    int heigh = 13, wid = 16;
    QPointF zeroPoint;
    QPointF firstPoint;
    float singleWidth = 0, singleHeight = 0;

    void showElem();
    void drawRole(QPainter* painter);
    Role* last = role1;

    void keyPressEvent(QKeyEvent *event) override;
    QPaintEvent* event;
    void moveUp(Role* role);
    void moveDown(Role* role);
    void moveLeft(Role* role);
    void moveRight(Role* role);
public:
    bool isLink(const int& x1, const int& y1, const int& x2, const int& y2, bool flag, bool isTwo, Role* role);
protected:
    bool linkStraight(const int& x1, const int& y1, const int& x2, const int& y2, bool flag, bool isTwo);
    bool linkOneCorner(const int& x1, const int& y1, const int& x2, const int& y2, bool flag, bool isTwo, Role* role);
    bool linkTwoCorner(const int& x1, const int& y1, const int& x2, const int& y2, bool flag, bool isTwo, Role* role);

    //frozeMode
    bool isFroze();
    bool froze = true;
    //draw line
    vector<vector<int>> cornerPoint2;

    //time clock
    int time = 0;
    QTimer *timer;
    int progress;
    QString minute;
    QString second;
    QString display;
    QLCDNumber* timeScreen;

    //+30s tool
    bool timeFlag = false, isTimeExist = false;
    int extendTimeX;
    int extendTimeY;
    void extendTime();
    void drawExtendTimeBox(QPainter* painter);

    //shuffle tool
    void generateShuffle();
    int shuffleX;
    int shuffleY;
    bool shuffleFlag = false, isShuffleExist = false;;
    void drawShuffleBox(QPainter* painter);
    bool isRoleTrapped(Role* role);

    //flash tool
    void generateFlash();
    int flashX;
    int flashY;
    bool flashFlag = false, isFlashExist = false;;
    void mousePressEvent(QMouseEvent*event) override;
    int curTime, nextTime;
    void drawFlashBox(QPainter* painter);
    QPointF NowDragPos;
    QTransform ViewToWinTrans;
    QTransform WinToViewTrans;

    //pause tool
    QPushButton* pauseButton;
    int pauseFlag = 0;

    //save tool
    QPushButton* saveButton;
    QString fileName = "D:/Lab/SEP_lab/Qt/QLink/data1.txt";

    //load tool
    QPushButton* loadButton;

    //two player model
    QPushButton* twoPlayerButton;
    bool twoPlayer = false;

    //start menu
    QPushButton* onePlayerBox;
    bool onePlayer = false;
    QPushButton* twoPlayerBox;
    QPushButton* startGame;
    QPushButton* exitGame;
    void playStatus();
    void drawName(QPainter* painter);

    //show score
    QLCDNumber* score;
    QLCDNumber* score2;
    void setScoreScreen();

    //is end
    bool isEnd();
    QPushButton* endButton;
    void refresh();
    bool isEndFlag = false;
    void afterEnd();


signals:
    void change();
    void extendTimeSignal(); //+1s tool
private slots:
    //timescreen
    void timeOutSlots();
    void showTime();

    void isExtendTime();   //+1s tool

    void shuffle(); //shuffle tool

    bool isFlash(); //flash tool

    void pauseGame(); //pause tool

    void saveData(); //save tool

    void loadData(); //load tool

    void twoPlayerModel(); //two player model

    void onePlayerModel(); //one player model

    void initWindow();

    void initMenu();
    void playModelChoice();
    void backToMenu();
};
#endif // MAINWINDOW_H
