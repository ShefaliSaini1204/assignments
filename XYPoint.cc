#include "XYPoint.h"


XYPoint::XYPoint() : x(0.0), y(0.0) {}

XYPoint::XYPoint(double x_val, double y_val) : x(x_val), y(y_val) {}

double XYPoint::getX() const {
    return x;
}

double XYPoint::getY() const {
    return y;
}

void XYPoint::setX(double x_val) {
    x = x_val;
}

void XYPoint::setY(double y_val) {
    y = y_val;
}
