#ifndef _TPC8407_h
#define _TPC8407_h
#include "Arduino.h"

class TPC8407 {
  private:
    int fet2, fet4, fet6, fet8;
    int vel;
  public:
    TPC8407(int _fet2, int _fet4, int _fet6, int _fet8, int _vel);
    TPC8407(int _fet2, int _fet4, int _fet6, int _fet8);
    void forward();        // 前進
    void forward(int pwm); // pwmの速度で前進
    void back();           // 後退
    void back(int pwm);    // pwmの速度で後退
    void halt();           // オープンにする
    void brake();          // ブレーキ
};

#endif
