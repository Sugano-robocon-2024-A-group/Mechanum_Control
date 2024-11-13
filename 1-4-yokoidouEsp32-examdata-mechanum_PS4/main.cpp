#include <Arduino.h>
#include <PS4Controller.h>
#define ESP32_RTOS
#include "OTA.h"
#include "mecanum_pwm_motor.h"

double Cont_A = 0.0;
double Cont_B = 0.0;
int PS4_LStickX = 0;
int PS4_LStickY = 0;


void setup() {
  Serial.begin(115200);
  Serial.begin(115200);
  PS4.begin("1c:69:20:e6:20:d2"); // PS4コントローラのMACアドレスを指定
  Serial.println("Booting");
  setupOTA("Mecanum_Robot", "SSID", "PASSWORD");
  // Your setup code

  init_pwm_setup();

  Serial.println("Ready.");//準備完了

}

void loop() {
  int pwm = 255; // max = 255
  int move_time = 100; //ms

  //これでLスティックを使った形。斜め移動を取り入れたver.
  //PS4.LStickX(),PS4.LStickY(),PS4.RStickX(),PS4.RStickY()がカーソルキー
  
if (PS4.isConnected()) {
  //ここで恐らくは「start」のコードが入ってくる

  //
  CalculateCont(Cont_A, Cont_B);
  //これで動かす
  LetsMoveMoter(pwm, Cont_A, Cont_B);
  delay(move_time);
}
}
