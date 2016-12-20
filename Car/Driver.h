#ifndef __DRIVER_H__
#define __DIRVER_H__

class Driver {
  private:
    int AIA;
    int AIB;
    int BIA;
    int BIB;
    int speed;
  public:
    Driver(int AIA, int AIB, int BIA, int BIB);
    ~Driver();
    void init();
    void drive(int dir, int speed);
    int reverse(int dir);
    int getdir(char c);
    void stop();
    void backward(int speed);
    void forward(int speed);
    void turnright(int speed);
    void turnleft(int speed);
};

#endif
