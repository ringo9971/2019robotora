# 2019 Robot Triathlon

[2019ロボットトライアストン](http://www.robot-triathlon.org/2019/index.html)に使用するプログラムです.

回路部品
* [Arduino Due](https://store.arduino.cc/usa/due)
* [モータードライバ](http://akizukidenshi.com/catalog/g/gK-10721/)
* [フォトカプラ](http://akizukidenshi.com/catalog/g/gI-07690/)
* [NPNトランジスタ](http://akizukidenshi.com/catalog/g/gI-04268/)
* [PNPトランジスタ](http://akizukidenshi.com/catalog/g/gI-02612/) 

フォトセンサは某先輩からもらったもので, 型番はわかりません.
1個40円位の安いやつだと言ってました.

部で使われているArduino Nanoとは違い, Arduino Dueを使用しています. 

なので, Nanoで使うことは出来ないので注意してください. 

# Function
```arduino
// switch-caseの更新に使います
  update();

// 前進
  forward();

// 後退
  back();

// 右回転
  right_rotation();

// 左回転
  left_rotation();

// Hブリッジのデッドタイム用に使用します
  stop();

// フラグの初期化
  fold_flag();

// ラインに沿って右に曲がります. 十字路でも使用できます
  r_rightangle();

// ラインに沿って左に曲がります. 十字路でも使用できます
  l_leftangle();

// 操作量の分だけモーターを動かします. linetrace_motor_operation()で使用します
  motor_operation();

// 取ってきた角度からPD制御を用いて操作量を決定します. linetrace_motor_operation()で使用します
  manipulation_calc();

// ライントレースします
  linetrace_motor_operation();

// setup()でnormalize()に必要なセンサの最大値と最小値を更新します. 今回のプログラムでは使用していません
  configure_initial();

// センサの値を読みます. getangle()で使用します
  fort_read();

// センサの最大値と最小値を更新します. configure_initial()で使用します
  getminmax();

// センサの値を正規化します. getangle()で使用します
  normalize();

// ラインに一番近いフォトインタラプタの番号を返します. getangle()で使用します
  maxlightnum();

// 現在の角度とflagを更新します
  getangle();
```
