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
        //t = time(NULL);           // 現在時刻を取得
        //tm = localtime(&t);       // `tm` に現在時刻をコピー
        isConfigScreenInitialized = true; // 初期化フラグを設定
    }
    unsigned long currentTime = millis();   //描画タイム用
    static bool menuSelected = false;

    if (systemState.config.selectedMenuItem != 7) {
        t = time(NULL);           // 本体内蔵RTCより現在時刻を取得
        tm = localtime(&t);       // `tm` に現在時刻をコピー
        rtcInitialized = true;    // 初期化フラグを設定
    }

    //clearDisplay();                     //メイン画面だけ初期化
    setFontNormal();
    // ヘッダー表示
    printCentering(0,3,"=-=-=-=-=-=-= SETUP MODE =-=-=-=-=-=-=");
    gfx.setCursor(20, 20);
    gfx.setTextColor(TFT_WHITE,TFT_DARKCYAN);
    gfx.setTextSize(1);
    gfx.setFont(&fonts::efontJA_16_b);
    gfx.printf("センサー状態(S:%d)(G1:%d)(G2:%d)(G3:%d)",digitalRead(START_SENS),digitalRead(GOAL_SENS_1),digitalRead(GOAL_SENS_2),digitalRead(GOAL_SENS_3));

    // メニュー表示(１文字８x１６なので８の倍数で設定するときれい)
    int baseX = 18;
    int baseY = 40; //16,32,48,64,80,96,112,128,144,160,176,192

    gfx.setTextSize(1);
    gfx.setTextColor(TFT_WHITE, TFT_BLACK);
    
    gfx.setCursor(baseX + 2, baseY);
    gfx.setTextSize(0.9,1);
    if(systemState.config.selectedMenuItem == 7){
        gfx.setTextColor(TFT_WHITE, TFT_DARKGREEN);
        gfx.printf("決定ボタンで次の項目・左右ボタンで数値変更");
        gfx.setTextColor(TFT_WHITE, TFT_BLACK);
    }else{
        gfx.printf("上下ボタンで項目選択・決定ボタンで数値変更");    //項目番号１
    }
    gfx.setTextSize(1);
    gfx.setFont(&fonts::AsciiFont8x16);
    gfx.setCursor(baseX + 8, baseY + 16);
    gfx.printf("BGM VOLUME < %d > (0 - 25)",systemState.config.bgmVolume);               //項目番号２
    gfx.setCursor(baseX + 8, baseY + 32);
    gfx.printf("BGM DUCKING? %s",systemState.config.bgmDucking ? "<ON>" : "<OFF>");   //項目番号３
    gfx.setCursor(baseX + 8, baseY + 48);
    gfx.printf("EVERY RACE SEND Bluetooth");                                           //項目番号４
    gfx.setCursor(baseX + 8, baseY + 64);
    gfx.printf("VIEW RACE HISTORY 7times");                                                //項目番号５
    gfx.setCursor(baseX + 8, baseY + 80);
    gfx.printf("SEND RACE HISTORY (Today) (Bluetooth)");                                     //項目番号６
    gfx.setCursor(baseX + 8, baseY + 96);
    gfx.printf("CLEAR FASTEST LAP");                                        //項目番号７
    //gfx.setCursor(baseX + 8, baseY + 112);
    //gfx.printf("INITIALIZE");                                        //項目番号８
    gfx.setCursor(baseX + 8, baseY + 112);
    if(systemState.config.selectedMenuItem == 7){
        gfx.setTextColor(TFT_WHITE, TFT_DARKGREEN);
        gfx.printf("DATE/TIME SET: %04d/%02d/%02d %02d:%02d:%02d\n",
                    tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                    tm->tm_hour, tm->tm_min, tm->tm_sec);
        gfx.setTextColor(TFT_WHITE, TFT_BLACK);
    }else{
        gfx.printf("DATE/TIME SET: %04d/%02d/%02d %02d:%02d:%02d\n",
                    tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                    tm->tm_hour, tm->tm_min, tm->tm_sec);
    }
    gfx.setCursor(baseX + 8, baseY + 128);
    gfx.printf("RECALL DEFAULT SETTINGS");                                        //項目番号１０
    gfx.setTextColor(TFT_RED,TFT_BLACK);
    printCentering(0,baseY + 144,"# # # # # SETUP EXIT # # # # # ");                      //項目番号１２

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
    int arrowBaseHori = 154 + menuValue * 2;
    gfx.fillTriangle(arrowBaseX, arrowBaseY, arrowBaseX - 10, arrowBaseY + 5, arrowBaseX, arrowBaseY + 10, TFT_RED);
    gfx.fillTriangle(arrowBaseXX,arrowBaseYY,arrowBaseXX + 10,arrowBaseYY+ 5,arrowBaseXX,arrowBaseYY + 10, TFT_RED);
    if(systemState.config.selectedMenuItem == 7){
        switch(menuValue){
            case 0:
            //年
               gfx.fillTriangle(arrowBaseHori, arrowBaseY - 10, arrowBaseHori + 5, arrowBaseY + 1, arrowBaseHori + 10, arrowBaseY - 10, TFT_GOLD);
            break;
            case 1:
            //月
                gfx.fillTriangle(arrowBaseHori + 35, arrowBaseY - 10, arrowBaseHori + 40, arrowBaseY + 1, arrowBaseHori + 45, arrowBaseY - 10, TFT_GOLD);
            break;
            case 2:
            //日
                gfx.fillTriangle(arrowBaseHori + 55, arrowBaseY - 10, arrowBaseHori + 60, arrowBaseY + 1, arrowBaseHori + 65, arrowBaseY - 10, TFT_GOLD);
            break;
            case 3:
            //時
                gfx.fillTriangle(arrowBaseHori + 76, arrowBaseY - 10, arrowBaseHori + 81, arrowBaseY + 1, arrowBaseHori + 86, arrowBaseY - 10, TFT_GOLD);
            break;
            case 4:
                gfx.fillTriangle(arrowBaseHori + 99, arrowBaseY - 10, arrowBaseHori + 104, arrowBaseY + 1, arrowBaseHori + 109, arrowBaseY - 10, TFT_GOLD);
            break;
            case 5:
                gfx.fillTriangle(arrowBaseHori + 121, arrowBaseY - 10, arrowBaseHori + 126, arrowBaseY + 1, arrowBaseHori + 131, arrowBaseY - 10, TFT_GOLD);
            break;
            return;
        }
    }
    // ボタン操作
    gfx.setCursor(10, 5);
    gfx.setTextColor(TFT_WHITE, TFT_BLACK);

    if(systemState.ir_state.upButton){
        gfx.printf("UP");
        systemState.ir_state.upButton = false;
        systemState.config.selectedMenuItem--;
        isConfigScreenInitialized = false; //画面クリアフラグ
        
    }
    if(systemState.ir_state.downButton){
        gfx.printf("DOWN");
        systemState.ir_state.downButton = false;
        systemState.config.selectedMenuItem++;
        isConfigScreenInitialized = false; //画面クリアフラグ
        
    }
    if(systemState.ir_state.enterButton){
        gfx.printf("SET");
        menuSelected = true;                //決定ボタン押されたよフラグ
        systemState.ir_state.enterButton = false;
        isConfigScreenInitialized = false; //画面クリアフラグ
        delay(15);
        if(systemState.config.HistoryMode){
            systemState.config.HistoryMode = false;
        }   
    }

