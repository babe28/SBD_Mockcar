/*  設定画面関連
*   mockcar-race-program v0.2
*   graphic.cpp
*/
#include "main.hpp"

struct tm *tm;
time_t t;
int menuValue = 0;

void handleConfigMenu() {
    static bool isConfigScreenInitialized = false; // 初期化済みかどうかを記録
    static bool mp3Changed = false;
    static bool rtcInitialized = false;


    if (!isConfigScreenInitialized) {
        gfx.fillScreen(TFT_BLACK);
                //初回クリア
        t = time(NULL);           // 現在時刻を取得
        tm = localtime(&t);       // `tm` に現在時刻をコピー
        isConfigScreenInitialized = true; // 初期化フラグを設定
    }
    unsigned long currentTime = millis();   //描画タイム用
    static bool menuSelected = false;

    if (systemState.config.selectedMenuItem == 8 && !rtcInitialized) {
        t = time(NULL);           // 現在時刻を取得
        tm = localtime(&t);       // `tm` に現在時刻をコピー
        rtcInitialized = true;    // 初期化フラグを設定
    }

    //clearDisplay();                     //メイン画面だけ初期化
    setFontNormal();
    // ヘッダー表示
    printCentering(0,3,"= = = = = = = SETUP MODE = = = = = = =");
    gfx.setCursor(20, 20);
    gfx.setTextColor(TFT_WHITE,TFT_DARKCYAN);
    gfx.setTextSize(1);
    gfx.printf("SENSOR_STATE(S:%d)(G1:%d)(G2:%d)(G3:%d)",digitalRead(START_SENS),digitalRead(GOAL_SENS_1),digitalRead(GOAL_SENS_2),digitalRead(GOAL_SENS_3));

    // メニュー表示(１文字８x１６なので８の倍数で設定するときれい)
    int baseX = 18;
    int baseY = 40; //16,32,48,64,80,96,112,128,144,160,176,192

    gfx.setTextSize(1);
    gfx.setTextColor(TFT_WHITE, TFT_BLACK);
    gfx.setCursor(baseX + 8, baseY);
    gfx.printf("SENSOR GAIN (S): 000 ");    //項目番号１
    gfx.setCursor(baseX + 8, baseY + 16);
    gfx.printf("BGM VOLUME < %d >",systemState.config.bgmVolume);               //項目番号２
    gfx.setCursor(baseX + 8, baseY + 32);
    gfx.printf("BGM DUCKING? %s",systemState.config.bgmDucking ? "YES" : "NO");   //項目番号３
    gfx.setCursor(baseX + 8, baseY + 48);
    gfx.printf("EVERY RACE SEND BT)");                                           //項目番号４
    gfx.setCursor(baseX + 8, baseY + 64);
    gfx.printf("VIEW RACE HISTORY");                                                //項目番号５
    gfx.setCursor(baseX + 8, baseY + 80);
    gfx.printf("SEND RACE HISTORY(Bluetooth)");                                     //項目番号６
    gfx.setCursor(baseX + 8, baseY + 96);
    gfx.printf("CLEAR FASTEST LAP");                                        //項目番号７
    gfx.setCursor(baseX + 8, baseY + 112);
    gfx.printf("INITIALIZE");                                        //項目番号８
    gfx.setCursor(baseX + 8, baseY + 128);
    if(systemState.config.selectedMenuItem == 8){
    gfx.setTextColor(TFT_WHITE, TFT_DARKGREEN);
    gfx.printf("RTC SET: %04d/%02d/%02d %02d:%02d:%02d\n",
                tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                tm->tm_hour, tm->tm_min, tm->tm_sec);
    gfx.setTextColor(TFT_WHITE, TFT_BLACK);
    
    }else{
    gfx.printf("RTC SET: %04d/%02d/%02d %02d:%02d:%02d\n",
                tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                tm->tm_hour, tm->tm_min, tm->tm_sec);
    }
    gfx.setCursor(baseX + 8, baseY + 144);
    gfx.printf("RECALL DEFAULT SETTINGS");                                        //項目番号１０
    gfx.setTextColor(TFT_RED,TFT_BLACK);
    printCentering(0,baseY + 160,"# # # # # SETUP EXIT # # # # # ");                      //項目番号１２

    //ゲインは+50まで(5ステップ)
    if(systemState.config.sensorGainStart > 50){
        systemState.config.sensorGainStart = 0;
    }
    //BGMは20最大
    if(systemState.config.bgmVolume > 20){
        systemState.config.bgmVolume = 0;
    }

    // メニュー選択の矢印表示
    int arrowBaseX = gfx.width() - 10;      //右端から
    int arrowBaseXX = 8;
    int arrowBaseY = 42 + systemState.config.selectedMenuItem * 16; // 選択メニューに応じて位置調整
    int arrowBaseYY = 42 + systemState.config.selectedMenuItem * 16;
    int arrowBaseHori = 120 + menuValue * 20;
    gfx.fillTriangle(arrowBaseX, arrowBaseY, arrowBaseX - 10, arrowBaseY + 5, arrowBaseX, arrowBaseY + 10, TFT_RED);
    gfx.fillTriangle(arrowBaseXX,arrowBaseYY,arrowBaseXX + 10,arrowBaseYY+ 5,arrowBaseXX,arrowBaseYY + 10, TFT_RED);
    if(systemState.config.selectedMenuItem == 8){
       gfx.fillTriangle(arrowBaseHori, arrowBaseY - 8, arrowBaseHori + 5, arrowBaseY + 2, arrowBaseHori + 10, arrowBaseY - 8, TFT_GOLD);
    }
    // ボタン操作
    gfx.setCursor(10, 5);
    gfx.setTextColor(TFT_WHITE, TFT_BLACK);

    if(systemState.ir_state.upButton){
        gfx.printf("UP");
        systemState.ir_state.upButton = false;
        systemState.config.selectedMenuItem--;
        isConfigScreenInitialized = false; //一回だけ初期化
        
    }
    if(systemState.ir_state.downButton){
        gfx.printf("DOWN");
        systemState.ir_state.downButton = false;
        systemState.config.selectedMenuItem++;
        isConfigScreenInitialized = false; //一回だけ初期化
        
    }
    if(systemState.ir_state.rightButton){
        gfx.printf("RIGHT");
        systemState.ir_state.rightButton = false;
        menuValue++;
        isConfigScreenInitialized = false; //一回だけ初期化
        
    }
    if(systemState.ir_state.leftButton){
        gfx.printf("LEFT");
        systemState.ir_state.leftButton = false;
        menuValue--;
        isConfigScreenInitialized = false;
    }
    if(systemState.ir_state.enterButton){
        gfx.printf("SET");
        menuSelected = true;
        systemState.ir_state.enterButton = false;
        isConfigScreenInitialized = false; //一回だけ初期化
        delay(15);
        if(systemState.config.HistoryMode){
            systemState.config.HistoryMode = false;
        }
        
    }


    /* メニュー上下限界超えたとき */
    if(systemState.config.selectedMenuItem > 10){
        systemState.config.selectedMenuItem = 0;
        isConfigScreenInitialized = false; //一回だけ初期化
    }else if(systemState.config.selectedMenuItem < 0){
        systemState.config.selectedMenuItem = 10;
        isConfigScreenInitialized = false; //一回だけ初期化
    }

    /* 項目飛ばし
    if(systemState.config.selectedMenuItem == 2){
        systemState.config.selectedMenuItem = 4;
        isConfigScreenInitialized = false; //一回だけ初期化
    }else if(systemState.config.selectedMenuItem == 3){
        systemState.config.selectedMenuItem = 1;
        isConfigScreenInitialized = false; //一回だけ初期化
    }*/


    //設定メニューの項目
    if(menuSelected){   //設定ボタン押されたら
        if(systemState.config.selectedMenuItem == 0){
            //センサーゲイン設定項目（スタート）
            systemState.config.sensorGainStart += 5;
            isConfigScreenInitialized = false;
        }else if(systemState.config.selectedMenuItem == 1){
            //VOLUME設定項目
            systemState.config.bgmVolume += 5;
            mp3Changed = true;
            isConfigScreenInitialized = false;
        }else if(systemState.config.selectedMenuItem == 2){
            //DUCKING 反転
            systemState.config.bgmDucking = !systemState.config.bgmDucking;
        }else if(systemState.config.selectedMenuItem == 3){
            //毎レースBluetooth送信
        
        }else if(systemState.config.selectedMenuItem == 4){
            //レースヒストリー表示
            systemState.config.HistoryMode = true;
            clearDisplay();
        }else if(systemState.config.selectedMenuItem == 6){
            //レース消去
                clearRaceHistory();
                printCentering(0,100,"CLEAR HISTORY");
                systemState.config.selectedMenuItem = 1;
        }else if(systemState.config.selectedMenuItem == 8){
            //RTC TOKEI SETTING
        menuValue++;
        if (menuValue > 5) { // 最後の項目を超えたら
            menuValue = 0;

            // RTC更新処理
            updateInternalRtc(tm);
            delay(10);
            updateExternalRtc(tm);

            // 完了メッセージ
            gfx.setCursor(18, 160);
            gfx.setTextColor(TFT_GREEN, TFT_BLACK);
            gfx.printf("RTC UPDATED!");
            delay(100);
            
        }
        }else if(systemState.config.selectedMenuItem == 9){ //RECALL DEF
            //初期設定にもどす
            systemState.config.bgmVolume = 5;
            systemState.config.sensorGainStart=0;
        }else if(systemState.config.selectedMenuItem == 10){
            //セットアップモード抜ける
            delay(10);
            systemState.config.setupMode = false;
            clearDisplay();
            systemState.config.selectedMenuItem = 0;    //設定画面にもう一度入るとおかしくなるから
        }

            if (systemState.config.selectedMenuItem != 8) {
                rtcInitialized = false; // 他のメニューに移動したらフラグリセット
            }

        delay(100);
        gfx.setTextColor(TFT_WHITE,TFT_BLACK);
        menuSelected = false;   //押されてないよ～って
        isConfigScreenInitialized = false; //一回だけ初期化

    }


        // **左右ボタンで値を変更**
        if(systemState.config.selectedMenuItem == 8){

            if (systemState.ir_state.rightButton) {
                systemState.ir_state.rightButton = false;
                switch (menuValue) {
                    case 0: tm->tm_year++; break;           // 年
                    case 1: tm->tm_mon = (tm->tm_mon + 1) % 12; break; // 月
                    case 2: tm->tm_mday++; break;           // 日
                    case 3: tm->tm_hour++; break;           // 時
                    case 4: tm->tm_min++; break;            // 分
                    case 5: tm->tm_sec++; break;            // 秒
                }
            }
            if (systemState.ir_state.leftButton) {
                systemState.ir_state.leftButton = false;
                switch (menuValue) {
                    case 0: tm->tm_year--; break;           // 年
                    case 1: tm->tm_mon = (tm->tm_mon - 1 + 12) % 12; break; // 月
                    case 2: tm->tm_mday--; break;           // 日
                    case 3: tm->tm_hour--; break;           // 時
                    case 4: tm->tm_min--; break;            // 分
                    case 5: tm->tm_sec--; break;            // 秒
                }
            }
            
        }

    delay(50); // 入力遅延

    if(mp3Changed){
        setVolumeMP3(systemState.config.bgmVolume);
        delay(100);
        printCentering(0,5,"VOLUME CHANGED");
        mp3Changed = false;
    }
}



void drawRtcSetMenu(struct tm* tm) {
    clearDisplay(); // 全画面クリア

    // 時計設定メニューを描画
    gfx.setCursor(18, 40);
    gfx.setTextSize(1);
    gfx.setTextColor(TFT_WHITE, TFT_BLACK);
    gfx.printf("RTC SETTING\n");
    gfx.printf("YEAR: %04d\n", tm->tm_year + 1900);
    gfx.printf("MONTH: %02d\n", tm->tm_mon + 1);
    gfx.printf("DAY: %02d\n", tm->tm_mday);
    gfx.printf("HOUR: %02d\n", tm->tm_hour);
    gfx.printf("MIN: %02d\n", tm->tm_min);
    gfx.printf("SEC: %02d\n", tm->tm_sec);

    // 矢印を選択中の項目に描画
    int arrowBaseX = 120; // 文字列の長さを基準に調整
    int arrowY = 40 + menuValue * 16;
    gfx.fillTriangle(arrowBaseX, arrowY, arrowBaseX + 10, arrowY + 5, arrowBaseX, arrowY + 10, TFT_YELLOW);
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