#ifndef __DRIVER_H__
#define __DIRVER_H__

class Driver {
  private:
    int AIA;
    int AIB;
    int BIA;
    int BIB;
    bool un;
  public:
    Driver(int AIA, int AIB, int BIA, int BIB);
    ~Driver();
    void init();
    void drive(int dir);
    int reverse(int dir);
    int getdir(char c);
    void stop();
    void backward();
    void forward();
    void turnright();
    void turnleft();
    bool active();
    void setActive(bool un);
};

#endif