if(systemState.ir_state.rightButton){
    gfx.printf("RIGHT");
    systemState.ir_state.rightButton = false;

    if(systemState.config.selectedMenuItem == 7){
        switch (menuValue) {
            case 0: 
                tm->tm_year++; 
                break; // 年

            case 1: // 月
                tm->tm_mon++; 
                if (tm->tm_mon > 11) tm->tm_mon = 0; // 12月を超えたら1月に戻す
                break;

            case 2: // 日
                tm->tm_mday++; 
                if (tm->tm_mday > getDaysInMonth(tm->tm_year, tm->tm_mon)) {
                    tm->tm_mday = 1; // 月末を超えたら1日に戻す
                }
                break;

            case 3: 
                if (++tm->tm_hour > 23) tm->tm_hour = 0; // 24時を超えたら0時
                break;

            case 4: 
                if (++tm->tm_min > 59) tm->tm_min = 0; // 60分を超えたら0分
                break;

            case 5: 
                if (++tm->tm_sec > 59) tm->tm_sec = 0; // 60秒を超えたら0秒
                break;
        }
    }

    isConfigScreenInitialized = false; // 画面クリアフラグ
}

if(systemState.ir_state.leftButton){
    gfx.printf("LEFT");
    systemState.ir_state.leftButton = false;

    if(systemState.config.selectedMenuItem == 7){
        switch (menuValue) {
            case 0: 
                tm->tm_year--; 
                break; // 年

            case 1: // 月
                tm->tm_mon--; 
                if (tm->tm_mon < 0) tm->tm_mon = 11; // 1月を超えて戻ったら12月に
                break;

            case 2: // 日
                tm->tm_mday--; 
                if (tm->tm_mday < 1) {
                    tm->tm_mday = getDaysInMonth(tm->tm_year, tm->tm_mon); // 1日未満なら月の最終日へ
                }
                break;

            case 3: 
                if (--tm->tm_hour < 0) tm->tm_hour = 23; // 0時未満なら23時
                break;

            case 4: 
                if (--tm->tm_min < 0) tm->tm_min = 59; // 0分未満なら59分
                break;

            case 5: 
                if (--tm->tm_sec < 0) tm->tm_sec = 59; // 0秒未満なら59秒
                break;
        }
    }

    isConfigScreenInitialized = false; // 画面クリアフラグ
}


    /* メニュー上下限界超えたとき */
    if(systemState.config.selectedMenuItem > 9){
        systemState.config.selectedMenuItem = 0;
        isConfigScreenInitialized = false; //一回だけ初期化
    }else if(systemState.config.selectedMenuItem < 0){
        systemState.config.selectedMenuItem = 9;
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
        }else if(systemState.config.selectedMenuItem == 7){
            //RTC TOKEI SETTING
            menuValue++;//横にずれていく
        if (menuValue > 5) { // 最後の項目を超えたら
            menuValue = 0;
            updateInternalRtc(tm);// RTC更新処理
            delay(50);
            updateExternalRtc(tm);
            // 完了メッセージ
            gfx.setCursor(18, 160);
            gfx.setTextColor(TFT_GREEN, TFT_BLACK);
            gfx.printf("RTC UPDATED!");
            delay(100);
            rtcInitialized = true;
        }

        }else if(systemState.config.selectedMenuItem == 8){ //RECALL DEF
            //初期設定にもどす
            systemState.config.bgmVolume = 5;
            systemState.config.sensorGainStart=0;
        }else if(systemState.config.selectedMenuItem == 9){
            //セットアップモード抜ける
            delay(10);
            systemState.config.setupMode = false;
            clearDisplay();
            menuValue = 0;
            systemState.config.selectedMenuItem = 0;    //設定画面にもう一度入るとおかしくなるから
            Serial.printf("SET TIME: %04d/%02d/%02d %02d:%02d:%02d\n",
                    tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                    tm->tm_hour, tm->tm_min, tm->tm_sec);
            if(rtc_read()){
                Serial.println("RTC Read:");
            }else{
                Serial.println("RTC Read Err!");
            }


        }

            if (systemState.config.selectedMenuItem != 7) {
                rtcInitialized = false; // 他のメニューに移動したらフラグリセット
            }

        delay(100);
        gfx.setTextColor(TFT_WHITE,TFT_BLACK);
        menuSelected = false;   //押されてないよ～って
        isConfigScreenInitialized = false; //一回だけ初期化

    }else{
        //設定ボタン押されてないとき（つまりいつも）

    }


    delay(50); // 入力遅延

    if(mp3Changed){
        setVolumeMP3(systemState.config.bgmVolume);
        delay(100);
        printCentering(0,5,"VOLUME CHANGED");
        mp3Changed = false;
    }
}

int getDaysInMonth(int year, int month) {
    static const int daysInMonth[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    // うるう年判定 (4で割り切れる年はうるう年、100で割り切れる年は平年、400で割り切れる年はうるう年)
    if (month == 1) { // 2月（インデックス1）
        if ((year + 1900) % 4 == 0 && ((year + 1900) % 100 != 0 || (year + 1900) % 400 == 0)) {
            return 29; // うるう年の2月は29日
        }
        return 28; // 平年の2月は28日
    }
    return daysInMonth[month]; // それ以外の月
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