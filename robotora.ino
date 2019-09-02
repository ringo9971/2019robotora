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

#include <TPC8407.h>

// モーターのコンストラクタ
TPC8407 right_motor(5, 4, 3, 2);
TPC8407 left_motor(9, 8, 7, 6);

// フォトセンサ関連
const int32_t FOT_NUM     = 8;                     // フォトセンサの数
const int32_t MAXLIGHT    = 100000;                // 明るさの最大値
const int32_t MINLIGHT    = 0;                     // 明るさの最小値
const int32_t MIDDLELIGHT = (MAXLIGHT+MINLIGHT)/2; // 明るさの中間値
int32_t light[FOT_NUM];                            // 取ってきた明るさ
int32_t maxlight[FOT_NUM];                         // 取ってきた明るさの最大値
int32_t minlight[FOT_NUM];                         // 取ってきた明るさの最小値
int32_t brightnum  = 0;                            // 一番明るい場所
int32_t pbrightnum = 0;                            // 一個前の明るい場所
boolean lightflag[FOT_NUM];                        // 明るかったらtrue

// モータースピード
const int32_t MAXSPEEDNUM = 2;                    // スピードレベルの個数
const int32_t rightspeed[MAXSPEEDNUM] = {255, 0}; // 遅い→ 速い
const int32_t leftspeed[MAXSPEEDNUM] = {255, 0};  // 遅い→ 速い
boolean rightturn = false;                        // 右直角
boolean leftturn = false;                         // 左直角

// PID
const double pgain = 0.93; // pgain
const double dgain = 130;  // dgain
int32_t manipulation;      // 操作量
double angle  = 0.0;       // 角度
double pangle = 0.0;       // 一個前の角度
boolean lineflag = true;   // ライン上にいるか

int32_t timer; // timer

