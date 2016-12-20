#include <Arduino.h>
#include "Driver.h"

Driver::Driver(int AIA, int AIB, int BIA, int BIB)
{
  this->AIA = AIA;
  this->AIB = AIB;
  this->BIA = BIA;
  this->BIB = BIB;
  this->speed = 255;
}

Driver::~Driver(){}

void Driver::init()
{
  pinMode(AIA, OUTPUT);;
  pinMode(AIB, OUTPUT);
  pinMode(BIA, OUTPUT);
  pinMode(BIB, OUTPUT);
}

void Driver::drive(int dir, int speed)
{
  /* key direction
   *  
   *          3
   *      1       2
   *         4
   */

  if (dir == 1) turnleft(speed);
  else if (dir == 2) turnright(speed);
  else if (dir == 3) forward(speed);
  else if (dir == 4) backward(speed);
  else stop();
}

int Driver::reverse(int dir)
{
  if (dir == 1 || dir == 2) dir = 3 - dir;
  else if (dir == 3 || dir == 4) dir = 7 - dir;

  return dir;
}

int Driver::getdir(char c)
{
  switch (c)
  {
    case 'l': return 1; break;
    case 'r': return 2; break;
    case 'f': return 3; break;
    case 'b': return 4; break;
    default: return -1;
  }
}

void Driver::stop()
{
  analogWrite(AIA, 0);
  analogWrite(AIB, 0);
  analogWrite(BIA, 0);
  analogWrite(BIB, 0);
}

void Driver::backward(int speed)
{
  analogWrite(AIA, 0);
  analogWrite(AIB, speed);
  analogWrite(BIA, 0);
  analogWrite(BIB, speed);
}

void Driver::forward(int speed)
{
  analogWrite(AIA, speed);
  analogWrite(AIB, 0);
  analogWrite(BIA, speed);
  analogWrite(BIB, 0);
}

void Driver::turnright(int speed)
{
  analogWrite(AIA, speed);
  analogWrite(AIB, 0);
  analogWrite(BIA, 0);
  analogWrite(BIB, speed);
}

void Driver::turnleft(int speed)
{
  analogWrite(AIA, 0);
  analogWrite(AIB, speed);
  analogWrite(BIA, speed);
  analogWrite(BIB, 0);
}
