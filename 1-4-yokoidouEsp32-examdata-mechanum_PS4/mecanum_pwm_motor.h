// 参考：https://rikoubou.hatenablog.com/entry/2017/06/05/172953
#include <Arduino.h>
#include <PS4Controller.h>

//uint32_t ledcSetup(uint8_t channel, uint32_t freq, uint8_t resolution_bits);
//void ledcAttachPin(uint8_t pin, uint8_t channel);
//void ledcWrite(uint8_t channel, uint32_t duty);

/* 使うピンの定義 */
// 左前車輪
const int LEFT_FRONT_IN1 = 34;
const int LEFT_FRONT_IN2 = 35;
const int LEFT_FRONT_PWM = 33;

// 左後車輪
const int LEFT_BACK_IN1 = 26;
const int LEFT_BACK_IN2 = 27;
const int LEFT_BACK_PWM = 13;

// 右前車輪
const int RIGHT_FRONT_IN1 = 22;
const int RIGHT_FRONT_IN2 = 3;
const int RIGHT_FRONT_PWM = 21;

// 右後車輪
const int RIGHT_BACK_IN1 = 4;
const int RIGHT_BACK_IN2 = 0;
const int RIGHT_BACK_PWM = 15;

/* チャンネルの定義 */
const int CHANNEL_LEFT_FRONT = 0;
const int CHANNEL_LEFT_BACK = 1;
const int CHANNEL_RIGHT_FRONT = 2;
const int CHANNEL_RIGHT_BACK = 3;

const int LEDC_TIMER_BIT = 8;   // PWMの範囲(8bitなら0〜255、10bitなら0〜1023)
const int LEDC_BASE_FREQ = 490; // 周波数(Hz)
const int VALUE_MAX = 255;      // PWMの最大値

// 関数のプロトタイプ宣言
void frontLeftMotor(int32_t pwm);
void backLeftMotor(int32_t pwm);
void frontRightMotor(int32_t pwm);
void backRightMotor(int32_t pwm);
void brake();
void coast();
void LetsMoveMoter(int32_t pwm, double Cont_A, double Cont_B);
void CalculateCont(double &Cont_A, double &Cont_B);

void init_pwm_setup() {
  // 各ピンを出力モードに設定
  pinMode(LEFT_FRONT_IN1, OUTPUT);
  pinMode(LEFT_FRONT_IN2, OUTPUT);
  pinMode(LEFT_BACK_IN1, OUTPUT);
  pinMode(LEFT_BACK_IN2, OUTPUT);
  pinMode(RIGHT_FRONT_IN1, OUTPUT);
  pinMode(RIGHT_FRONT_IN2, OUTPUT);
  pinMode(RIGHT_BACK_IN1, OUTPUT);
  pinMode(RIGHT_BACK_IN2, OUTPUT);

  // 各チャンネルに対してPWMの設定
  ledcSetup(CHANNEL_LEFT_FRONT, LEDC_BASE_FREQ, LEDC_TIMER_BIT);
  ledcSetup(CHANNEL_LEFT_BACK, LEDC_BASE_FREQ, LEDC_TIMER_BIT);
  ledcSetup(CHANNEL_RIGHT_FRONT, LEDC_BASE_FREQ, LEDC_TIMER_BIT);
  ledcSetup(CHANNEL_RIGHT_BACK, LEDC_BASE_FREQ, LEDC_TIMER_BIT);

  // 各PWMピンをチャンネルにアタッチ
  ledcAttachPin(LEFT_FRONT_PWM, CHANNEL_LEFT_FRONT);
  ledcAttachPin(LEFT_BACK_PWM, CHANNEL_LEFT_BACK);
  ledcAttachPin(RIGHT_FRONT_PWM, CHANNEL_RIGHT_FRONT);
  ledcAttachPin(RIGHT_BACK_PWM, CHANNEL_RIGHT_BACK);
}

//
void LetsMoveMoter(int32_t pwm, double Cont_A, double Cont_B) {
  const int VALUE_MAX = 255;
  double absv=sqrt((PS4.LStickX()^2)+(PS4.LStickY()^2))*(VALUE_MAX/128);
  if (pwm > VALUE_MAX || pwm < -VALUE_MAX) pwm = VALUE_MAX;

  double directions[4] = {Cont_A, Cont_B, -Cont_B, -Cont_A};
  frontLeftMotor(absv * directions[0]);
  frontRightMotor(absv * directions[1]);
  backRightMotor(absv * directions[2]);
  backLeftMotor(absv * directions[3]);
  //これで呼び出せているはず
  //1が正典、-1が反転
}

