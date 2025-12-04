#pragma once
#include <QObject>
#include <QTimer>
#include <QFile>
#include <QTextStream>

class speed
{
    Q_OBJECT
    Q_PROPERTY(int value READ value WRITE setvalue NOTIFY updated)

private:
    int speedValue;

public:
    explicit speed(QObject *parent = nullptr);

    int getValue() const { return speedValue; }
    void setValue(int v);

};
