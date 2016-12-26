#include "RFClient.h"
#include "Driver.h"
#include "Timer.h"

#define RFNTRY 4
#define LED_PIN 8
#define LED_M A1
#define LED_R A0

const int IFR_LEFT = A3;
const int IFR_MIDDLE = A4;
const int IFR_RIGHT = A5;

RFClient radio(9, 10);
Driver car(7, 6, 5, 3);

//data send or receive
uint8_t rx_data[2]; //receive 2 byte
uint8_t tx_data = 0; //send 1 byte

/*
   state
   0: nhan roi
   1: di chuyen
*/

uint8_t state = 1;
/*
   mark
   0 chua gap
   1 gap vat diem danh dau
*/
bool mark = false;
/*
   map
   a - > c - > d -> b
   0     1     2    3
*/
// huong + / -
uint8_t flip = 1;
int     dir = -1;
uint8_t trace[5] = {0, 0, 0, 0, 0};
uint8_t pos = 1; // o giua c d
uint8_t itrace = 0;

uint8_t target = 3;

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

  if (state == 1)
  {
    LineFollower();
  }
  else if (state == 2)
  {
    // vua cham vao vach den
    if (!timer1.value()) car.drive(flip == 1 ? 3 : 4);
    else
    {
      if (readSensor() != 7) state = 3;
      else car.drive(flip == 1 ? 3 : 4);
    }
  }
  else if (state == 3)
  {
    if(pos == target) state = 10;
    else state = 1;
  }
  else if(state == 10)
  {
    //pause
    pause();
  }
}

void tracking(int p) {
  if (itrace < 5)
    trace[itrace++] = p;
  else
  {
    for (uint8_t i = 3; i >= 0; i--)
      trace[i] = trace[i + 1];

    trace[5] = p;
  }
}

uint8_t trace_index() {
  if (itrace < 5) return itrace;
  return 4;
}

void RFWaitReponse() {
  if (radio.available())
  {
    unsigned long timestamp = millis();
    bool done = false;
    while (!done)
    {
      done = radio.read(&rx_data, sizeof(rx_data));
      Serial.print("Data = ");
      Serial.print(rx_data[0]);
      Serial.print(" ");
      Serial.println(rx_data[1]);
    }

    Serial.print("T1: ");
    Serial.print(millis() - timestamp);
    Serial.print(" ");

    bool ok = radio.nRQ_sendCommand(RFNTRY, &tx_data, sizeof(uint8_t));

    if (ok) Serial.print("ok ");
    else Serial.print("fail ");
    Serial.print(" Time: ");
    Serial.println(millis() - timestamp);
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

  if (right && left && middle) across();
  {
    if (right) dir = 2;
    else if (left) dir = 1;
    else if (middle) dir = (flip == 1 ? 3 : 4);
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

void across() {
  state = 2;
  timer1.start();

  tracking(pos);
  pos += flip;
  digitalWrite(LED_R, LOW);
  digitalWrite(LED_M, LOW);
  digitalWrite(LED_PIN, LOW);
}

void pause()
{
  dir = -1;
  car.stop();
}

