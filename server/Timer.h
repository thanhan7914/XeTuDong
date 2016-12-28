#ifndef __TIMER_H__
#define __TIMER_H__

#include <Arduino.h>

class Timer {
  private:
    unsigned long timestamp = 0L;
    unsigned long delay = 100L;
    uint8_t  val = 0;
    bool interval = false;
    bool istart = false;
    void (*event)(uint8_t);
  public:
    Timer() {}
    Timer(unsigned long delay) {
      this->delay = delay;
    }
    ~Timer() {}
    void onTick(void (*event)(uint8_t)) {
      this->event = event;
    }
    void setInterval(bool interval) {
      this->interval = interval;
    }
    uint8_t tick() {
      if (!istart) return 0;
      if (millis() - timestamp > this->delay)
      {
        val = 255 - val;
        timestamp = millis();
        istart = interval;
        if (event != NULL)
          event(val);
      }

      return val;
    }
    void start() {
      timestamp = millis();
      val = 0;
      this->istart = true;
    }
    void start(bool un) {
      timestamp = millis();
      val = un ? 255 : 0;
      this->istart = true;
    }
    uint8_t value() {
      return val;
    }
    void setValue(bool un) {
      val = un ? 255 : 0;
    }
    void pause() {
      this->istart = false;
      this->val = 0;
    }
};

#endif

