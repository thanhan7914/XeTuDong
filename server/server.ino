#include <avr/pgmspace.h>
#include <Servo.h>
#include "APClient.h"
#include "RFClient.h"
#include "Timer.h"

//define pin
#define LED_PIN 8
#define SERVO_LEFT 6
#define SERVO_RIGHT 5

//radio
#define RFNTRY 4

//State
#define EMPTY 0
#define GWATER 1
#define GETDONE 2
#define GOTOWATER 3
#define GOTOTARGET 4
#define GTDONE 5
#define TRDONE 6
#define ROLLBACK 7
#define DONETRANS 8
#define OUTOF 10
#define PAUSE 20

//phan hoi
#define REFUSE 2
#define ACCEPT 1

Servo left, right;
uint8_t angle_left[2] = {28, 42};
uint8_t angle_right[2] = {158, 140};
uint8_t cabin = 0;
uint8_t gstop = 0;
APClient esp8266(2, 4);
RFClient radio(9, 10);

//data send or receive
uint8_t tx_data[2] = {0, 0};
uint8_t rx_data;
uint16_t queue[4] = {0, 0, 0, 0};
uint8_t nrq = 0;

uint8_t state = EMPTY;
uint8_t max_water[2] = {2, 2};
uint8_t water[2] = {2, 2};
uint16_t _current = 0;

/*
   stt_order
   0 view
   1 success
   2 error
*/
uint8_t stt_order = 0;

Timer timer1(2000), timer2(100), timer3(100);

