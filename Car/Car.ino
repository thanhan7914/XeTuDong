#include "RFClient.h"
#include "Driver.h"
#include "Timer.h"

//radio
#define RFNTRY 4

#define LED_PIN 8
#define LED_M A1
#define LED_R A0

//STATE
#define EMPTY 0
#define FOLLOWER 1
#define COLLISION 2
#define ACROSS 3
#define PAUSE 10
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

//dinh nghi phan hoi
#define REFUSE 2
#define ACCEPT 1

//so luong vi tri
#define NODE 4

const int IFR_LEFT = A3;
const int IFR_MIDDLE = A4;
const int IFR_RIGHT = A5;

RFClient radio(9, 10);
Driver car(6, A2, 5, 3);

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
//tracking
uint8_t trace[5] = {0, 0, 0, 0, 0};
uint8_t itrace = 0;
uint8_t pos = 0; // vi tri giua c d

uint8_t target = 0;

//100 vua du de vuot qua vach den
Timer timer1(100);

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_M, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  car.init();
  radio.initRF24();
  Serial.print("Done Setup");
}

void loop()
{
  //doi phan hoi tu server
  RFWaitReponse();

  timer1.tick();

  if (mode == EMPTY || mode == PAUSE) return;

  switch (state)
  {
    case FOLLOWER:
      LineFollower();
      break;
    case COLLISION:
      {
        // vua cham vao vach den
        if (timer1.value()) car.drive(3);
        else
        {
          if (readSensor() != 7) state = ACROSS;
          else car.drive(3);
        }
      }
      break;
    case ACROSS:
      {
        dir = -1;
        car.drive(dir);
        car.pause();
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
            mode = ROLLBACK;
            state = FOLLOWER;
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
        car.drive(dir);
        car.pause();
      }
      break;
  }
}

void tracking(int p) {
  if (itrace < 5)
    trace[itrace++] = p;
  else
  {
    for (uint8_t i = 3; i >= 0; i--)
      trace[i] = trace[i + 1];

    trace[4] = p;
  }
}

uint8_t trace_index() {
  if (itrace < 5) return itrace;
  return 4;
}

void RFWaitReponse() {
  if (radio.available())
  {
    bool done = false;
    while (!done)
      done = radio.read(&rx_data, sizeof(rx_data));

    // neu mode khac empty -> refuse
    if (mode != EMPTY) tx_data = REFUSE;
    else
    {
      switch (rx_data[0])
      {
        case GWATER:
          if (pos != 0) //0 la vi tri lay nuoc
          {
            //do line
            target = 0;
            state = FOLLOWER;
            tx_data = GOTOWATER;
            mode = GOTOWATER;
          }
          break;
        case GETDONE:
          {

            target = rx_data[1] >> 4;
            state = FOLLOWER;
            tx_data = GOTOTARGET;
            mode = GOTOTARGET;
          }
          break;
      }
    }

    bool ok = radio.nRQ_sendCommand(RFNTRY, &tx_data, sizeof(uint8_t));
  }
}

void LineFollower()
{
  int left = digitalRead(IFR_LEFT);
  int middle = digitalRead(IFR_MIDDLE);
  int right = digitalRead(IFR_RIGHT);

  if (left) digitalWrite(LED_PIN, HIGH);
  else digitalWrite(LED_PIN, LOW);

  if (middle) digitalWrite(LED_M, HIGH);
  else digitalWrite(LED_M, LOW);

  if (right) digitalWrite(LED_R, HIGH);
  else digitalWrite(LED_R, LOW);

  if (right && left && middle) collision();
  {
    if (middle) dir = 3;
    else if (left) dir = 1;
    else if (right) dir = 2;
    else if (dir != 2 && dir != 1) dir = 1 + random(2);

    car.drive(dir);
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

  tracking(pos);
  pos++;
  if (pos >= NODE) pos = 0;
  digitalWrite(LED_R, LOW);
  digitalWrite(LED_M, LOW);
  digitalWrite(LED_PIN, LOW);
}

bool sendData(uint8_t data)
{
  tx_data = data;
  return radio.nRQ_sendCommand(RFNTRY, &tx_data, sizeof(uint8_t));
}

