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

   カテゴリ           ノーマル
   ギアボックス       タミヤ製・6速ギアボックスHE
   モータ             マブチモータRE-260
   モータの端子間電圧 5V
   移動用電池         エネループ6本

   ロボトラ用のプログラムです

   使用マイコン Arduino Due
   32Bit, 3.3V駆動なので気をつけて

   Created 2019/08/28〜
   By Ebina
*/

#include <TPC8407.h>
#include <Servo.h>

// サーボのコンストラクタ
Servo sv;

// モーターのコンストラクタ
TPC8407 right_motor(9, 8, 11, 10);
TPC8407 left_motor(4, 5, 2, 3);

// フォトセンサ関連
const int32_t FOT_NUM     = 8;                     // フォトセンサの数
const int32_t MAXLIGHT    = 100000;                // 明るさの最大値
const int32_t MINLIGHT    = 0;                     // 明るさの最小値
const int32_t MIDDLELIGHT = (MAXLIGHT+MINLIGHT)/2; // 明るさの中間値
int32_t light[FOT_NUM];                            // 取ってきた明るさ
int32_t brightnum  = 0;                            // 一番明るい場所
int32_t pbrightnum = 0;                            // 一個前の明るい場所
boolean lightflag[FOT_NUM];                        // 明るかったらtrue
  /* 3376,  3278,  3233,  3368,  3525,  3467,  3260,  3266 */
int32_t maxlight[FOT_NUM] = { // 取ってきた明るさの最大値
3401,
3250,
3170,
3336,
3513,
3428,
3198,
3268
};
  /* 1037,  934,  483,  924,  1218,  1429,  646,  1093 */
int32_t minlight[FOT_NUM] = { // 取ってきた明るさの最小値
615,
562,
282,
453,
966,
1055,
321,
693
};

int32_t theta = 75;

// モータースピード
const int32_t MAXSPEED = 4;                      // 最大スピード
const int32_t rightspeed[5] = {255, 170, 142, 65, 0}; // 遅い→ 速い
const int32_t leftspeed[5]  = {255, 180, 150, 70, 10}; // 遅い→ 速い

// フラグ
boolean rightturn = false; // 右直角
boolean leftturn = false;  // 左直角
boolean lineflag = true;   // ライン上にいるか

// PD制御
const double pgain = 1.25; // pgain
const double dgain = 230;  // dgain
int32_t manipulation;      // 操作量
double angle  = 0.0;       // 角度
double pangle = 0.0;       // 一個前の角度

// psd関係
int32_t rpsd, mpsd, lpsd;
int32_t prpsd = -1, pmpsd = -1, plpsd = -1;
int32_t now, past = -1;
double lpf = 0.7;
// 迷いの森
const int32_t obs_ad = 2000;
int32_t direction = 0, pdirection = 3;
int32_t forestslope = 0;
int32_t foresttimer;
boolean robs = false, mobs = false, lobs = false;
boolean straight = true;

// loop内で使用
int32_t state = 150; // switch-caseで使用
int32_t timer;       // timer

