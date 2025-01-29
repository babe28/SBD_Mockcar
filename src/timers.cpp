/* timers.cpp 
* タイマー関連をまとめたファイル
*
*　stopTimer
*
*/

#include "main.hpp"

void stopTimer(int timerId) {
    //割り込みで来る
    //該当タイマー番号のタイマーガストップしたことを記録する関数
    //isActive = アクティブになりましたよ記録（タイマーが３つ全部止まるまでクリアしない）
    //isSense = 割り込みでtrueタイマー止めたらfalseに。
    Timer &timer = systemState.race.timers[timerId];
    Sensor &goal_sensor = systemState.race.goalSensors[timerId];

    if (timer.isTiming) {//タイマーが動いていたら
        timer.stopTime = millis() - systemState.race.startTime; // startTimeから現在時刻を引いて、ストップ時間（つまり経過時間）を記録
        timer.isTiming = false;                         //タイマー稼働中フラグを停止
        goal_sensor.isActive = true;                    //ゴールセンサーアクティブフラグ      
        Serial.printf("[DEBUG] Timer %d stopped at %lu ms\n", timerId + 1, timer.stopTime);
    }
}

void resetTimers() {
    systemState.race.startTime = 0;                   //スタート時間クリア
    systemState.race.raceFlag = false;                //レース中ではない
    systemState.race.goalCount = 0;                   //ゴールカウントクリア
    systemState.race.startSensor.isActive = false;    //スタートセンサー非アクティブ
    systemState.race.startSensor.lastTriggerTime = 0;

    for (int i = 0; i < 3; i++) {
        Timer &timer = systemState.race.timers[i];
        timer.stopTime = 0;
        timer.isTiming = false;
    }
    if(SerialDebug){
      Serial.println("[DEBUG] Timers and race state reset!");
    }
}