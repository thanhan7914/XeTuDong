#include <SPI.h>
#include "APClient.h"
#include "esp_constants.h"

APClient::APClient(uint8_t rx, uint8_t tx)
{
  esp8266 = new SoftwareSerial(rx, tx);
}

APClient::~APClient()
{
  delete esp8266;
}

void APClient::setupWAP()
{
  //Setup a Wireless Access Point
  //begin Serial
  esp8266->begin(115200);
  this->delay(1000);
  initESP8266();
}

void APClient::initESP8266()
{
  sendCommand("AT+RST", 2000, false);
  sendCommand("AT+CWMODE=2", 2000, false);
  sendCommand("AT+CWSAP=\"Auto Car\",\"Gameover\",1,4", 3000, false);
  sendCommand("AT+CIFSR", 1000, false);
  sendCommand("AT+CIPMUX=1", 1000, false);
  sendCommand("AT+CIPSERVER=1,80", 1000, false);
}

void APClient::connectionHandler()
{
  if (esp8266->available())
  {
    String bufferRequest = "";
    bool hasRequest = false;
    char c;

    do
    {
      c = esp8266->read();
      if (c == '\n')
      {
        hasRequest = bufferRequest.indexOf("+IPD,") != -1 || bufferRequest.indexOf(":GET") != -1;
        if (hasRequest) break;
      }
      else bufferRequest += c;
    } while (c != -1);
    //flush
    while (esp8266->available()) esp8266->read();
    //handler
    if (requestHandler != NULL)
      requestHandler(bufferRequest);
    if (hasRequest && _onRequest != NULL)
      this->_onRequest(bufferRequest);
  }
}

void APClient::write(char c)
{
  esp8266->write(c);
}

void APClient::print(String s)
{
  esp8266->print(s);
}

String APClient::sendCommand(const String& msg, const int timeout, bool debug)
{
  String response = "";
  if (this->log != NULL) this->log(msg);
  esp8266->println(msg);

  long int time = millis();

  while ( (time + timeout) > millis())
  {
    while (esp8266->available())
    {
      // The esp has data so display its output to the serial window
      char c = esp8266->read(); // read the next character.
      response += c;
    }
  }

  if (debug)
  {
    Serial.println(response);
  }

  return response;
}

void APClient::setHandler(void (*hd)(const String& request))
{
  this->requestHandler = hd;
}

void APClient::render(const String& html)
{
  String httpHeader = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n";
  httpHeader.concat("Content-Length: ");
  httpHeader.concat(html.length());
  httpHeader.concat("\r\n");
  httpHeader.concat("Connection: close\r\n\r\n");

  int len = httpHeader.length() + html.length();

  String beginSendCmd = String(CMD_SEND_BEGIN) + "," + String(len);
  sendCommand(beginSendCmd, 5, false);
  sendCommand(httpHeader, 5, false);
  sendCommand(html, 10, false);
  sendCommand(CMD_SEND_END, 5, false);
}

void APClient::render(void(*sendMsg)(void), int html_len)
{
  String httpHeader = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n";
  httpHeader.concat("Content-Length: ");
  httpHeader.concat(html_len);
  httpHeader.concat("\r\n");
  httpHeader.concat("Connection: close\r\n\r\n");

  int len = httpHeader.length() + html_len;

  String beginSendCmd = String(CMD_SEND_BEGIN) + "," + String(len);
  sendCommand(beginSendCmd, 5, false);
  sendCommand(httpHeader, 5, false);
  sendMsg();
  sendCommand(CMD_SEND_END, 5, false);
}

void APClient::delay(unsigned long dl)
{
  unsigned long tm = millis();
  while (millis() -  tm < dl);
}

void APClient::setLog(void (*log)(const String msg))
{
  this->log = log;
}

void APClient::onRequest(void (*_onRequest)(const String& request))
{
  this->_onRequest = _onRequest;
}

