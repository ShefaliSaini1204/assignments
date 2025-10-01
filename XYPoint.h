#ifndef XYPOINT_H
#define XYPOINT_H

class XYPoint {

public:
    XYPoint();                         
    XYPoint(double x, double y);       

    double getX() const;               
    double getY() const;               

    void setX(double x);               
    void setY(double y);      

private:
    double x;
    double y;	
};

#endif
