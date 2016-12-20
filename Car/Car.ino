#include "RFClient.h"
#include "APClient.h"
#include "Driver.h"
#include "html_template.h"

int LED_PIN = 8;
int dir = -1;

const int IFR_LEFT = A3;
const int IFR_MIDDLE = A4;
const int IFR_RIGHT = A5;

APClient esp8266(2, 4);
//RFClient radio(9, 10);
Driver car(9, 6, 5, 3);

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  // initWAP();
  digitalWrite(LED_PIN, HIGH);
  car.init();
  Serial.print("Done Setup");
}

void loop()
{
  // esp8266.connectionHandler();
  LineFollower();
}

void initWAP()
{
  esp8266.setLog(log);
  esp8266.setHandler(requestHandler);
  esp8266.onRequest(render);
  esp8266.setupWAP();
}

void LineFollower()
{
  int left = digitalRead(IFR_LEFT);
  int middle = digitalRead(IFR_MIDDLE);
  int right = digitalRead(IFR_RIGHT);

  if (right) dir = 2;
  else if (left) dir = 1;
  else if (middle) dir = 3;
  else if (dir != 2 || dir != 1) dir = 1 + random(2);

  car.drive(dir, 255);
}

void log(const String msg)
{
  Serial.println(msg);
}

void requestHandler(const String& request)
{
  Serial.println(request);

  if (request.indexOf("L1off") != -1)
  {
    digitalWrite(LED_PIN, LOW);
  }
  else if (request.indexOf("L1on") != -1)
  {
    digitalWrite(LED_PIN, HIGH);
  }
}

void render(const String& buffer)
{
  esp8266.render(tmpl_index);
}

