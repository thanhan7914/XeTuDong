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
  pinMode(AIA, OUTPUT);;
  pinMode(AIB, OUTPUT);
  pinMode(BIA, OUTPUT);
  pinMode(BIB, OUTPUT);
}

void Driver::drive(int dir)
{
  /* key direction

              3
          1       2
             4
  */

  if (dir == 1) turnleft();
  else if (dir == 2) turnright();
  else if (dir == 3) forward();
  else if (dir == 4) backward();
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
  un = false;
  digitalWrite(AIA, LOW);
  digitalWrite(AIB, LOW);
  digitalWrite(BIA, LOW);
  digitalWrite(BIB, LOW);
}

void Driver::backward()
{
  un = true;
  digitalWrite(AIA, LOW);
  digitalWrite(AIB, HIGH);
  digitalWrite(BIA, LOW);
  digitalWrite(BIB, HIGH);
}

void Driver::forward()
{
  un = true;
  digitalWrite(AIA, HIGH);
  digitalWrite(AIB, LOW);
  digitalWrite(BIA, HIGH);
  digitalWrite(BIB, LOW);
}

void Driver::turnright()
{
  un = true;
  digitalWrite(AIA, HIGH);
  digitalWrite(AIB, LOW);
  digitalWrite(BIA, LOW);
  digitalWrite(BIB, HIGH);
}

void Driver::turnleft()
{
  un = true;
  digitalWrite(AIA, LOW);
  digitalWrite(AIB, HIGH);
  digitalWrite(BIA, HIGH);
  digitalWrite(BIB, LOW);
}

void Driver::setActive(bool un)
{
  this->un = un;
}

bool Driver::active()
{
  return un;
}

