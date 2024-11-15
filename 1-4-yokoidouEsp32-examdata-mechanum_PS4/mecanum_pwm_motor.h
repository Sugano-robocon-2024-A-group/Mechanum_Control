// 参考：https://rikoubou.hatenablog.com/entry/2017/06/05/172953
#include <Arduino.h>
#include <PS4Controller.h>

//uint32_t ledcSetup(uint8_t channel, uint32_t freq, uint8_t resolution_bits);
//void ledcAttachPin(uint8_t pin, uint8_t channel);
//void ledcWrite(uint8_t channel, uint32_t duty);

/* 使うピンの定義 */
/*GPIO01を右前EN, GPIO22を右前1, GPIO23を右前2
GPIO21を左前EN, GPIO18を左前1, GPIO19を左前2
GPIO13を左後EN,GPIO27を左後1,GPIO14を左後2
GPIO23を右後EN,GPIO32を右後1,GPIO33を右後2*/
// 左前車輪
const int LEFT_FRONT_IN1 = 21;
const int LEFT_FRONT_IN2 = 18;
const int LEFT_FRONT_PWM = 19;

// 左後車輪
const int LEFT_BACK_IN1 = 13;
const int LEFT_BACK_IN2 = 27;
const int LEFT_BACK_PWM = 14;

// 右前車輪
const int RIGHT_FRONT_IN1 = 22;
const int RIGHT_FRONT_IN2 = 3;
const int RIGHT_FRONT_PWM = 21;

// 右後車輪
const int RIGHT_BACK_IN1 = 4;
const int RIGHT_BACK_IN2 = 32;
const int RIGHT_BACK_PWM = 33;

/* チャンネルの定義 */
const int CHANNEL_LEFT_FRONT = 0;
const int CHANNEL_LEFT_BACK = 1;
const int CHANNEL_RIGHT_FRONT = 2;
const int CHANNEL_RIGHT_BACK = 3;

const int LEDC_TIMER_BIT = 8;   // PWMの範囲(8bitなら0〜255、10bitなら0〜1023)
const int LEDC_BASE_FREQ = 490; // 周波数(Hz)
const int VALUE_MAX = 255;      // PWMの最大値

// 関数のプロトタイプ宣言
void frontLeftMotor(double pwm);//doubleにする？？
void backLeftMotor(double pwm);
void frontRightMotor(double pwm);
void backRightMotor(double pwm);
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
  const int VALUE_MAX = pwm;
  double absv=sqrt((PS4.LStickX()^2)+(PS4.LStickY()^2))*(VALUE_MAX/128)*2;
  //if (pwm < VALUE_MAX || pwm > -VALUE_MAX) pwm = VALUE_MAX;
  Serial.printf("%.2lf\n", absv);

  double directions[4] = {Cont_A, Cont_B, -Cont_B, -Cont_A};
  if(absv<5){
    absv=0.0;
    }
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
void frontLeftMotor(double pwm) {
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
void backLeftMotor(double pwm) {
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
void frontRightMotor(double pwm) {
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
void backRightMotor(double pwm) {
  if (pwm >= 0) {
    digitalWrite(RIGHT_BACK_IN1, HIGH);
    digitalWrite(RIGHT_BACK_IN2, LOW);
  } else {
    digitalWrite(RIGHT_BACK_IN1, LOW);
    digitalWrite(RIGHT_BACK_IN2, HIGH);
  }
  ledcWrite(CHANNEL_RIGHT_BACK, abs(pwm) > VALUE_MAX ? VALUE_MAX : abs(pwm));
}
