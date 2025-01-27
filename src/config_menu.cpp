/*  設定画面関連
*   mockcar-race-program v0.2
*   graphic.cpp
*/

#include "main.hpp"
void handleConfigMenu() {
    static bool isConfigScreenInitialized = false; // 初期化済みかどうかを記録
    if (!isConfigScreenInitialized) {
        gfx.fillScreen(TFT_BLACK);
                //初回クリア
        isConfigScreenInitialized = true; // 初期化フラグを設定
    }
    unsigned long currentTime = millis();   //描画タイム用
    static bool menuSelected = false;
    //clearDisplay();                     //メイン画面だけ初期化
    setFontNormal();
    // ヘッダー表示
    printCentering(0,3,"= = = = = = = SETUP MODE = = = = = = =");
    gfx.setCursor(8, 20);
    gfx.setTextColor(TFT_WHITE,TFT_DARKCYAN);
    gfx.setTextSize(1);
    gfx.printf("SENSOR_STATE(S:%d)(G1:%d)(G2:%d)(G3:%d)",digitalRead(START_SENS),digitalRead(GOAL_SENS_1),digitalRead(GOAL_SENS_2),digitalRead(GOAL_SENS_3));

    // メニュー表示(１文字８x１６なので８の倍数で設定するときれい)
    int baseX = 18;
    int baseY = 40; //16,32,48,64,80,96,112,128,144,160,176,192

    gfx.setTextSize(1);
    gfx.setTextColor(TFT_WHITE, TFT_BLACK);
    gfx.setCursor(baseX + 8, baseY);
    gfx.printf("SENSOR GAIN (S): %.2f", systemState.config.sensorGainStart);    //項目番号１
    gfx.setCursor(baseX + 8, baseY + 16);
    gfx.printf("SENSOR GAIN (G): %.2f", systemState.config.sensorGainGoal);     //項目番号２
    //gfx.setCursor(baseX + 8, baseY + 32);
    //gfx.printf("VIEW RACE HISTORY");                                            //項目番号３
    //gfx.setCursor(baseX + 8, baseY + 48);
    //gfx.printf("SEND RACE HISTORY(BT)");                                           //項目番号４
    gfx.setCursor(baseX + 8, baseY + 64);
    gfx.printf("VIEW RACE HISTORY");                                        //項目番号５
    gfx.setCursor(baseX + 8, baseY + 80);
    gfx.printf("SEND RACE HISTORY(Bluetooth)");                                        //項目番号６
    gfx.setCursor(baseX + 8, baseY + 96);
    gfx.printf("CLEAR FASTEST LAP");                                        //項目番号７
    gfx.setCursor(baseX + 8, baseY + 112);
    gfx.printf("INITIALIZE");                                        //項目番号８
    //gfx.setCursor(baseX + 8, baseY + 128);
    //gfx.printf("RECALL DEFAULT SETTINGS");                                        //項目番号９
    gfx.setCursor(baseX + 8, baseY + 144);
    gfx.printf("RECALL DEFAULT SETTINGS");                                        //項目番号１０
    gfx.setTextColor(TFT_RED,TFT_BLACK);
    printCentering(0,baseY + 160,"# # # # # SETUP EXIT # # # # # ");                      //項目番号１２

    //ゲインは+50まで(5ステップ)
    if(systemState.config.sensorGainStart > 50){
        systemState.config.sensorGainStart = 0;
    }
    if(systemState.config.sensorGainGoal > 50){
        systemState.config.sensorGainGoal = 0;
    }

    // メニュー選択の矢印表示
    int arrowBaseX = gfx.width() - 10;      //右端から
    int arrowBaseXX = 8;
    int arrowBaseY = 42 + systemState.config.selectedMenuItem * 16; // 選択メニューに応じて位置調整
    int arrowBaseYY = 42 + systemState.config.selectedMenuItem * 16;
    gfx.fillTriangle(arrowBaseX, arrowBaseY, arrowBaseX - 10, arrowBaseY + 5, arrowBaseX, arrowBaseY + 10, TFT_RED);
    gfx.fillTriangle(arrowBaseXX,arrowBaseYY,arrowBaseXX + 10,arrowBaseYY+ 5,arrowBaseXX,arrowBaseYY + 10, TFT_RED);
// ボタン操作
    gfx.setCursor(10, 5);
    gfx.setTextColor(TFT_WHITE, TFT_BLACK);


    /* メニュー上下限界超えたとき */
    if(systemState.config.selectedMenuItem > 10){
        systemState.config.selectedMenuItem = 0;
        isConfigScreenInitialized = false; //一回だけ初期化
    }else if(systemState.config.selectedMenuItem < 0){
        systemState.config.selectedMenuItem = 10;
        isConfigScreenInitialized = false; //一回だけ初期化
    }
    if(systemState.config.selectedMenuItem == 2){
        systemState.config.selectedMenuItem = 4;
        isConfigScreenInitialized = false; //一回だけ初期化
    }else if(systemState.config.selectedMenuItem == 3){
        systemState.config.selectedMenuItem = 1;
        isConfigScreenInitialized = false; //一回だけ初期化
    }


    //設定メニューの項目
    if(menuSelected){   //設定ボタン押されたら
        if(systemState.config.selectedMenuItem == 0){
            //センサーゲイン設定項目（スタート）
            systemState.config.sensorGainStart += 5;
            isConfigScreenInitialized = false; //一回だけ初期化
        }else if(systemState.config.selectedMenuItem == 1){
            //センサーゲイン設定項目（ゴール）
            systemState.config.sensorGainGoal += 5;
            isConfigScreenInitialized = false; //一回だけ初期化
        }else if(systemState.config.selectedMenuItem == 4){
            systemState.config.HistoryMode = true;
            clearDisplay();
        }else if(systemState.config.selectedMenuItem == 6){ //
                //clearRaceHistory();
                gfx.printf("CLEAR HISTORY");
                systemState.config.selectedMenuItem = 1;
        }else if(systemState.config.selectedMenuItem == 9){ //RECALL DEF
            systemState.config.sensorGainGoal=0;
            systemState.config.sensorGainStart=0;

        }else if(systemState.config.selectedMenuItem == 10){
            delay(10);
            systemState.config.setupMode = false;
            clearDisplay();
            systemState.config.selectedMenuItem = 0;    //設定画面にもう一度入るとおかしくなるから
        }

        delay(100);
        gfx.setTextColor(TFT_WHITE,TFT_BLACK);
        menuSelected = false;   //押されてないよ～って
        isConfigScreenInitialized = false; //一回だけ初期化

    }
    
    delay(50); // 入力遅延

    

}


void clearRaceHistory() {
    for (int i = 0; i < 5; i++) {
        TimerHistory &history = systemState.history[i];
        history.raceNumber = 0;
        for (int j = 0; j < 3; j++) {
            history.times[j] = 0;
        }
    }

    systemState.currentHistoryIndex = -1; // 履歴が空であることを示す
    Serial.println("[DEBUG] All race history cleared.");

}