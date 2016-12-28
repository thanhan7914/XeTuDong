#include "RFClient.h"
#include "Timer.h"

#define AIA 6
#define AIB 9
#define BIA 5
#define BIB 3
#define BUZZER 4

//radio
#define RFNTRY 4
//STATE
#define EMPTY 0
#define FOLLOWER 1
#define COLLISION 2
#define ACROSS 3
#define PAUSE 20
#define WAITNRF 11

//mode
#define GWATER 1
#define GETDONE 2
#define GOTOWATER 3
#define GOTOTARGET 4
#define GTDONE 5
#define TRDONE 6
#define ROLLBACK 7
#define DONETRANS 8
#define WAIT 9

//dinh nghi phan hoi
#define REFUSE 2
#define ACCEPT 1

//so luong vi tri
#define NODE 4

const int IFR_LEFT = A3;
const int IFR_MIDDLE = A4;
const int IFR_RIGHT = A5;
uint8_t spd = 180;

RFClient radio(8, 10);

//data send or receive
//recieve 2 bytes -> room.water
//send 1 byte -> state
uint8_t rx_data[2];
uint8_t tx_data = 0;

/*
    state
    EMPTY: nhan roi
    FOLLOER: do duong
    ...
*/

uint8_t state = EMPTY;

/*
   mode
   EMPTY 0
   GWATER 1
*/

uint8_t mode = EMPTY;

/*
    map
    a - > c - > d -> b -> a
    0     1     2    3    0
*/
int     dir = -1;
uint8_t pos = 1; // vi tri giua c d

uint8_t target = 1;

//100 vua du de vuot qua vach den
Timer timer1(80), timer2(5000), timer3(500);

void wait_room(uint8_t vol) {
  mode = ROLLBACK;
  state = FOLLOWER;
}

void buzzer(uint8_t v) {
  if (v) tone(BUZZER, 500);
  else noTone(BUZZER);
}

void setup() {
  Serial.begin(9600);
  pinMode(BUZZER, OUTPUT);

  pinMode(AIA, OUTPUT);
  pinMode(AIB, OUTPUT);
  pinMode(BIA, OUTPUT);
  pinMode(BIB, OUTPUT);
  radio.initRF24();
  timer2.onTick(wait_room);
  timer3.setInterval(true);
  timer3.onTick(buzzer);
  Serial.print("Done Setup");
}

void loop()
{
  //doi phan hoi tu server
  RFWaitReponse();

  timer1.tick();
  timer2.tick();

  if (timer2.value())
  {
    timer3.tick();
  }
  else noTone(BUZZER);

  if (mode == EMPTY || mode == PAUSE)
  {
    dir = -1;
    drive(dir);
    pause();
    return;
  }

  switch (state)
  {
    case FOLLOWER:
      LineFollower();
      break;
    case COLLISION:
      {
        // vua cham vao vach den
        if (timer1.value()) drive(3);
        else
        {
          if (readSensor() != 7) state = ACROSS;
          else drive(3);
        }
      }
      break;
    case ACROSS:
      {
        dir = -1;
        drive(dir);
        pause();
        state = WAITNRF;
      }
      break;
    case WAITNRF:
      {
        if (pos == target)
        {
          if (mode == GOTOWATER)
          {
            sendData(GTDONE);
            mode = PAUSE;
            state = PAUSE;
          }
          else if (mode == GOTOTARGET)
          {
            sendData(TRDONE);
            target = 0;
            mode = WAIT;
            state = PAUSE;
            timer2.start(true);
            timer3.start(true);
          }
          else if (mode == ROLLBACK)
          {
            sendData(DONETRANS);
            mode = EMPTY;
            state = EMPTY;
          }
        }
        else state = FOLLOWER;
      }
      break;
    case PAUSE:
      {
        dir = -1;
        drive(dir);
        pause();
      }
      break;
  }
}

void RFWaitReponse() {
  if (radio.available())
  {
    bool done = false;
    while (!done)
      done = radio.read(&rx_data, sizeof(rx_data));

    Serial.print(rx_data[0]);
    Serial.print(" ");
    Serial.print(rx_data[1]);
    Serial.println();
    Serial.println(mode);

    // neu mode khac empty -> refuse
    if (rx_data[0] == GWATER)
    {
      Serial.println("GWATER");
      if (pos != 0) //0 la vi tri lay nuoc
      {
        //do line
        target = 0;
        state = FOLLOWER;
        tx_data = GOTOWATER;
        mode = GOTOWATER;
      }
      else
      {
        tx_data = GTDONE;
        mode = PAUSE;
        state = PAUSE;
        sendData(tx_data);
      }
    }
    else if (rx_data[0] == GETDONE)
    {
      Serial.println("GETDONE");
      target = rx_data[1];
      state = FOLLOWER;
      tx_data = GOTOTARGET;
      mode = GOTOTARGET;
    }
    else tx_data = REFUSE;

    bool ok = radio.nRQ_sendCommand(RFNTRY, &tx_data, sizeof(uint8_t));
  }
}

void LineFollower()
{
  int left = digitalRead(IFR_LEFT);
  int middle = digitalRead(IFR_MIDDLE);
  int right = digitalRead(IFR_RIGHT);

  if (right && left && middle) collision();
  {
    if (right) dir = 2;
    else if (left) dir = 1;
    else if (middle) dir = 3;
    // else if (dir != 2 && dir != 1) dir = 1 + random(2);

    drive(dir);
  }
}

uint8_t readSensor() {
  int left = digitalRead(IFR_LEFT);
  int middle = digitalRead(IFR_MIDDLE);
  int right = digitalRead(IFR_RIGHT);

  return (left << 2) | (middle << 1) | right;
}

void collision() {
  if (timer1.value()) return;
  state = COLLISION;
  timer1.start(true);

  pos++;
  if (pos >= NODE) pos = 0;
}

bool sendData(uint8_t data)
{
  tx_data = data;
  return radio.nRQ_sendCommand(RFNTRY, &tx_data, sizeof(uint8_t));
}

//Driver
void pause()
{
  analogWrite(AIA, 0);
  analogWrite(AIB, 0);
  analogWrite(BIA, 0);
  analogWrite(BIB, 0);
}

void backward()
{
  analogWrite(AIA, 0);
  analogWrite(AIB, spd);
  analogWrite(BIA, 0);
  analogWrite(BIB, spd);
}

void forward()
{
  analogWrite(AIA, spd);
  analogWrite(AIB, 0);
  analogWrite(BIA, spd);
  analogWrite(BIB, 0);
}

void turnright()
{
  analogWrite(AIA, spd);
  analogWrite(AIB, 0);
  analogWrite(BIA, 0);
  analogWrite(BIB, spd);
}

void turnleft()
{
  analogWrite(AIA, 0);
  analogWrite(AIB, spd);
  analogWrite(BIA, spd);
  analogWrite(BIB, 0);
}

void drive(int dir)
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
  else pause();
}