int getMemoryFree() {
  extern int __heap_start;
  extern int *__brkval;

  return (int) SP - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void writeLed(uint8_t vol) {
  digitalWrite(LED_PIN, vol);
}

void rotate(uint8_t arg0) {
  cabin = _current && 0xf;
  //0 left
  //1 right

  if (cabin == 1)
    left.write(left.read() + 1);
  else if (cabin == 2)
    right.write(right.read() - 1);
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  right.attach(SERVO_RIGHT);
  left.attach(SERVO_LEFT);
  left.write(0);
  right.write(180);
  //configuration wap
  initWAP();
  //setup nrf
  radio.initRF24();
  timer2.onTick(writeLed);
  timer2.setInterval(true);

  timer3.onTick(rotate);
  timer3.setInterval(true);

  Serial.println(getMemoryFree());

  //nhap nhay led
  timer1.start(true);
  timer2.start(true);
}

void loop() {
  if (timer1.tick() == 255)
  {
    timer2.tick();
  }
  else
  {
    timer1.setValue(false);
    writeLed(0);
  }

  RFWaitResponse();
  esp8266.connectionHandler();
  if (state == OUTOF) return;

  timer3.tick();
  if (timer3.value())
  {
    int g = 0;
    if (cabin == 1)
      g = left.read();
    else if (cabin == 2)
      g = right.read();

    if ((cabin == 1 && g >= gstop) || (cabin == 2 && g <= gstop))
    {
      timer3.pause();
      tx_data[0] = GETDONE;
      tx_data[1] = _current >> 8;
      sendData();
      state = PAUSE;
      Serial.println("done ");
      Serial.println(_current);
      Serial.println(_current >> 8);
    }
  }

  switch (state)
  {
    case EMPTY:
      if (nrq > 0)
      {
        //neu co yeu cau nuoc
        //gui yeu cau den xe
        //send request
        nrq--;
        _current = queue[nrq];
        //byte 0 chu lenh
        //byte 1 chua room.water
        //lenh
        tx_data[0] = GWATER;
        tx_data[1] = 0;
        Serial.println("Goi nuoc");
        bool ok = sendData();

        if (!ok)
        {
          nrq++;
          state = EMPTY;
        }
        else
          state = GWATER;
      }
      break;
    case GWATER:
      break;
  }
}

void RFWaitResponse() {
  if (radio.available())
  {
    long timestamp = millis();
    bool done = false;
    while (!done)
    {
      done = radio.read(&rx_data, sizeof(uint8_t));
      Serial.print("Data = ");
      Serial.println(rx_data);
    }

    Serial.print("Time: ");
    Serial.println(millis() - timestamp);

    switch (rx_data)
    {
      case GTDONE:
        {
          int f = (cabin == 1) ? water[0] : water[1];
          int m = (cabin == 1) ? max_water[0] : max_water[1];
          int h = f - m;
          if (h >= 0)
            gstop = (cabin == 1) ? angle_left[h] : angle_right[h];
          else gstop = 90;
          timer3.start(true);
        }
        break;
      case DONETRANS:
        {
          state = EMPTY;
        }
        break;
    }
  }
}

void initWAP()
{
  esp8266.setLog(log);
  esp8266.setHandler(requestHandler);
  esp8266.onRequest(render);
  esp8266.setupWAP("XeTuDong", "Gameover");
}


void log(const String msg)
{
  Serial.println(msg);
}

void requestHandler(const String& request)
{
  Serial.print("Request: ");
  Serial.println(request);

  if (water[0] == 0 && water[1] == 0)
  {
    state = OUTOF;
    return;
  }

  if (nrq < 4 && (water[0] > 0 || water[1] > 0) && !timer1.value())
  {
    tx_data[0] = 0;
    tx_data[1] = 0;
    if (request.indexOf("r1") != -1) tx_data[0] = 1;
    else if (request.indexOf("r2") != -1) tx_data[0] = 2;
    //loai nuoc
    if (request.indexOf("w1") != -1 && water[0] > 0)
    {
      tx_data[1] = 1;
      water[0]--;
    }
    else if (request.indexOf("w2") != -1 && water[1] > 0)
    {
      tx_data[1] = 2;
      water[1]--;
    }

    if (tx_data[0] > 0 && tx_data[1] > 0)
    {
      //ok
      stt_order = 1;
      queue[nrq] = tx_data[0] << 8;
      queue[nrq] |= tx_data[1];
      nrq++;
      timer1.start(true);
      timer2.start(true);
    }
  }
}

void render(const String& buffer)
{
  //374 bytes header, 392 bytes footer + xxx bytes content
  int len = 808;
  if (stt_order == 1) len += 23;
  if (nrq > 0) len += 49 + 23 * nrq;
  esp8266.render(index, len);
  stt_order = 0;
}

void index()
{
  //374 bytes
  showTmpl(PSTR("<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>He thong mang nuoc tu dong</title><style media='screen'>*{margin: 0;padding: 0}body{margin: 0 auto; text-align: center;}h1, h3{padding: 20px;}input{margin: auto 8px; padding-left: 8px;padding-right: 8px;}</style></head><body><h1>Xin chao! Ban muon loai nuoc gi?</h1><h3>"));
  //23 bytes
  if (stt_order == 1)
    showTmpl(PSTR("Yeu cau duoc chap nhan!"));
  String info  = "<br>Trang thai: ";
  info.concat(state);
  info.concat(", Lavie: ");
  info.concat(water[0]);
  info.concat(", Revive: ");
  info.concat(water[1]);
  info.concat("<br>");
  esp8266.print(info);
  if (nrq > 0)
  {
    //49 bytes
    showTmpl(PSTR("Danh sach: <br/><span style='padding-left: 25px'>"));
    for (uint8_t i = 0; i < nrq; i++)
    {
      //23 bytes
      String srq = "room ";
      srq.concat(queue[i] >> 8);
      srq.concat(": nuoc loai ");
      srq.concat(queue[i] & 0xff);
      srq.concat("<br>");
      esp8266.print(srq);
    }
  }

  //392 bytes
  showTmpl(PSTR("</h3><h4><form action='/' method='get'><select name='r'> <option value='0'>--Chon phong cua ban--</option><option value='r1'>Room 1</option><option value='r2'>Room 2</option></select><select name='w'><option value='0'>--Chon loai nuoc uong--</option><option value='w1'>Lavie</option><option value='w2'>7UP Revive</option></select><input type='submit' value='Submit'></form></h4></body></html>"));
}

void showTmpl(PGM_P s) {
  char c;
  while ((c = pgm_read_byte(s++)) != 0)
    esp8266.write(c);
}

bool sendData() {
  bool ok = radio.nRQ_sendCommand(RFNTRY, &tx_data, sizeof(tx_data));
  Serial.print("Send request");
  Serial.print(tx_data[0]);

  if (ok) Serial.println(" ok ");
  else Serial.println(" fail ");
  return ok;
}

