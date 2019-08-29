/*
   ____   ___  _  ___  
  |___ \ / _ \/ |/ _ \ 
    __) | | | | | (_) |
   / __/| |_| | |\__, |
  |_____|\___/|_|  /_/ 

   ____       _           _   
  |  _ \ ___ | |__   ___ | |_ 
  | |_) / _ \| '_ \ / _ \| __|
  |  _ < (_) | |_) | (_) | |_ 
  |_| \_\___/|_.__/ \___/ \__|
                            
   _____     _       _   _                     
  |_   _| __(_) __ _| |_| |__  _ __ ___  _ __  
    | || '__| |/ _` | __| '_ \| '__/ _ \| '_ \ 
    | || |  | | (_| | |_| | | | | | (_) | | | |
    |_||_|  |_|\__,_|\__|_| |_|_|  \___/|_| |_|
                                             
   チーム名 夢工房B
   機体名   うすしお

   ロボトラ用のプログラムです

   使用マイコン Arduino Due
   32Bit, 3.3V駆動なので気をつけて

   Created 2019/08/28〜
   By Ebina
 */


const int8_t  FOT_NUM     = 8;                     // フォトセンサの数
const int32_t MAXLIGHT    = 100000;                // 最大値
const int32_t MINLIGHT    = 0;                     // 最小値
const int32_t MIDDLELIGHT = (MAXLIGHT+MINLIGHT)/2; // 中間値

int32_t light[FOT_NUM];    // 取ってきた明るさ
int32_t maxlight[FOT_NUM]; // 明るさの最大値
int32_t minlight[FOT_NUM]; // 明るさの最小値
int32_t timer;

int8_t brightnum  = 0; // 一番明るい場所
int8_t pbrightnum = 0; // 一個前の明るい場所
double angle = 0.0;    // 角度

void setup() {
  Serial.begin(115200);                // debug用
  delay(100);                          // 多分安全

  analogReadResolution(12);            // analogReadが12bitで読まれる(Dueのみ）

  for(int8_t i = 0; i < FOT_NUM; i++){ // 初期化
    maxlight[i] = -32000;
    minlight[i] =  32000;
  }
  timer = millis();

  configure_initial();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void loop() {
  fot_read();
  normalize();               // 正規化
  brightnum = maxlightnum(); // 最大の明るさ
  getangle();                // 角度
  Serial.print(angle);
  Serial.println("");

  delay(10);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/* void right_motor_forward(int pwm){ */
/*   analogWrite(RM1, pwm); */
/*   digitalWrite(RM2, LOW); */
/* } */

/* void right_motor_back(int pwm){ */
/*   digitalWrite(RM1, LOW); */
/*   analogWrite(RM2, pwm); */
/* } */

/* void left_motor_forward(int pwm){ */
/*   analogWrite(LM1, pwm); */
/*   digitalWrite(LM2, LOW); */
/* } */

/* void left_motor_back(int pwm){ */
/*   digitalWrite(LM1, LOW); */
/*   analogWrite(LM2, pwm); */
/* } */

/* void right_motor_halt(){ */
/*   digitalWrite */
/* } */

void configure_initial(){     // 最初に行われる初期設定
  while(millis()-timer < 2000){ // 最大, 最小値の更新
    fot_read();
    getminmax();
  }
}

void fot_read(){ // センサの値を読む
  for(int8_t i = 0; i < FOT_NUM; i++){
    light[i] = analogRead(i);
  }
}

void getminmax(){ // 最大値と最小値を更新
  for(int8_t i = 0; i < FOT_NUM; i++){
    maxlight[i] = max(maxlight[i], light[i]);
    minlight[i] = min(minlight[i], light[i]);
  }
}

void normalize(){ // 最大値と最小値で正規化
  for(int8_t i = 0; i < FOT_NUM; i++){
    light[i] = map(light[i], minlight[i], maxlight[i], MINLIGHT, MAXLIGHT);
    light[i] = constrain(light[i], MINLIGHT, MAXLIGHT);
  }
}

int8_t maxlightnum(){                // 最大の明るさの場所
  int8_t maxnum = 0;                   // 場所
  int32_t maxper = light[0];           // 最大の明るさ
  for(int8_t i = 1; i < FOT_NUM; i++){
    if(maxper < light[i]){
      maxper = light[i];
      maxnum = i;
    }
  }

  if(maxper < MIDDLELIGHT) return -1;  // どれも明るくない
  return maxnum;
}

void show_light(){ // debug用
  for(int8_t i = 0; i < FOT_NUM; i++){
    if(i) Serial.print(",");
    Serial.print(light[i]);
  }
  /* Serial.print("\n"); */
}

void getangle(){                                         // 角度を返す
  if(brightnum == -1){
    if(pbrightnum == 0) angle = -255.0;                      // 最小値
  }else{
    if(pbrightnum == 0) angle = -255.0;                      // 最小値
    else if(brightnum != FOT_NUM-1){
      double cur = abs(light[brightnum+1]-light[brightnum-1]); // 一番明るいところの左右の差
      // {{{ 特殊な操作
      cur = pow(cur, 0.25);

      cur = mymap(cur, 5.5, 17.0, -1.0, 1.0);
      cur = constrain(cur, -1.0, 1.0);

      cur = acos(cur);

      if(light[brightnum+1]-light[brightnum-1] > 0) cur = 6.0-cur;
      cur += (brightnum-1)*6.0;
      // }}}
      cur = mymap(cur, 0.0, 36.0, -255.0, 255.0);
      angle = constrain(cur, -255.0, 255.0);
    }
    pbrightnum = brightnum;
  }
}

double mymap(double x, double in_min, double in_max, double out_min, double out_max){ // double型のmap
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