void CalculateCont(double &Cont_A, double &Cont_B) {
  int PS4_LStickX = PS4.LStickX();
  int PS4_LStickY = PS4.LStickY();

  double absv=sqrt((PS4_LStickX^2)+(PS4_LStickY^2));

  Serial.printf("Left Stick x at %d\n", PS4_LStickX);
  Serial.printf("Left Stick y at %d\n", PS4_LStickY);

  // 調整の部分
  double cos_theta = PS4_LStickX / (absv);//Vxの向き成分
  double sin_theta = PS4_LStickY / (absv);//Vyの向き成分（絶対値かけたら速度。VALUW＿MAXに絶対値を入れ混む⇒小さくなりすぎる）

  Cont_A=-(cos_theta+sin_theta)/sqrt(2);
  Cont_B=-(cos_theta-sin_theta)/sqrt(2);
  //
}

//この先は各車輪ごとの制御

// 左前車輪の制御
void frontLeftMotor(int32_t pwm) {
  if (pwm >= 0) {
    digitalWrite(LEFT_FRONT_IN1, HIGH);
    digitalWrite(LEFT_FRONT_IN2, LOW);
  } else {
    digitalWrite(LEFT_FRONT_IN1, LOW);
    digitalWrite(LEFT_FRONT_IN2, HIGH);
  }
  ledcWrite(CHANNEL_LEFT_FRONT, abs(pwm) > VALUE_MAX ? VALUE_MAX : abs(pwm));
}

// 左後車輪の制御
void backLeftMotor(int32_t pwm) {
  if (pwm >= 0) {
    digitalWrite(LEFT_BACK_IN1, HIGH);
    digitalWrite(LEFT_BACK_IN2, LOW);
  } else {
    digitalWrite(LEFT_BACK_IN1, LOW);
    digitalWrite(LEFT_BACK_IN2, HIGH);
  }
  ledcWrite(CHANNEL_LEFT_BACK, abs(pwm) > VALUE_MAX ? VALUE_MAX : abs(pwm));
}

// 右前車輪の制御
void frontRightMotor(int32_t pwm) {
  if (pwm >= 0) {
    digitalWrite(RIGHT_FRONT_IN1, HIGH);
    digitalWrite(RIGHT_FRONT_IN2, LOW);
  } else {
    digitalWrite(RIGHT_FRONT_IN1, LOW);
    digitalWrite(RIGHT_FRONT_IN2, HIGH);
  }
  ledcWrite(CHANNEL_RIGHT_FRONT, abs(pwm) > VALUE_MAX ? VALUE_MAX : abs(pwm));
}

// 右後車輪の制御
void backRightMotor(int32_t pwm) {
  if (pwm >= 0) {
    digitalWrite(RIGHT_BACK_IN1, HIGH);
    digitalWrite(RIGHT_BACK_IN2, LOW);
  } else {
    digitalWrite(RIGHT_BACK_IN1, LOW);
    digitalWrite(RIGHT_BACK_IN2, HIGH);
  }
  ledcWrite(CHANNEL_RIGHT_BACK, abs(pwm) > VALUE_MAX ? VALUE_MAX : abs(pwm));
}

/*
#include <Arduino.h>
#include <PS4Controller.h>

//uint32_t    ledcSetup(uint8_t channel, uint32_t freq, uint8_t resolution_bits);
//void        ledcAttachPin(uint8_t pin, uint8_t channel);
//void        ledcWrite(uint8_t channel, uint32_t duty);


使うピンの定義
// Left
const int LEFT_IN1 = 23;
const int LEFT_IN2 = 22;
const int LEFT_IN3 = 1;
const int LEFT_IN4 = 3;
// Right
const int RIGHT_IN1 = 4;
const int RIGHT_IN2 = 16;
const int RIGHT_IN3 = 15;
const int RIGHT_IN4 = 2;*/

