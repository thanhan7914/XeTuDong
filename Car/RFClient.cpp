#include "RFClient.h"
#include "printf.h"

RFClient::RFClient(uint8_t _cepin, uint8_t _cspin)
{
  radio = new RF24(_cepin, _cspin);
}

RFClient::~RFClient()
{
  delete radio;
}

void RFClient::RFListening()
{
  isRFListening = true;
  radio->startListening();
}

void RFClient::initRF24()
{
  printf_begin();
  radio->begin();
  radio->openWritingPipe(pipes[0]);
  radio->openReadingPipe(1, pipes[1]);
  RFListening();
  radio->printDetails();
}

bool RFClient::nRQ_sendCommand(uint8_t n, const void* buf, uint8_t len)
{
  if (isRFListening) radio->stopListening();
  bool ok = false;
  int _delay = 30;

  while (n > 0)
  {
    ok = radio->write( buf, len);
    printf("Sent response.\n\r");
    n--;
    if (ok || n == 0) break;
    unsigned long tm = millis();
    while (millis() - tm < _delay);
  }

  RFListening();
  timestamp = millis();
  return ok;
}

bool RFClient::sendCommand(const void* buf, uint8_t len)
{
  return nRQ_sendCommand(1, buf, len);
}

bool RFClient::available()
{
  return radio->available();
}

bool RFClient::read(void* buf, uint8_t len)
{
  return radio->read(buf, len);
}

