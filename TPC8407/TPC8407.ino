#include "TPC8407.h"

// モーターのコンストラクタ
TPC8407 motor(2, 4, 3, 5);

void setup() {
}

void loop() {
  // 1秒間前進
  motor.forward();
  delay(1000);

  // 1秒間オープンにする
  motor.halt();
  delay(1000);

  // 1秒間後退
  motor.back();
  delay(1000);

  // 1秒間ブレーキ
  motor.brake();
  delay(1000);
}

