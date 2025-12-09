#pragma once
#include <QObject>

class Speed : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY updated)

public:
    explicit Speed(QObject *parent = nullptr);

    int value() const { return speedValue; }
    void setValue(int v);

signals:
    void updated();

private:
    int speedValue = 0;
};
