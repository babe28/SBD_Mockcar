/*  IO関連その他をまとめたファイル
*   mockcar-race-program v0.8
*   io_others.cpp
*
*   checkStartSensor()      //スタートセンサーポーリング
*   stopTimer(int)          //タイマーストップ・時間記録
*   checkRestButton         //リセットボタン確認
*   checkSetupButton
*   isSensorTriggered()
*/

#include "main.hpp"

//割り込み設定ここから
// ゴールセンサー用割り込みでタイマーストップを指示
void IRAM_ATTR goalSensorISR1() {
    stopTimer(0);   //タイマー1を停止
    systemState.race.goalSensors[0].isSense = true;
}
void IRAM_ATTR goalSensorISR2() {
    stopTimer(1);   //タイマー2を停止
    systemState.race.goalSensors[1].isSense = true;
}
void IRAM_ATTR goalSensorISR3() {
    stopTimer(2);   //タイマー3を停止
    systemState.race.goalSensors[2].isSense = true;
}

//スタートセンサーのチェック・ポーリング・チャタリング対策アリ
void checkStartSensor() {
    static bool lastSensorState = HIGH; // 前回のセンサー状態
    static unsigned long lastTriggerTime = 0;

    int sensorState = digitalRead(START_SENS);
    // HIGHからLOWに変化（センサーが押された）
    if (sensorState == LOW && lastSensorState == HIGH) {
        if (millis() - lastTriggerTime > 100) { // 50ms デバウンス
            if(systemState.config.setupMode){           //もし設定画面なら
                lastTriggerTime = millis();
                return;                                 //スタートレースしません。
            }

            startRace(); // レース開始処理を呼び出す
            lastTriggerTime = millis();
        }
    }

    lastSensorState = sensorState; // 状態を更新
}


//割り込み使用に変更から更に戻した
void checkResetButton() {
    static unsigned long lastTriggerTime = 0;
    if(digitalRead(RESET_BUTTON_PIN)==LOW){
        resetFlag = true;
        systemState.race.resetFlag = true;
    }

    if(resetFlag && (millis() - lastTriggerTime > 250)){
        systemState.buttons[0].isPressed = false;
        resetFlag = false;
        //Serial.println("[DEBUG] checkResetButton executed.\n Detect Check Button.");
        resetRaceState();
        systemState.race.startSensor.isActive = false;
        drawIdleScreen();
        delay(10);
        lastTriggerTime = millis();
        return;
    }
}

void checkReadyButton(){
    static bool lastButtonState = HIGH;
    static unsigned long lastTriggerTime = 0;
    int buttonState = digitalRead(START_BUTTON_PIN);
    if(buttonState == LOW && lastButtonState == HIGH){
      if(millis() - lastTriggerTime > 150){
        if(systemState.config.setupMode || systemState.race.raceFlag){
          lastTriggerTime = millis();
          return;
        }

        if(!systemState.race.bgmFlag){
          playMP3(0); //曲が終わって再生が止まるとbgmFlagがtrueのままで再生できなくなるから、対処すべし
        }
        systemState.race.signalDrawing = true;
        lastTriggerTime = millis();
      }
    }

}


/***************************************************
 * I/O関連
**************************************************** */
void eeprom_initialize(){
  //設定保存用（EEPROMの後継ライブラリPreferences）
  bool doesExist;
  String settings = "none";                   //ボード設定記録用（modern,legacy)
  Preferences preferences;                    //ここから読み書きルーチン
  preferences.begin("my_settings",false); 

  doesExist = preferences.isKey("on_cycle");   //設定があるかどうか確認
  if(!doesExist){
    /* 初期起動のときはここが実行される */
    on_cycle = 1;
    preferences.putUInt("on_cycle", on_cycle);//起動回数書き込み 9999回超えたらリセットしたほうがいいな
    preferences.putString("settings","none");
    Serial.println("First Load Initialize");
  }
  else {
    //２回目以降の起動のときはここ実行
    //best_time_onboard = preferences.getFloat("besttime");
    on_cycle = preferences.getUInt("on_cycle");
    on_cycle++;
    Serial.println("Load Initialize");
    Serial.printf("Cycle:%d \n",on_cycle);
    preferences.putUInt("on_cycle", on_cycle);//起動回数書き込み
    
    if(on_cycle > 30000){ //intの限界を超えないようにリセット
      on_cycle = 1;
      preferences.putUInt("on_cycle",on_cycle);
    }
  }
  preferences.end();//preferences終了

  Serial.printf("Board Boot Counter:%d \n",on_cycle);
  delay(100);                           // delay
}

void memory_wirte(){
  Preferences preferences;
  preferences.begin("my_settings",RW_MODE); //２番目の引数が省略・・・読み書きモード
}

//Bluetoothへ
void sendBluetoothData() {

}


//センサーをポーリング  ここはセンサーの値を取得するだけ
bool isSensorTriggered() {
    return false;
}

// ゴールセンサーの割り込みを無効化 今のところ使っていない
void disableGoalSensorInterrupts() {
    detachInterrupt(digitalPinToInterrupt(GOAL_SENS_1));
    detachInterrupt(digitalPinToInterrupt(GOAL_SENS_2));
    detachInterrupt(digitalPinToInterrupt(GOAL_SENS_3));
}

void enableGoalSensorInterrupts() {
    attachInterrupt(digitalPinToInterrupt(GOAL_SENS_1), goalSensorISR1, FALLING);
    attachInterrupt(digitalPinToInterrupt(GOAL_SENS_2), goalSensorISR2, FALLING);
    attachInterrupt(digitalPinToInterrupt(GOAL_SENS_3), goalSensorISR3, FALLING);
}



