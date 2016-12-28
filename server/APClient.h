#ifndef __APClient_H__
#define __APClient_H__

#include <SoftwareSerial.h>

class APClient {
  private:
    SoftwareSerial* esp8266;
    void (*requestHandler)(const String& request);
    void (*log)(const String msg);
    void (*_onRequest)(const String& request);
  public:
    APClient(uint8_t rx, uint8_t tx);
    ~APClient();
    void setupWAP(const String ssid, const String password);
    void initESP8266(const String ssid, const String password);
    void connectionHandler();
    String sendCommand(const String& msg, const int timeout, bool debug);
    void setHandler(void (*hd)(const String& request));
    void render(const String& html);
    void render(void(*sendMsg)(void), int html_len);
    void sleep(unsigned long dl);
    void setLog(void (*log)(const String msg));
    void onRequest(void (*_onRequest)(const String& request));
    void write(char c);
    void print(String s);
};

#endif

