#ifndef LINKTEST_H
#define LINKTEST_H

#include <QObject>
#include <QtTest/QtTest>

class linkTest : public QObject
{
    Q_OBJECT
public:
    explicit linkTest(QObject *parent = nullptr);

signals:

};

#endif // LINKTEST_H
