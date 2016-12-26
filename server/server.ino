#include <avr/pgmspace.h>
#include "APClient.h"
#include "RFClient.h"
#include "Timer.h"

#define RFNTRY 4
#define LED_PIN 8

APClient esp8266(2, 4);
RFClient radio(9, 10);

//data send or receive
uint8_t tx_data[2] = {0, 0};
uint8_t rx_data;
uint16_t queue[4] = {0, 0, 0, 0};
uint8_t nrq = 0;

uint8_t state = 0;
uint8_t water[2] = {3, 2};

/*
   stt_order
   0 view
   1 success
   2 error
*/
uint8_t stt_order = 0;

Timer timer1(2000), timer2(100);

int getMemoryFree() {
  extern int __heap_start;
  extern int *__brkval;

  return (int) SP - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void blink(uint8_t vol) {
  digitalWrite(LED_PIN, vol);
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  initWAP();
  radio.initRF24();
  timer2.onTick(blink);
  timer2.setInterval(true);
  timer1.start(true);
  timer2.start(true);
  Serial.println(getMemoryFree());
}

void loop() {
  if (timer1.tick() == 255)
  {
    timer2.tick();
  }
  else
  {
    timer1.setValue(false);
    blink(0);
  }

  RFWaitResponse();
  esp8266.connectionHandler();

  //  if (state == 0)
  //  {
  //    tx_data[0] = 9;
  //    sendData();
  //  }
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
    }
}

void initWAP()
{
  esp8266.setLog(log);
  esp8266.setHandler(requestHandler);
  esp8266.onRequest(render);
  esp8266.setupWAP();
}


void log(const String msg)
{
  Serial.println(msg);
}

void requestHandler(const String& request)
{
  Serial.print("Request: ");
  Serial.println(request);

  if (nrq < 4 && water[0] > 0 && water[1] > 0 && !timer1.value())
  {
    tx_data[0] = 0;
    tx_data[1] = 0;
    if (request.indexOf("r1") != -1) tx_data[0] = 1;
    else if (request.indexOf("r2") != -1) tx_data[0] = 2;
    //loai nuoc
    if (request.indexOf("w1") != -1) tx_data[1] = 1;
    else if (request.indexOf("w2") != -1) tx_data[1] = 2;

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

  showTmpl(PSTR("</h3><h4><form action='/' method='get'><select name='r'> <option value='0'>--Chon phong cua ban--</option><option value='r1'>Room 1</option><option value='r2'>Room 2</option></select><select name='w'><option value='0'>--Chon loai nuoc uong--</option><option value='w1'>Lavie</option><option value='w2'>7UP Revive</option></select><input type='submit' value='Submit'></form></h4></body></html>"));
}

void showTmpl(PGM_P s) {
  char c;
  while ((c = pgm_read_byte(s++)) != 0)
    esp8266.write(c);
}

bool sendData() {
  bool ok = radio.nRQ_sendCommand(RFNTRY, &tx_data, sizeof(uint8_t));
  Serial.print("Send request");
  Serial.print(tx_data[0]);

  if (ok) Serial.println(" ok ");
  else Serial.println(" fail ");
  return ok;
}