void setup() {
  Serial.begin(115200); // debug用
  delay(100);           // 多分安全

  pinMode(15, INPUT_PULLUP); // 右側スイッチ
  pinMode(18, INPUT_PULLUP); // 真ん中スイッチ
  pinMode(21, INPUT_PULLUP); // 左側スイッチ

  switch(!digitalRead(21)*4+!digitalRead(18)*2+!digitalRead(15)){
    case 0:
      state = 0;   // スタート
      break;
    case 1:
      state = 4;   // 迷いの森
      break;
    case 2:
      state = 100; // 迷いの森抜けた後
      break;
    case 3:
      state = 150; // ボーナス
      break;
    case 4:
      state = 160;
      break;
    case 5:
      state = 300;
      break;
    case 6:
      state = 999;
      break;
    case 7:
      state = 1000;
      break;
    default:
      state = 0;
  }

  sv.attach(13);
  sv.write(75);

  analogReadResolution(12); // analogReadが12bitで読まれる(Dueのみ）

  timer = millis();         // timerの更新
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void loop() {
  while(true){
    getangle();
    read_psd();
    switch(state){
       // 迷いの森まで
      case 0:
        if(millis()-timer >= 800 && rightturn){ // 最初のT字路
          r_rightangle(3, 2, 30);
          update();
        }else{
          linetrace_motor_operation(1);
          fold_flag();
        }
        break;
      case 1:
        linetrace_motor_operation(2);
        if(rightturn || leftturn){               // ボール前のT字路
          right_motor.forward(rightspeed[2]);
          left_motor.brake();
          delay(85);
          brake();
          delay(150);
          forward(2);
          delay(660);
          stop();                                // 少し待機児童
          delay(1500);
          back(2);
          delay(300);
          l_leftangle(MAXSPEED);                 // T字路
          l_leftangle(2, 2, 150);
          update();
        }
        break;
      case 2:
        linetrace_motor_operation(2);
        if(rightturn || leftturn){               // 最初のT字路
          forward(2);
          delay(150);
          r_rightangle(MAXSPEED, 2);             // 右に曲がる
          fold_flag();
          update();
        }
        break;
      case 3:
        linetrace_motor_operation(MAXSPEED);     // 直線
        if(leftturn){               // 迷いの森前のT字路
          right_motor.halt();
          left_motor.forward(leftspeed[2]);
          delay(100);
          update();
        }
        break;

       // ここから迷いの森
      case 4: 
        forest(1);
        if(millis()-timer >= 1000 && lineflag){
          update();
        }
        break;
      case 5:
        forest(1);
        if(millis()-timer >= 1000 && lineflag){
          update();
        }
        break;
      case 6:
        forward(1);
        delay(200);
        l_leftangle(MAXSPEED);
        update();
        break;
      case 7:
        state = 100;

       // 迷いの森抜けた後
      case 100:
        linetrace_motor_operation(MAXSPEED);
        if(!lineflag){                           // 1個目の左カーブ
          stop();
          brake();
          delay(100);
          l_leftangle(MAXSPEED);
          update();
        }
        break;
      case 101:                                  // うねうね
        linetrace_motor_operation(MAXSPEED-1);
        if(millis()-timer >= 4000){              // 右カーブ
          if(rightturn || leftturn){
            stop();
            brake();
            delay(100);
            r_rightangle(MAXSPEED);
            update();
          }
        }else{
          fold_flag();
        }
        break;
      case 102:                                  // 最後の直線
        linetrace_motor_operation(MAXSPEED);
        if(millis()-timer >= 2500){
          if(rightturn || leftturn) update();
        }
        break;
      case 103:
        state = 150;
        break;


       // ボーナスの所
      case 150:
        if(millis()-timer >= 1000 && (rightturn || leftturn)){ // 十字路
          forward(1);
          delay(200);
          l_leftangle(2, 1, 90);
          update();
        }else{
          linetrace_motor_operation(1);
          fold_flag();                      // requied
        }
        break;
      case 151:
        if(analogRead(10) > 2000) update(); // コーンが近づいたら
        else linetrace_motor_operation(1);
        break;
      case 152:
        stop();
        left_rotation(1);                   // 少し左回転
        delay(550);
        read_psd();
        while(mpsd <= 1500){
          read_psd();
          left_rotation(1);                   // 少し左回転
        }
        stop();
        update();
      case 153:
        update();
        break;
      case 154:
        read_psd();
        if(analogRead(10) > 3500){          // コーンに接近
          update();
        }else{
          go_to_goal();
        }
        break;
      case 155:
        if(millis()-timer <= 200){
          go_to_goal();
        }else{
          stop();                           // ここでシュートする
          delay(1500);
          update();
        }
        break;
      case 156:
        back(1);
        delay(1200);
        stop();
        right_motor.forward(rightspeed[1]);
        delay(200);
        l_leftangle(1, 1, 90);
        update();
        break;
      case 157:
        if(rightturn || leftturn){ // 十字路
          update();
        }else{
          linetrace_motor_operation(1);
        }
        break;
      case 158:
        forward(1);
        delay(180);
        l_leftangle(2, 1, 50);
        update();
        break;
      case 159:                              // ボール回収
        stop();

        right_motor.forward(rightspeed[1]);
        delay(100);

        forward(1);
        delay(1050);
        stop();
        delay(1500);
        back(1);
        delay(1300);
        /* l_leftangle(1); */
        l_leftangle(1, 1, 30);
        update();
        break;

      case 160:
        if(analogRead(10) > 2000) update(); // コーンが近づいたら
        else linetrace_motor_operation(1);
        break;
      case 161:
        stop();
        right_rotation(1);                   // 少し左回転
        delay(550);
        stop();
        update();
      case 162:
        update();
        break;
      case 163:
        read_psd();
        if(analogRead(10) > 3500){          // コーンに接近
          update();
        }else{
          go_to_goal();
        }
        break;
      case 164:
        if(millis()-timer <= 150){
          go_to_goal();
        }else{
          stop();                           // ここでシュートする
          delay(1500);
          update();
        }
        break;
      case 165:
        stop();

      case 300:
        if(millis()-timer >= 1000 && (rightturn || leftturn)){ // 十字路
          forward(1);
          delay(200);
          l_leftangle(2, 1, 90);
          update();
        }else{
          linetrace_motor_operation(1);
          fold_flag();                      // requied
        }
        break;
      case 301:
        if(analogRead(10) > 2000) update(); // コーンが近づいたら
        else linetrace_motor_operation(1);
        break;
      case 302:
        stop();
        right_rotation(1);                   // 少し左回転
        delay(550);
        read_psd();
        while(mpsd <= 1500){
          right_rotation(1); 
          read_psd();
        }
        stop();
        update();
      case 303:
        update();
        break;
      case 304:
        read_psd();
        if(analogRead(10) > 3500){          // コーンに接近
          update();
        }else{
          go_to_goal();
        }
        break;
      case 305:
        if(millis()-timer <= 150){
          go_to_goal();
        }else{
          stop();                           // ここでシュートする
          delay(1500);
          update();
        }
        break;
      case 306:
        stop();


      case 1000:
        // キャリブレーションをするなら必要
        for(int32_t i = 0; i < FOT_NUM; i++){ // 初期化
          maxlight[i] = -32000;
          minlight[i] =  32000;
        }
        configure_initial();
        while(true){
          getangle();
          Serial.print(angle);
          Serial.print("\n");
          delay(10);
        }
        break;

      default:
        stop();
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void update(){
  state++;
  fold_flag();
  timer = millis();
}

// 前進
void forward(int speed){
  right_motor.forward(rightspeed[speed]);
  left_motor.forward(leftspeed[speed]);
}

// 後退
void back(int speed){
  right_motor.back(rightspeed[speed]);
  left_motor.back(leftspeed[speed]);
}

// 右回転
void right_rotation(int speed){
  right_motor.back(rightspeed[speed]);
  left_motor.forward(leftspeed[speed]);
}

// 左回転
void left_rotation(int speed){
  right_motor.forward(rightspeed[speed]);
  left_motor.back(leftspeed[speed]);
}

// 停止(デッドタイム1ms用)
void stop(){
  right_motor.halt();
  left_motor.halt();
  delay(1);
}

void brake(){
  right_motor.brake();
  left_motor.brake();
}

// フラグの初期化
void fold_flag(){
  rightturn = false;
  leftturn  = false;
  lineflag  = true;
}

void read_psd(){
  rpsd = analogRead(9);
  mpsd = analogRead(10);
  lpsd = analogRead(11);

  rpsd = lpf*rpsd+(1-lpf)*prpsd;
  mpsd = lpf*mpsd+(1-lpf)*pmpsd;
  lpsd = lpf*lpsd+(1-lpf)*plpsd;

  prpsd = rpsd;
  pmpsd = mpsd;
  plpsd = lpsd;

  if(rpsd > obs_ad) robs = true;
  else robs = false;
  if(mpsd > obs_ad) mobs = true;
  else mobs = false;
  if(lpsd > obs_ad) lobs = true;
  else lobs = false;
}

int32_t get_direct(){
  read_psd();
  if(robs && mobs && lobs){
    if(pdirection == 2) return 5;
    return 1;
  }
  if(robs && mobs){
    pdirection = 2;
    return 1;
  }
  if(mobs && lobs){
    pdirection = 4;
    return 5;
  }
  if(robs && lobs){
    if(pdirection == 2) return 5;
    return 1;
  }
  if(robs){
    pdirection = 2;
    return 2;
  }
  if(mobs){
    if(pdirection == 2) return 5;
    return 1;
  }
  if(lobs){
    pdirection = 4;
    return 4;
  }
  return 3;
}

void forest(int32_t speed){
  read_psd();
  if(robs || mobs || lobs){
    if(direction%2 == 0){
      stop();
      right_rotation(speed);
      delay(500);
      stop();
      forward(speed);
      delay(900);
      stop();
      left_rotation(speed);
      delay(500);
      stop();
    }else{
      stop();
      left_rotation(speed);
      delay(500);
      stop();
      forward(speed);
      delay(900);
      stop();
      right_rotation(speed);
      delay(500);
      stop();
    }
    direction++;
  }else{
    forward(speed);
  }
}
/* void forest(int32_t speed){ */
/*   direction = get_direct(); */

/*   if(direction != pdirection){ */
/*     stop(); */
/*   } */
/*   if(direction != 3) straight = false; */

/*   switch(direction){ */
/*     case 1: */
/*       left_rotation(speed+1); */
/*       forestslope -= 2; */
/*       break; */
/*     case 2: */
/*       right_motor.forward(rightspeed[speed]); */
/*       left_motor.halt(); */
/*       forestslope--; */
/*       break; */
/*     case 3: */
/*       if(!straight){ */
/*         foresttimer = millis(); */
/*         straight = true; */
/*       }else if(millis()-foresttimer >= 1000){ */
/*         if(forestslope > 0){ */
/*           right_motor.forward(rightspeed[speed]); */
/*           left_motor.halt(); */
/*           forestslope -= 2; */
/*           if(forestslope < 0) forestslope = 0; */
/*         }else if(forestslope < 0){ */
/*           right_motor.halt(); */
/*           left_motor.forward(leftspeed[speed]); */
/*           forestslope += 2; */
/*           if(forestslope > 0) forestslope = 0; */
/*         }else{ */
/*           forward(speed); */
/*         } */
/*       }else{ */
/*         forward(speed); */
/*       } */
/*       break; */
/*     case 4: */
/*       right_motor.halt(); */
/*       left_motor.forward(leftspeed[speed]); */
/*       forestslope++; */
/*       break; */
/*     case 5: */
/*       right_rotation(speed+1); */
/*       break; */
/*   } */
/*   pdirection = direction; */
/* } */

// 右に90度曲がる
void r_rightangle(int32_t maxspeed, int32_t minspeed, int32_t rad){
  stop();                           // デッドタイム
  getangle();
  while(lineflag){                  // 一度ラインが見えなくなるまで回転
    right_rotation(maxspeed);
    /* right_motor.back(rightspeed[maxspeed]); */
    /* left_motor.forward(leftspeed[maxspeed]); */
    getangle();
  }
  getangle();
  delay(10);
  while(!lineflag || angle >= rad){ // 復帰させる
    right_rotation(minspeed);
    /* right_motor.back(rightspeed[minspeed]); */
    /* left_motor.forward(leftspeed[minspeed]); */
    getangle();
  }
  stop();                           // デッドタイム
  fold_flag();
}
void r_rightangle(int32_t maxspeed, int32_t minspeed){
  r_rightangle(maxspeed, minspeed, 240);
}
void r_rightangle(int32_t speed){
  r_rightangle(speed, speed);
}

// 左に90度曲がる
void l_leftangle(int32_t maxspeed, int32_t minspeed, int32_t rad){
  stop();                            // デッドタイム
  getangle();
  while(lineflag){                   // 一度ラインが見えなくなるまで回転
    left_rotation(maxspeed);
    /* right_motor.forward(rightspeed[maxspeed]); */
    /* left_motor.back(leftspeed[maxspeed]); */
    getangle();
  }
  delay(10);
  while(!lineflag || angle <= -rad){ // 復帰させる
    left_rotation(minspeed);
    /* right_motor.forward(rightspeed[minspeed]); */
    /* left_motor.back(leftspeed[minspeed]); */
    getangle();
  }
  stop();                            // デッドタイム
  fold_flag();
}
void l_leftangle(int32_t maxspeed, int32_t minspeed){
  l_leftangle(maxspeed, minspeed, 240);
}
void l_leftangle(int32_t speed){
  l_leftangle(speed, speed);
}

// モーターを動かす
void motor_operation(int32_t speed, int32_t maxspeed){
  int32_t rv, lv;        // 右と左のpwm
  if(manipulation >= 0){ // 右回転
    rv = map(manipulation, 0, 255, rightspeed[speed], rightspeed[0]);
    lv = map(manipulation, 0, 255, leftspeed[speed], leftspeed[maxspeed]);
    rv = constrain(rv, 0, 255);
    lv = constrain(lv, 0, 255);
    right_motor.forward(rv);
    left_motor.forward(lv);
  }else{                 // 左回転
    rv = map(manipulation, 0, -255, rightspeed[speed], rightspeed[maxspeed]);
    lv = map(manipulation, 0, -255, leftspeed[speed], leftspeed[0]);
    rv = constrain(rv, 0, 255);
    lv = constrain(lv, 0, 255);
    right_motor.forward(rv);
    left_motor.forward(lv);
  }
}

// 操作量の計算
void manipulation_calc(){ 
  manipulation = pgain*angle+dgain*(angle-pangle);   // PD
  manipulation = constrain(manipulation, -255, 255); // 安全のために必要
  /* Serial.println(angle-pangle); */
}

// ライントレースをする
void linetrace_motor_operation(int32_t speed, int32_t maxspeed){
  manipulation_calc();
  motor_operation(speed, maxspeed);
}
void linetrace_motor_operation(int32_t speed){
  linetrace_motor_operation(speed, speed);
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
  int32_t maxnum = 0;                 // 場所
  int32_t maxper = light[0];          // 最大の明るさ
  for(int32_t i = 1; i < FOT_NUM; i++){
    if(maxper < light[i]){
      maxper = light[i];
      maxnum = i;
    }
  }

  if(maxper < MIDDLELIGHT) return -1; // どれも明るくない
  return maxnum;
}

// debug用
void show_light(){ 
  for(int32_t i = 0; i < FOT_NUM; i++){
    if(i) Serial.print(",");
    Serial.print(light[i]);
  }
  Serial.print("\n");
}
void show_minmax(){
  Serial.print("max");  Serial.print("\n");
  for(int32_t i = 0; i < FOT_NUM; i++){
    Serial.print(maxlight[i]);
    Serial.print("\n");
  }
  Serial.print("min");  Serial.print("\n");
  for(int32_t i = 0; i < FOT_NUM; i++){
    Serial.print(minlight[i]);
    Serial.print("\n");
  }
  delay(30000);
}

// 角度の更新
void getangle(){                                         
  fot_read();
  normalize();               // 正規化
  brightnum = maxlightnum(); // 最大の明るさ
  pangle = angle;
  if(lightflag[0] && lightflag[7]){
    rightturn = true;
    leftturn = true;
  }else if(lightflag[0] && lightflag[3]){
    leftturn = true;
  }else if(lightflag[4] && lightflag[7]){
    rightturn = true;
  }

  lineflag = false;
  for(int32_t i = 0; i < FOT_NUM; i++){
    if(lightflag[i]){
      lineflag = true;
      break;
    }
  }

  if(brightnum == -1){
    /* if(pbrightnum == 0) angle = -255.0;                        // 最小値 */
    if(pbrightnum == 0) angle = -255;                        // 最小値
  }else{
    /* if(pbrightnum == 0) angle = -255.0;                        // 最小値 */
    if(pbrightnum == 0) angle = -255;                        // 最小値
    else if(brightnum != FOT_NUM-1){
      double cur = abs(light[brightnum+1]-light[brightnum-1]); // 一番明るいところの左右の差
      // {{{ 特殊な操作
      cur = pow(cur, 0.28);

      cur = mymap(cur, 8.0, 24.0, -1.0, 1.0);
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

void go_to_goal(){
  read_psd();
  now = rpsd-lpsd;
  now = map(now, -2000, 2000, -255, 255);
  now = constrain(now, -255, 255);

  now = now*lpf+(1-lpf)*past;

  manipulation = now;
  motor_operation(1, 1);

  past = now;
}

// double型のmap
double mymap(double x, double in_min, double in_max, double out_min, double out_max){ 
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
