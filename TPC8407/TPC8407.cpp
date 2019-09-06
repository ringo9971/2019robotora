#include "Arduino.h"
#include "TPC8407.h"

TPC8407::TPC8407(int _fet2, int _fet4, int _fet6, int _fet8, int _vel) {
  fet2 = _fet2;
  fet4 = _fet4;
  fet6 = _fet6;
  fet8 = _fet8;
  vel  = _vel;
  pinMode(fet2, OUTPUT);
  pinMode(fet4, OUTPUT);
  pinMode(fet6, OUTPUT);
  pinMode(fet8, OUTPUT);
}
TPC8407::TPC8407(int _fet2, int _fet4, int _fet6, int _fet8) {
  fet2 = _fet2;
  fet4 = _fet4;
  fet6 = _fet6;
  fet8 = _fet8;
  vel = 0;
  pinMode(fet2, OUTPUT);
  pinMode(fet4, OUTPUT);
  pinMode(fet6, OUTPUT);
  pinMode(fet8, OUTPUT);
}

void TPC8407::forward() {
  analogWrite(fet2, 255-vel);
  digitalWrite(fet4, LOW);
  digitalWrite(fet6, HIGH);
  digitalWrite(fet8, LOW);
}
void TPC8407::forward(int pwm) {
  analogWrite(fet2, 255-pwm);
  digitalWrite(fet4, LOW);
  digitalWrite(fet6, HIGH);
  digitalWrite(fet8, LOW);
}

void TPC8407::back() {
  digitalWrite(fet2, LOW);
  analogWrite(fet4, 255-vel);
  digitalWrite(fet6, LOW);
  digitalWrite (fet8, HIGH);
}
void TPC8407::back(int pwm) {
  digitalWrite(fet2, LOW);
  analogWrite(fet4, 255-pwm);
  digitalWrite(fet6, LOW);
  digitalWrite (fet8, HIGH);
}

void TPC8407::halt() {
  digitalWrite(fet2, LOW);
  digitalWrite(fet4, LOW);
  digitalWrite(fet6, HIGH);
  digitalWrite(fet8, HIGH);
}

void TPC8407::brake() {
  digitalWrite(fet2, HIGH);
  digitalWrite(fet4, HIGH);
  digitalWrite(fet6, HIGH);
  digitalWrite(fet8, HIGH);
}
