#include "speed.hpp"

speed::speed(QObject *parent)
    :speedValue(0)
{
}

int speed::getValue() const
{
    return speedValue;
}

void speed::setValue(int v)
{
    if (speedValue != v) {
        speedValue = v;
        emit updated();
    }
}