/* チャンネルの定義 
const int CHANNEL_0 = 0;
const int CHANNEL_1 = 1;
const int CHANNEL_2 = 2;
const int CHANNEL_3 = 3;
const int CHANNEL_4 = 4;
const int CHANNEL_5 = 5;
const int CHANNEL_6 = 6;
const int CHANNEL_7 = 7;

const int LEDC_TIMER_BIT = 8;   // PWMの範囲(8bitなら0〜255、10bitなら0〜1023)
const int LEDC_BASE_FREQ = 490; // 周波数(Hz)
const int VALUE_MAX = 255;      // PWMの最大値

bool positive = false;

// int directions[4];    // タイヤの回転方向を格納（f-Left, f-Right, b-Left, b-Right）

// 関数のプロトタイプ宣言
void frontLeftMotor(int32_t pwm);
void backLeftMotor(int32_t pwm);
void frontRightMotor(int32_t pwm);
void backRightMotor(int32_t pwm);

void forward(int32_t pwm);
void backward(int32_t pwm);
void right(int32_t pwm);
void left(int32_t pwm);
void rightForward(int32_t pwm);
void leftForward(int32_t pwm);
void rightBackward(int32_t pwm);
void leftBackward(int32_t pwm);
void clockWise(int32_t pwm);
void counterClockWise(int32_t pwm);

void brake();
void coast();
void LetsMoveMoter(int32_t pwm, double Cont_A, double Cont_B);
void CalculateCont(double &Cont_A, double &Cont_B);


//
void LetsMoveMoter(int32_t pwm, double Cont_A, double Cont_B) {
  const int VALUE_MAX = 255;
  double absv=sqrt((PS4.LStickX()^2)+(PS4.LStickY()^2))*(VALUE_MAX/128);
  if (pwm > VALUE_MAX || pwm < -VALUE_MAX) pwm = VALUE_MAX;

  double directions[4] = {Cont_A, Cont_B, -Cont_B, -Cont_A};
  frontLeftMotor(absv * directions[0]);
  frontRightMotor(absv * directions[1]);
  backRightMotor(absv * directions[2]);
  backLeftMotor(absv * directions[3]);
  //これで呼び出せているはず
  //1が正典、-1が反転
}

void CalculateCont(double &Cont_A, double &Cont_B) {
  int PS4_LStickX = PS4.LStickX();
  int PS4_LStickY = PS4.LStickY();

  double absv=sqrt((PS4_LStickX^2)+(PS4_LStickY^2));

  Serial.printf("Left Stick x at %d\n", PS4_LStickX);
  Serial.printf("Left Stick y at %d\n", PS4_LStickY);

  // 調整の部分
  double cos_theta = PS4_LStickX / (absv);//Vxの向き成分
  double sin_theta = PS4_LStickY / (absv);//Vyの向き成分（絶対値かけたら速度。VALUW＿MAXに絶対値を入れ混む⇒小さくなりすぎる）

  Cont_A=-(cos_theta+sin_theta)/sqrt(2);
  Cont_B=-(cos_theta-sin_theta)/sqrt(2);
  //
}
//

void init_pwm_setup() {
  pinMode(LEFT_IN1, OUTPUT); // IN1
  pinMode(LEFT_IN2, OUTPUT); // IN2
  pinMode(LEFT_IN3, OUTPUT); // IN1
  pinMode(LEFT_IN4, OUTPUT); // IN2
  pinMode(RIGHT_IN1, OUTPUT); // IN1
  pinMode(RIGHT_IN2, OUTPUT); // IN2
  pinMode(RIGHT_IN3, OUTPUT); // IN1
  pinMode(RIGHT_IN4, OUTPUT); // IN2

  // ピンのセットアップ ledcSetup
  ledcSetup(CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_BIT);
  ledcSetup(CHANNEL_1, LEDC_BASE_FREQ, LEDC_TIMER_BIT);
  ledcSetup(CHANNEL_2, LEDC_BASE_FREQ, LEDC_TIMER_BIT);
  ledcSetup(CHANNEL_3, LEDC_BASE_FREQ, LEDC_TIMER_BIT);
  ledcSetup(CHANNEL_4, LEDC_BASE_FREQ, LEDC_TIMER_BIT);
  ledcSetup(CHANNEL_5, LEDC_BASE_FREQ, LEDC_TIMER_BIT);
  ledcSetup(CHANNEL_6, LEDC_BASE_FREQ, LEDC_TIMER_BIT);
  ledcSetup(CHANNEL_7, LEDC_BASE_FREQ, LEDC_TIMER_BIT);

  // ピンのチャンネルをセット ledcAttachPin⇒ledAttach
  ledcAttachPin(LEFT_IN1, CHANNEL_0);
  ledcAttachPin(LEFT_IN2, CHANNEL_1);
  ledcAttachPin(LEFT_IN3, CHANNEL_2);
  ledcAttachPin(LEFT_IN4, CHANNEL_3);
  ledcAttachPin(RIGHT_IN1, CHANNEL_4);
  ledcAttachPin(RIGHT_IN2, CHANNEL_5);
  ledcAttachPin(RIGHT_IN3, CHANNEL_6);
  ledcAttachPin(RIGHT_IN4, CHANNEL_7);
}

// 左前車輪
void frontLeftMotor(int32_t pwm) {//値が入ってくる
  if (pwm>=0){
    positive = true; //正回転
  }
  else{
    positive = false;//負回転
  }
  if (pwm > VALUE_MAX || pwm < -VALUE_MAX) {
    pwm = VALUE_MAX;
  }

  if (positive) {
    ledcWrite(CHANNEL_0, abs(pwm));
    ledcWrite(CHANNEL_1, 0);
  }
  else{
      ledcWrite(CHANNEL_0, 0);
      ledcWrite(CHANNEL_1, abs(pwm));
  }
}

// 左後車輪
void backLeftMotor(int32_t pwm) {
  if (pwm>=0) positive = true; else positive = false;
  if (pwm > VALUE_MAX || pwm < -VALUE_MAX) pwm = VALUE_MAX;

  if (positive) {
    ledcWrite(CHANNEL_2, abs(pwm));
    ledcWrite(CHANNEL_3, 0);
  }
  else{
      ledcWrite(CHANNEL_2, 0);
      ledcWrite(CHANNEL_3, abs(pwm));
  }
}

// 右前車輪
void frontRightMotor(int32_t pwm) {
  if (pwm>=0){
    positive = true; 
  }
  else{
    positive = false;
  }
  if (pwm > VALUE_MAX || pwm < -VALUE_MAX) {
    pwm = VALUE_MAX;
  }

  if (positive) {
    ledcWrite(CHANNEL_4, abs(pwm));
    ledcWrite(CHANNEL_5, 0);
  }
  else{
      ledcWrite(CHANNEL_4, 0);
      ledcWrite(CHANNEL_5, abs(pwm));
  }
}

// 右後車輪
void backRightMotor(int32_t pwm) {
  if (pwm>=0) positive = true; else positive = false;
  if (pwm > VALUE_MAX || pwm < -VALUE_MAX) pwm = VALUE_MAX;

  if (positive) {
    ledcWrite(CHANNEL_6, abs(pwm));
    ledcWrite(CHANNEL_7, 0);
  }
  else{
      ledcWrite(CHANNEL_6, 0);
      ledcWrite(CHANNEL_7, abs(pwm));
  }
}


/*

// 前進
void forward(int32_t pwm) {
  if (pwm > VALUE_MAX || pwm < -VALUE_MAX) pwm = VALUE_MAX;

  int directions[4] = {1, 1, 1, 1};

  frontLeftMotor  (pwm * directions[0]);
  frontRightMotor (pwm * directions[1]);
  backRightMotor  (pwm * directions[2]);
  backLeftMotor   (pwm * directions[3]);
}

// 後退
void backward(int32_t pwm) {
  if (pwm > VALUE_MAX || pwm < -VALUE_MAX) pwm = VALUE_MAX;
  
  int directions[4] = {-1, -1, -1, -1};

  frontLeftMotor  (pwm * directions[0]);
  frontRightMotor (pwm * directions[1]);
  backRightMotor  (pwm * directions[2]);
  backLeftMotor   (pwm * directions[3]);

}

// 右
void right(int32_t pwm) {
  if (pwm > VALUE_MAX || pwm < -VALUE_MAX) pwm = VALUE_MAX;
  
  int directions[4] = {1, -1, 1, -1};

  frontLeftMotor  (pwm * directions[0]);
  frontRightMotor (pwm * directions[1]);
  backRightMotor  (pwm * directions[2]);
  backLeftMotor   (pwm * directions[3]);

}

// 左
void left(int32_t pwm) {
  if (pwm > VALUE_MAX || pwm < -VALUE_MAX) pwm = VALUE_MAX;
  
  int directions[4] = {-1, 1, -1, 1};

  frontLeftMotor  (pwm * directions[0]);
  frontRightMotor (pwm * directions[1]);
  backRightMotor  (pwm * directions[2]);
  backLeftMotor   (pwm * directions[3]);

}

// 右前
void rightForward(int32_t pwm) {
  if (pwm > VALUE_MAX || pwm < -VALUE_MAX) pwm = VALUE_MAX;
  
  int directions[4] = {1, 0, 1, 0};

  frontLeftMotor  (pwm * directions[0]);
  frontRightMotor (pwm * directions[1]);
  backRightMotor  (pwm * directions[2]);
  backLeftMotor   (pwm * directions[3]);

}

// 左前
void leftForward(int32_t pwm) {
  if (pwm > VALUE_MAX || pwm < -VALUE_MAX) pwm = VALUE_MAX;
  
  int directions[4] = {0, 1, 0, 1};

  frontLeftMotor  (pwm * directions[0]);
  frontRightMotor (pwm * directions[1]);
  backRightMotor  (pwm * directions[2]);
  backLeftMotor   (pwm * directions[3]);

}

// 右後
void rightBackward(int32_t pwm) {
  if (pwm > VALUE_MAX || pwm < -VALUE_MAX) pwm = VALUE_MAX;
  
  int directions[4] = {0, -1, 0, -1};

  frontLeftMotor  (pwm * directions[0]);
  frontRightMotor (pwm * directions[1]);
  backRightMotor  (pwm * directions[2]);
  backLeftMotor   (pwm * directions[3]);

}

// 左後
void leftBackward(int32_t pwm) {
  if (pwm > VALUE_MAX || pwm < -VALUE_MAX) pwm = VALUE_MAX;
  
  int directions[4] = {-1, 0, -1, 0};

  frontLeftMotor  (pwm * directions[0]);
  frontRightMotor (pwm * directions[1]);
  backRightMotor  (pwm * directions[2]);
  backLeftMotor   (pwm * directions[3]);

}
// 右旋回
void clockWise(int32_t pwm) {
  if (pwm > VALUE_MAX || pwm < -VALUE_MAX) pwm = VALUE_MAX;
  
  int directions[4] = {1, -1, -1, 1};

  frontLeftMotor  (pwm * directions[0]);
  frontRightMotor (pwm * directions[1]);
  backRightMotor  (pwm * directions[2]);
  backLeftMotor   (pwm * directions[3]);

}

// 左旋回
void counterClockWise(int32_t pwm) {
  if (pwm > VALUE_MAX || pwm < -VALUE_MAX) pwm = VALUE_MAX;
  
  int directions[4] = {-1, 1, 1, -1};

  frontLeftMotor  (pwm * directions[0]);
  frontRightMotor (pwm * directions[1]);
  backRightMotor  (pwm * directions[2]);
  backLeftMotor   (pwm * directions[3]);

}*/


//ブレーキ機能、どうする？
/*
// ブレーキ
void brake() {
  ledcWrite(CHANNEL_0, VALUE_MAX);
  ledcWrite(CHANNEL_1, VALUE_MAX);
  ledcWrite(CHANNEL_2, VALUE_MAX);
  ledcWrite(CHANNEL_3, VALUE_MAX);
  ledcWrite(CHANNEL_4, VALUE_MAX);
  ledcWrite(CHANNEL_5, VALUE_MAX);
  ledcWrite(CHANNEL_6, VALUE_MAX);
  ledcWrite(CHANNEL_7, VALUE_MAX);
}

// 空転
void coast() {
  ledcWrite(CHANNEL_0, 0);
  ledcWrite(CHANNEL_1, 0);
  ledcWrite(CHANNEL_2, 0);
  ledcWrite(CHANNEL_3, 0);
  ledcWrite(CHANNEL_4, 0);
  ledcWrite(CHANNEL_5, 0);
  ledcWrite(CHANNEL_6, 0);
  ledcWrite(CHANNEL_7, 0);
}*/
