#include <Arduino.h>
#include "Driver.h"

Driver::Driver(int AIA, int AIB, int BIA, int BIB)
{
  this->AIA = AIA;
  this->AIB = AIB;
  this->BIA = BIA;
  this->BIB = BIB;
  this->un  = false;
}

Driver::~Driver() {}

void Driver::init()
{
  pinMode(AIA, OUTPUT);
  pinMode(AIB, OUTPUT);
  pinMode(BIA, OUTPUT);
  pinMode(BIB, OUTPUT);
}

void Driver::drive(int dir)
{
  /* key direction
  *
  *          3
  *      1       2
  *          4
  */

  if (dir == 1) turnleft();
  else if (dir == 2) turnright();
  else if (dir == 3) forward();
  else if (dir == 4) backward();
  else pause();
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

void Driver::pause()
{
  un = false;
  analogWrite(AIA, 0);
  analogWrite(AIB, 0);
  analogWrite(BIA, 0);
  analogWrite(BIB, 0);
}

void Driver::backward()
{
  un = true;
  analogWrite(AIA, 0);
  analogWrite(AIB, 255);
  analogWrite(BIA, 0);
  analogWrite(BIB, 255);
}

void Driver::forward()
{
  un = true;
  analogWrite(AIA, 255);
  analogWrite(AIB, 0);
  analogWrite(BIA, 255);
  analogWrite(BIB, 0);
}

void Driver::turnright()
{
  un = true;
  analogWrite(AIA, 255);
  analogWrite(AIB, 0);
  analogWrite(BIA, 0);
  analogWrite(BIB, 255);
}

void Driver::turnleft()
{
  un = true;
  analogWrite(AIA, 0);
  analogWrite(AIB, 255);
  analogWrite(BIA, 255);
  analogWrite(BIB, 0);
}

void Driver::setActive(bool un)
{
  this->un = un;
}

bool Driver::active()
{
  return un;
}

