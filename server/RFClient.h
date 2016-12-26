#ifndef __RFClient_H__
#define __RFClient_H__

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

class RFClient {
  private:
    const int RF_TIMEOUT = 250;
    const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
    RF24* radio;
    bool isRFListening = false;
    unsigned long timestamp;
  public:
    RFClient(uint8_t _cepin, uint8_t _cspin);
    ~RFClient();
    void RFListening();
    void initRF24();
    bool nRQ_sendCommand(uint8_t n, const void* buf, uint8_t len);
    bool sendCommand(const void* buf, uint8_t len);
    bool available();
    bool read(void* buf, uint8_t len);
    unsigned long getTimestamp() {return timestamp;}
};

#endif

