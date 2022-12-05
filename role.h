#ifndef ROLE_H
#define ROLE_H

#include <QMainWindow>
#include <iostream>
#include <QPointF>
#include <QProgressBar>
#include <QLCDNumber>
using namespace std;

#define MAX_MAP_SIZE 16

class Role : public QMainWindow
{
    Q_OBJECT
public:
    explicit Role(QWidget *parent = nullptr);

    //role x/y points relevant
    int roleX, roleY;
    void generateRoleXY();

    static const int LINE = 12, COLUMN = 15, TYPE = 15;
    static int map[MAX_MAP_SIZE + 1][MAX_MAP_SIZE + 1];
    QColor roleColor;

    //if box clicked
    bool isBoxClicked(int i, int j);
    void deletePoint(int i, int j);

    //link
    vector<vector<int>> clickedBox; //记录被选中的box
    vector<QPointF> cornerPoint; //记录link需要的相关点

    //score
    int scoreNum = 0;
    QLCDNumber* scoreScreen;
    void setScoreScreen();

    //save & load
    QString fileName = "D:/Lab/SEP_lab/Qt/QLink/data1.txt";
    QString fileName2 = "D:/Lab/SEP_lab/Qt/QLink/data2.txt";
    void saveData(int timeValue);
    void loadData(QProgressBar* timeBar);

signals:

};

#endif // ROLE_H
