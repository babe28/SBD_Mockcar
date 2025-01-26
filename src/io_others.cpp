/*  IO関連その他をまとめたファイル
*   mockcar-race-program v0.2
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

// リセットボタン用割り込み
void IRAM_ATTR handleResetButton() {
    systemState.buttons[0].isPressed = true; // リセットボタンが押されたフラグをセット
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

void stopTimer(int timerId) {
    //割り込みで来る
    //該当タイマー番号のタイマーガストップしたことを記録する関数
    Timer &timer = systemState.race.timers[timerId];
    Sensor &goal_sensor = systemState.race.goalSensors[timerId];

    if (timer.isTiming) {//タイマーが動いていたら
        timer.stopTime = millis() - systemState.race.startTime; // startTimeから現在時刻を引いて、ストップ時間（つまり経過時間）を記録
        timer.isTiming = false;                         //タイマー稼働中フラグを停止
        goal_sensor.isActive = true;                    //ゴールセンサーアクティブフラグ        
        //Serial.printf("[DEBUG] Timer %d stopped at %lu ms\n", timerId + 1, timer.stopTime);
    }
}




//割り込み使用に変更
void checkResetButton() {
    static unsigned long lastTriggerTime = 0;
    if(systemState.buttons[0].isPressed && (millis() - lastTriggerTime > 200)){
        systemState.buttons[0].isPressed = false;
        // Serial.println("[DEBUG] checkResetButton executed.\n Detect Check Button.");
        resetRaceState();
        systemState.race.startSensor.isActive = false;
        drawIdleScreen();
        lastTriggerTime = millis();
        return;
    }
}

//センサーをポーリング  ここはセンサーの値を取得するだけ
bool isSensorTriggered() {
    return true;
}

// -----------------------
// 設定ボタン・上・下ボタンのみここでスキャン
// -----------------------
void updateButtonStates() {
    unsigned long currentTime = millis();

    /* for (int i = 0; i < 3; i++) { // ボタンが3つの場合
        bool currentState = digitalRead(buttonPins[i]) == LOW; // LOWで押下状態

        if (currentState) {
            // ボタンが新たに押された場合
            if (!buttonStates[i].isPressed) {
                buttonStates[i].pressStartTime = currentTime;
                buttonStates[i].isPressed = true;
                buttonStates[i].isLongPressed = false; // 初期化
            }

            // 長押し判定
            if (currentTime - buttonStates[i].pressStartTime > LONG_PRESS_THR) {
                buttonStates[i].isLongPressed = true;
            }
        } else {
            // ボタンが離された場合
            if (buttonStates[i].isPressed) {
                // 短押し・長押しの判定
                if (buttonStates[i].isLongPressed) {
                    Serial.printf("Button %d was long-pressed.\n", i);
                } else {
                    Serial.printf("Button %d was short-pressed.\n", i);
                }
            }

            // 状態リセット
            buttonStates[i].isPressed = false;
            buttonStates[i].isLongPressed = false;
        }
    }*/
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

void Analyze_IR(){

  uint8_t receive_data[64],byte_count=0;
  size_t rxSize = 0;
  rmt_data_t *item = (rmt_data_t *)xRingbufferReceive(IRbuffer, &rxSize,2000);

  if(item){
    uint16_t t_base = item[0].duration0/8;
    //Serial.printf("%d items received.\n",rxSize/sizeof(rmt_data_t));
    //Serial.printf("T = %d\n",t_base);

    //Serial.printf("#  :{dur0,L,dur1,L} duty   :bit\n");
    for(uint16_t i=0;i<rxSize/sizeof(rmt_data_t); i++){
      uint8_t byte_data;
      float duty = (float)item[i].duration1 / item[i].duration0;
      int8_t bit0 = 1;
      if((duty >= 0.7) && (duty <= 1.3)){
          bit0=0;
      }
      else if((duty >= 2.1) &&(duty <= 3.6)){
          bit0 = 1;
      }
      //Serial.printf("%3d : {%4d,1,%4d,0} %4.2f : %2d \n",i,item[i].duration0,item[i].duration1,duty,bit0);

      if(i == 0) continue;
      if(bit0 < 0){
        if(item[i].duration1 !=0){
          Serial.printf("Receive illegular Signal.\n");
        }
        break;
      }

    uint16_t bit_possition = (i-1) % 8;
    if(bit_possition == 0){
      byte_data = 0;
    }
    byte_data += bit0 << bit_possition;
      if(bit_possition == 7){
        receive_data[byte_count++] = byte_data;
        byte_data = 0;
      }
    }

    Serial.printf("Decode data:");
    for(uint8_t i=0; i<byte_count; i++){
      Serial.printf("%02X \n",receive_data[i]);
    }

        // 一致するパターンを判定
        if (byte_count >= 4) {  // パターン比較に必要なデータがある場合のみ
            if (receive_data[0] == 0xEE && receive_data[1] == 0x87 &&
                receive_data[2] == 0x04 && receive_data[3] == 0x65) {
                printf("[IR] Execute\n");
            }
            if (receive_data[0] == 0xEE && receive_data[1] == 0x87 &&
                receive_data[2] == 0x08 && receive_data[3] == 0x65) {
                printf("[IR] LEFT BUTTON\n");
            }
            if(receive_data[0] == 0xEE && receive_data[1] == 0x87 &&
                receive_data[2] == 0x07 && receive_data[3] == 0x65) {
                printf("[IR] RIGHT BUTTONn\n");
            }
            if(receive_data[0] == 0xEE && receive_data[1] == 0x87 &&
                receive_data[2] == 0x0B && receive_data[3] == 0x65) {
                    printf("[IR] UP BUTTON\n");
            }
            if(receive_data[0] == 0xEE && receive_data[1] == 0x87 &&
                receive_data[2] == 0x0D && receive_data[3] == 0x65) {
                    printf("[IR] DOWN BUTTON\n");
            } 
         }else {
            printf("[IR] Insufficient data for pattern matching\n");
        }
    printf("\n");
// MENU BUTTON
//EE 87 02 65
//saisei buttton
//EE 87 04 65
//再生と決定ボタンは２回送信されている]
//押しっぱなしでリピート信号が出る
    vRingbufferReturnItem(IRbuffer, (void*) item);
  }//if(item)end

}//RECEIVEIR end