void setup() {
  Serial.begin(115200);                // debug用
  delay(100);                          // 多分安全

  analogReadResolution(12);            // analogReadが12bitで読まれる(Dueのみ）

  for(int32_t i = 0; i < FOT_NUM; i++){ // 初期化
    maxlight[i] = -32000;
    minlight[i] =  32000;
  }
  timer = millis();

  configure_initial();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void loop() {
  getangle();
  Serial.print("righ:");
  Serial.print(rightturn?1:0);
  Serial.print("\t");
  Serial.print("left:");
  Serial.print(leftturn?1:0);
  Serial.println("");
  rightturn = false;
  leftturn = false;
  linetrace_motor_operation(1);
  /* Serial.println(manipulation); */
  /* Serial.print(angle); */
  /* Serial.println(""); */

/*   delay(10); */
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

// 操作量の計算
void manipulation_calc(){ 
  manipulation = pgain*angle+dgain*(angle-pangle);   // PD
  manipulation = constrain(manipulation, -255, 255); // 安全のために必要
}

// 右に90度曲がる
void r_rightangle(int32_t speed){
  stop();                                         // デッドタイム
  while(lineflag){                                // 一度ラインが見えなくなるまで回転
    right_motor.back(rightspeed[speed]);
    left_motor.forward(leftspeed[speed]);
  }
  while(!lineflag || angle >= 120){               // 復帰させる
    right_motor.back(rightspeed[max(speed-1, 0)]);
    left_motor.forward(leftspeed[max(speed-1, 0)]);
  }
  stop();                                         // デッドタイム
  rightturn = false;
}

// 左に90度曲がる
void l_leftangle(int32_t speed){
  stop();                                           // デッドタイム
  while(lineflag){                                  // 一度ラインが見えなくなるまで回転
    right_motor.forward(rightspeed[speed]);
    left_motor.back(leftspeed[speed]);
  }
  while(!lineflag || angle <= -120){                // 復帰させる
    right_motor.forward(rightspeed[max(speed-1, 0)]);
    left_motor.back(leftspeed[max(speed-1, 0)]);
  }
  stop();                                           // デッドタイム
  leftturn = false;
}

// 停止(デッドタイム1ms用)
void stop(){
  right_motor.halt();
  left_motor.halt();
  delay(1);
}

// モーターを動かす
void motor_operation(int32_t speed){
  int32_t rv, lv;                       // 右と左のpwm
  if(rightturn) r_rightangle(speed);    // 右直角だったら
  else if(leftturn) l_leftangle(speed); // 左直角だったら
  else if(manipulation >= 0){           // 右回転
    rv = map(manipulation, 0, 255, rightspeed[speed], rightspeed[max(speed-1, 0)]);
    lv = map(manipulation, 0, 255, leftspeed[speed], leftspeed[min(speed+1, MAXSPEEDNUM-1)]);
    rv = constrain(rv, 0, 255);
    lv = constrain(lv, 0, 255);
    right_motor.forward(rv);
    left_motor.forward(lv);
  }else{                                // 左回転
    rv = map(manipulation, 0, -255, rightspeed[speed], rightspeed[min(speed+1, MAXSPEEDNUM-1)]);
    lv = map(manipulation, 0, -255, leftspeed[speed], leftspeed[max(speed-1, 0)]);
    rv = constrain(rv, 0, 255);
    lv = constrain(lv, 0, 255);
    right_motor.forward(rv);
    left_motor.forward(lv);
  }
}

// ライントレースをする
void linetrace_motor_operation(int speed){
  manipulation_calc();
  motor_operation(speed);
}

// 最初に行われる初期設定
void configure_initial(){
  while(millis()-timer < 2000){ // 最大, 最小値の更新
    fot_read();
    getminmax();
  }
}

// センサの値を読む
void fot_read(){
  for(int32_t i = 0; i < FOT_NUM; i++){
    light[i] = analogRead(i);
  }
}

// 最大値と最小値を更新
void getminmax(){ 
  for(int32_t i = 0; i < FOT_NUM; i++){
    maxlight[i] = max(maxlight[i], light[i]);
    minlight[i] = min(minlight[i], light[i]);
  }
}

// 最大値と最小値で正規化
void normalize(){ 
  for(int32_t i = 0; i < FOT_NUM; i++){
    light[i] = map(light[i], minlight[i], maxlight[i], MINLIGHT, MAXLIGHT);
    light[i] = constrain(light[i], MINLIGHT, MAXLIGHT);
  }
  for(int32_t i = 0; i < FOT_NUM; i++){
    lightflag[i] = light[i]>MIDDLELIGHT?true:false;
  }
}

// 最大の明るさの場所
int32_t maxlightnum(){                
  int32_t maxnum = 0;                   // 場所
  int32_t maxper = light[0];            // 最大の明るさ
  for(int32_t i = 1; i < FOT_NUM; i++){
    if(maxper < light[i]){
      maxper = light[i];
      maxnum = i;
    }
  }

  if(maxper < MIDDLELIGHT) return -1;   // どれも明るくない
  return maxnum;
}

// debug用
void show_light(){ 
  for(int32_t i = 0; i < FOT_NUM; i++){
    if(i) Serial.print(",");
    Serial.print(light[i]);
  }
  /* Serial.print("\n"); */
}

// 角度の更新
void getangle(){                                         
  fot_read();
  normalize();                                             // 正規化
  brightnum = maxlightnum();                               // 最大の明るさ
  pangle = angle;
  if(lightflag[0] && lightflag[7]){
    rightturn = true;
    leftturn = true;
  }else if(lightflag[0] && lightflag[3]){
    leftturn = true;
  }else if(lightflag[4] && lightflag[7]){
    rightturn = true;
  }

  if(brightnum == -1){
    if(pbrightnum == 0) angle = -255.0;                      // 最小値
    rightturn = true;
    leftturn  = true;
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

// double型のmap
double mymap(double x, double in_min, double in_max, double out_min, double out_max){ 
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
