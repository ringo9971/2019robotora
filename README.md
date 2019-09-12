# 2019 Robot Triathlon
ロボトラに使用するプログラムです.

ロボトラに参加する人は参考にしてください. 

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
  l_rightangle();

// 操作量の分だけモーターを動かします. linetrace_motor_operation()で使用します
  motor_operation();

// 取ってきた角度からPD制御を用いて操作量を決定します. linetrace_motor_operationで使用します
  manipulation_calc();

// ライントレースします
  linetrace_motor_operation();

// センサの値を読みます. getangle()で使用します
  fort_read();

// センサの値を正規化します. getangle()で使用します
  normalize();

// ラインに一番近いフォトインタラプタの番号を返します. getangle()で使用します
  maxlightnum();

// 現在の角度をとflagを更新します
  getangle();
```
