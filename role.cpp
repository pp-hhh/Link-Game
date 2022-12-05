#include "role.h"
#include <iostream>
#include <vector>
#include <QFile>
#include <QTextStream>
#include <QLCDNumber>
using namespace std;

Role::Role(QWidget *parent) : QMainWindow(parent)
{
    scoreScreen = new QLCDNumber(this);
}

void Role::setScoreScreen(){
    scoreScreen->setSegmentStyle(QLCDNumber::Flat);
    scoreScreen->setMode(QLCDNumber::Dec);
    scoreScreen->setDigitCount(2);
    scoreScreen->display(scoreNum);

}

void Role::generateRoleXY(){
    srand((int)time(0));
    roleX = rand() % (COLUMN + 2);
    int yPoint[2] = {0, LINE + 1};
    if(roleX == 0 || roleX == COLUMN + 1){
        roleY = rand() % (LINE + 2);
    }else{
        roleY = yPoint[rand()%1];
    }
}

bool Role::isBoxClicked(int i, int j){
    vector<int> point = {i, j};
    if(std::find(clickedBox.begin(), clickedBox.end(), point) != clickedBox.end() && map[i][j] > 0 && map[i][j] <= TYPE) {
        return true;
    }
    return false;
}

void Role::deletePoint(int i, int j){
    vector<int> point = {i, j};
    for(auto iter = clickedBox.begin();iter!=clickedBox.end();iter++){        //从vector中删除指定的某一个元素
        if(*iter==point){
            clickedBox.erase(iter);
            break;
        }
    }
}

void Role::saveData(int timeValue){
    QFile file(fileName);
    QTextStream out(&file);
    out.setCodec("UTF-8");
    if(file.open(QIODevice::WriteOnly | QIODevice::Text)){
        //time bar value
        out << timeValue << endl;
        //role坐标
        out << (roleX) << endl;
        out << (roleY) << endl;
        //map data
        for(int i = 0; i < MAX_MAP_SIZE + 1; ++i){
            for(int j = 0; j < MAX_MAP_SIZE + 1; ++j){
                out << Role::map[i][j] << endl;
            }
        }
        out << scoreNum << endl;
        file.close();
    }
}

void Role::loadData(QProgressBar* timeBar){
    QFile file(fileName);
    QTextStream in(&file);
    in.setCodec("UTF-8");
    if(file.open(QIODevice::ReadOnly)){
        int timeValue = 0;
        //time bar value
        in >> timeValue;
        timeBar->setValue(timeValue);
        //role 坐标
        in >> roleX;
        in >> roleY;
        //map data
        for(int i = 0; i < MAX_MAP_SIZE + 1; ++i){
            for(int j = 0; j < MAX_MAP_SIZE + 1; ++j){
                in >> Role::map[i][j];
            }
        }
        in >> scoreNum;
        //update();
        file.close();
    }
}

int Role::map[MAX_MAP_SIZE + 1][MAX_MAP_SIZE + 1];
