/*  グラフィック・描画関連
*   mockcar-race-program v0.9
*   graphic.cpp
* 
*   displaySplashScreen()   起動画面表示
*   updateDisplay()         SystemStateによって画面更新
*   drawIdleScreen()        アイドル（待機）画面
*   drawRaceScreen()        レース画面アップデート
*   updateTimers()          タイマー表示更新（レース中）
*   drawStatusBar(int,Str,Str,usign long)
*   printRaceHistory()      レース履歴表示
*
*   drawStatus();
*
*/

#include "main.hpp"


/***************************************************
 * 画面表示系関数
**************************************************** */
//ディスプレイ初期化（起動時）
void displaySplashScreen() {

    gfx.fillScreen(gfx.color888(0,0,0));
    gfx.setFont(&fonts::efontJA_16_b);          //日本語フォントはこれを使用する
    gfx.setTextColor(TFT_WHITE,TFT_BLACK);
    gfx.setTextWrap(false);                   //テキスト折り返し　しない
    gfx.setTextSize(1.3);                     //テキストサイズ
    printCentering(0,30,"モックカーレースプログラム v0.9");
    Serial.print("MOCKCAR RACE PROGRAM v0.9 \nNow Booting\n");
    String messageis = String(on_cycle);        //起動回数
    gfx.setTextSize(1);
    printCentering(0,55,"累積ボード起動回数 " + messageis + "回");
    gfx.setTextSize(1.3);
    printCentering(0,80,"日本ソープボックスダービー協会");

    gfx.setFont(&fonts::AsciiFont8x16);       //英字基本フォントの予定

    gfx.setColor(TFT_SKYBLUE);
    gfx.drawRect(59,170,242,9);          //プログレスバー外枠
    delay(500);

    gfx.setTextSize(0.9);
    printCentering(0,150,"Board StartUP!");
    for(int i=0;i < 180;i++){ 
      gfx.setColor(TFT_CYAN);
      gfx.fillRect(61,172,i + 60,5);           //意味無しプログレスバー 172+240
      int progress = (i * 100) / 180; 
      String count = String(progress);

      printCentering(0,185,"Loading " + count + "%");
      delay(5);
    }

    gfx.setTextSize(1.2);
    printCentering(0,200,"copyright NSBD");
    gfx.setCursor(5,5);
    //gfx.qrcode("https://nsbd.org");//画面いっぱいにでた


    delay(600); // プログレスバー完了後の待機
}


/***************************************************
 * 描画モード切り替え
 * History  履歴表示（セットアップメニュー内の想定）
 * Racing   レース中
 * └── drawRaceScreen() レース画面描画
 * Setup    セットアップメニュー
 * Idle     待機中
 * └── raceSignalDraw()  レース開始シグナル表示
 ***************************************************/

void updateDisplay() {//描画を分けるところ。描画関連はまずここ
    static DisplayState previousState = DisplayState::Idle;
    DisplayState currentState;

        //順番で画面の優先順位変わる。レーシングが一番上か。
        //ヒストリー表示時＞レース始まる＞ヒストリー抜けるとレースが表示される（裏でセットアップ出てる）
        //これを直したほうがいい。
        
    if (systemState.config.HistoryMode) {
        currentState = DisplayState::History;
    } else if (systemState.race.raceFlag) {
        currentState = DisplayState::Racing;
    } else if (systemState.config.setupMode){
        currentState = DisplayState::Setup;
    } else {
        currentState = DisplayState::Idle;
    }

    //画面ステート切り替わりで初期化
    if (currentState != previousState) {
        gfx.fillScreen(TFT_BLACK);
        printf("status changed! FillScreen \n");
    }

    switch (currentState) {
        case DisplayState::Idle:
            if(systemState.race.signalDrawing){
                raceSignalDraw();       //シグナル点灯のほうが優先
            }else{
                drawIdleScreen();
            }
            drawStatusBar();
            break;
        case DisplayState::Setup:
            handleConfigMenu();      //設定画面描画
            drawStatusBar();
            break;
        case DisplayState::Racing:
            drawRaceScreen();       // レース画面を描画（下地）
            updateTimers();         // タイムを更新（タイム部分だけ上書き）
                                    //　タイマーが３つ止まったらendRace();
            drawStatusBar();
            updateMovingBars();
            break;
        case DisplayState::History:
            drawRaceHistory(); 
            break;
    }

    previousState = currentState;
}

void drawIdleScreen() {     //待機画面
    static bool isIdleScreenInitialized = false; // 初期化済みかどうかを記録
    if (!isIdleScreenInitialized) {             //初期化してなければ
        gfx.fillScreen(TFT_BLACK);
                //初回クリア
        isIdleScreenInitialized = true; // 初期化フラグを設定
    }else if(systemState.race.signalFlag){
        isIdleScreenInitialized = false;
    }
    TimerHistory &lastRace = systemState.history[systemState.currentHistoryIndex];

    for (int i = 0; i < 3; i++) {
        
        gfx.fillCircle(70 + i * 9, 39 + i * 67, 29, TFT_GOLD); // 番号の背景
        gfx.setFont(&fonts::AsciiFont8x16);
        gfx.setTextSize(4.3);
        gfx.setTextColor(TFT_BLACK);
        gfx.setCursor(CIRCLE_TEXT_X + i * 9, 15 + i * 65);
        gfx.printf("%d", i + 1);                                //番号

        gfx.setColor(TFT_GOLD);
        gfx.drawFastHLine(DRAWLINE_X,77,350);   //水平線描画
        gfx.drawFastHLine(DRAWLINE_X + 5,79,350);   //水平線描画
        gfx.drawFastHLine(DRAWLINE_X + 10,142,350);   //水平線描画
        gfx.drawFastHLine(DRAWLINE_X + 15,144,350);   //水平線描画
        gfx.setColor(TFT_WHITE);

        gfx.setFont(&fonts::Font7); // 7セグフォント
        gfx.setTextSize(1.25);
        gfx.setTextColor(resetFlag ? TFT_CYAN : TFT_WHITE, TFT_BLACK);
        gfx.setCursor(TIMER_SEGMENT_X + i * 5, 15 + i * 66);
        gfx.printf("%02lu.%03lu",lastRace.times[i] / 1000,lastRace.times[i] % 1000); // 前回のタイム表示
        //gfx.printf("00.000"); // 初期タイム表示
    }
}

void drawRaceScreen() {
    // 番号の背景と番号の描画
    // updateDisplayから呼ばれる
    for (int i = 0; i < 3; i++) {
        gfx.fillCircle(70 + i * 9, 40 + i * 67, 29, TFT_GOLD); // 番号の背景
        gfx.setFont(&fonts::AsciiFont8x16);
        gfx.setTextSize(4.3);
        gfx.setTextColor(TFT_BLACK);
        gfx.setCursor(54 + i * 9, 15 + i * 65);
        gfx.printf("%d", i + 1);                                //番号
    }
}

void updateTimers() {   //レース中のタイマー管理
    static unsigned long previousTimes[3] = {0, 0, 0};

    for (int i = 0; i < 3; i++) {
        Timer &timer = systemState.race.timers[i];                              //インスタンス作成　構造体コピー
        Sensor &goal_sens = systemState.race.goalSensors[i];
        unsigned long elapsedTime = millis() - systemState.race.startTime;      //経過時間

        if (timer.isTiming) {
            //タイマーが動いているとき・・
            //Serial.printf("[DEBUG] Timer %d: Running, Elapsed: %lu ms\n", i + 1, elapsedTime);
            unsigned long elapsed = systemState.race.timers[i].isTiming     //タイマーが動いていれば
                                ? millis() - systemState.race.startTime     //経過時間を
                                : systemState.race.timers[i].stopTime;      //止まっていれば停止秒を入れる

            // タイムが変化した場合のみ更新　実際のレースタイマー
            if (elapsed != previousTimes[i]) {
                previousTimes[i] = elapsed;
                // 99.999秒以上の処理
                if (elapsedTime >= 99999) {
                    gfx.setFont(&fonts::Font7);
                    gfx.setTextSize(1.25);
                    gfx.setTextColor(TFT_RED, TFT_BLACK);
                    gfx.setCursor(94 + i * 5, 15 + i * 66); //少し左に
                    //gfx.printf("--.---");   //表示を変える
                    gfx.printf("%03lu.%02lu", elapsed / 1000, elapsed % 100);
                }else{
                    gfx.setFont(&fonts::Font7); //通常表示はこっち
                    gfx.setTextSize(1.25);
                    gfx.setTextColor(systemState.race.timers[i].isTiming ? TFT_GOLD : TFT_WHITE, TFT_BLACK);
                    gfx.setCursor(TIMER_SEGMENT_X + i * 5, 15 + i * 66);
                    gfx.printf("%02lu.%03lu", elapsed / 1000, elapsed % 1000);

                    gfx.setColor(TFT_GOLD);
                    gfx.drawFastHLine(DRAWLINE_X,77,350);   //水平線描画
                    gfx.drawFastHLine(DRAWLINE_X + 5,79,350);   //水平線描画
                    gfx.drawFastHLine(DRAWLINE_X + 10,142,350);   //水平線描画
                    gfx.drawFastHLine(DRAWLINE_X + 15,144,350);   //水平線描画
                    gfx.setColor(TFT_WHITE);
                }
            }
            // タイムが変化した場合のみ更新　ここまで

            // 未走行タイマーの履歴保存 タイマーが止まっていてなおかつ０秒（未計測の場合）？ありえなくない？
            if (!timer.isTiming && timer.stopTime == 0) {
                timer.stopTime = 99999; // 999秒として記録・ありえない値を代入
                }
            if(!timer.isTiming && elapsed > 99999){     //99秒超えたら
                timer.stopTime = 99999;
                timer.isTiming = false;
            }
        //タイマーが動いている判定
        } else {    //isTiming
            //さあ、タイマーが止まりました（ALL)
            //Serial.printf("[DEBUG] Timer %d: Stopped, Stop Time: %lu ms\n", i + 1, timer.stopTime);
            gfx.setFont(&fonts::Font7);
            gfx.setTextSize(1.25);
            gfx.setTextColor(TFT_BLACK, TFT_WHITE);
            gfx.setCursor(126 + i * 5, 15 + i * 66);
            gfx.printf("%02lu.%03lu", timer.stopTime / 1000, timer.stopTime % 1000);

            goal_sens.isActive = false; //追加
            //systemState.race.goalCount++;   //追加
            goal_sens.isSense = false;  
            Serial.printf("[DEBUG] Timers[%d]stopped\n",i);
        }
    }//forここまで

        if(systemState.race.timers[0].isTiming == false &&
        systemState.race.timers[1].isTiming == false &&
        systemState.race.timers[2].isTiming == false){            
            endRace();
            Serial.println("[DEBUG] All timers stopped. EndRace.");
            systemState.race.goalCount = 0;
        }
}


void drawStatusBar() {
    int x = 0;    // 画面左側
    int y = gfx.height() - 18; // ステータスバーのY座標
    int screenWidth = gfx.width();
    int barHeight = 20; // ステータスバーの高さ
    unsigned long currentTime = millis();   //描画タイム用
    gfx.setFont(&fonts::AsciiFont8x16);
    gfx.setTextSize(1);
    unsigned long seconds = systemState.config.bestTime / 1000;
    unsigned long milliseconds = systemState.config.bestTime % 1000;

      // 前回更新からの経過時間を計算
    if (currentTime - lastUpdateTime >= updateInterval) {
        lastUpdateTime = currentTime; // 更新時刻を記録
        // 状態に応じてステータスバーを描画
        if(systemState.config.HistoryMode){
            
        } else if(systemState.race.raceFlag){
            gfx.fillRect(x, y, screenWidth, barHeight, TFT_VIOLET);
            gfx.setTextColor(TFT_BLACK, TFT_VIOLET);
            gfx.setCursor(5, y + 2);
            //gfx.printf("COUNT:%d FASTEST: %02lu.%03lu <%s>", raceCount, seconds, milliseconds, message.c_str());
            gfx.printf("START! <GOALCOUNT:%d> / FASTEST : %02lu.%03lu",systemState.race.goalCount + 1, seconds, milliseconds);
        } else if(systemState.config.setupMode){
            gfx.fillRect(x, y, screenWidth, barHeight, TFT_DARKGRAY);
            gfx.setTextColor(TFT_WHITE, TFT_DARKGRAY);
            gfx.setCursor(5, y + 2);
            gfx.printf("SETUP MODE");
        } else{
            gfx.fillRect(x, y, screenWidth, barHeight, TFT_SKYBLUE);
            gfx.setTextColor(TFT_BLACK, TFT_SKYBLUE);
            gfx.setCursor(5, y + 2);
            gfx.printf("FASTEST TIME: %02lu.%03lu <RACECOUNT:%d>", seconds, milliseconds,systemState.race.totalRaceCount);
        }

        if(systemState.config.boardOPMode == 3){
            gfx.fillRect(x, y, screenWidth, barHeight, TFT_ORANGE);
            gfx.setTextColor(TFT_BLACK, TFT_ORANGE);
            gfx.setCursor(5, y + 2);
            gfx.printf("WARNING:I/O ERROR! CHECK CONNECT");
        }
        
    }
}

void drawRaceHistory() {
        u_int8_t y_offset = 20;
        setFontJapan();
        gfx.setTextColor(TFT_BLACK,TFT_WHITE);
        gfx.setCursor(317,3 + y_offset);
        gfx.printf("戻");
        gfx.setCursor(317,3 + y_offset * 2);
        gfx.printf("る");
        gfx.setCursor(317,3 + y_offset * 3);
        gfx.printf("に");
        gfx.setCursor(317,3 + y_offset * 4);
        gfx.printf("は");
        gfx.setCursor(317,3 + y_offset * 5);
        gfx.printf("決");
        gfx.setCursor(317,3 + y_offset * 6);
        gfx.printf("定");
        gfx.setCursor(317,3 + y_offset * 7);
        gfx.printf("ボ");
        gfx.setCursor(317,3 + y_offset * 8);
        gfx.printf("タ");
        gfx.setCursor(317,3 + y_offset * 9);
        gfx.printf("ン");

        setFontNormal();
        printCentering(0,5,"--- Race History (LAST 7) ---");
        gfx.setCursor(20, 22);
        gfx.printf("Race#    Car1    Car2    Car3");

        // 最速タイムの特定
        int fastestRaceIndex = -1;
        int fastestCarIndex = -1;
        unsigned long fastestTime = 999999; // 初期値を最大に設定

        for (int i = 0; i < 7; i++) {
            int historyIndex = (systemState.currentHistoryIndex - i + 7) % 7;
            TimerHistory &history = systemState.history[historyIndex];
            for (int car = 0; car < 3; car++) {
                if (history.times[car] > 0 && history.times[car] < fastestTime) {
                    fastestTime = history.times[car];
                    fastestRaceIndex = historyIndex;
                    fastestCarIndex = car;
                }
            }
        }
    // データ部分の表示（新しいレースほど下に表示するよう調整）
        int yOffset = 40; // データの描画開始Y位置
        int rowHeight = 17; // 各行の高さ

        for (int i = 6; i >= 0; i--) { // 逆順にループ
            int displayIndex = 6 - i; // 表示用のインデックス
            int historyIndex = (systemState.currentHistoryIndex - i + 7) % 7;
            TimerHistory &history = systemState.history[historyIndex];

            if(history.raceNumber == 0){
                continue; //未使用スロットはスキップ
            }
            gfx.setCursor(20, yOffset + displayIndex * rowHeight);
            gfx.printf("%4d    ", history.raceNumber);

            for (int car = 0; car < 3; car++) {
                if (historyIndex == fastestRaceIndex && car == fastestCarIndex) {
                    gfx.setTextColor(TFT_YELLOW, TFT_BLACK); // 最速タイムの強調表示
                } else {
                    gfx.setTextColor(TFT_WHITE, TFT_BLACK);
                }
                gfx.printf("%02lu.%03lu  ", 
                        history.times[car] / 1000, history.times[car] % 1000);
                    gfx.setTextColor(TFT_WHITE, TFT_BLACK);
            }
        }

        // 最速タイムの表示
        gfx.setTextColor(TFT_BLACK, TFT_YELLOW);
        gfx.setCursor(20, yOffset + 128);
        gfx.setTextSize(1.4);

        gfx.printf(" Fastest Time : %02lu.%03lu sec",
                fastestTime / 1000, fastestTime % 1000);

        gfx.setCursor(29, yOffset + 153);
        gfx.printf("Todays Fastest: %02lu.%03lu sec",
                systemState.config.bestTime / 1000,
                systemState.config.bestTime % 1000);


    if(systemState.ir_state.enterButton){
        gfx.printf("BACK");
        systemState.ir_state.enterButton = false;
        delay(15);
        if(systemState.config.HistoryMode){
            clearDisplay();
            systemState.config.HistoryMode = false;
        }
        //systemState.ir_state.isReceived = false;
    }


}
    /*
    Serial.println("Race History:");
    for (int i = 0; i < 5; i++) {
        const TimerHistory &history = systemState.history[i];
        Serial.printf("Race %d: ", history.raceNumber);
        for (int j = 0; j < 3; j++) {
            Serial.printf("Car %d: %s ms, ", j + 1, history.times[j] == 999 ? "N/A" : String(history.times[j]).c_str());
        }
        Serial.println();
    }
        */

void raceSignalDraw() {
    static bool signal_init = true;
    static unsigned long wait_time = 1000;   // シグナルウェイトタイム（ミリ秒）
    static unsigned long lastUpdateTime = 0;
    static int signal_step = 0; // 現在のステップを管理
    unsigned long currentTime = millis();   // 現在時刻

    if (signal_init) {
        // 初期化処理（外円描画と消灯赤描画）
        gfx.fillScreen(TFT_BLACK);
        gfx.setColor(TFT_DARKGRAY);
        gfx.drawCircle(70, 100, 44); // 外円1
        gfx.drawCircle(180, 100, 44); // 外円2
        gfx.drawCircle(290, 100, 44); // 外円3
        gfx.setColor(gfx.color888(70, 0, 0));
        gfx.fillCircle(70, 100, 40); // 消灯赤1
        gfx.fillCircle(180, 100, 40); // 消灯赤2
        gfx.fillCircle(290, 100, 40); // 消灯赤3
        signal_init = false;
        lastUpdateTime = currentTime; // 初期化後のタイムスタンプ
        return; // 初回は初期化のみ
    }

    if (systemState.race.signalDrawing) {
        if (currentTime - lastUpdateTime >= wait_time) {
            lastUpdateTime = currentTime; // タイムスタンプを更新

            // 各ステップに応じた描画
            switch (signal_step) {
                case 0: // 赤1点灯
                    gfx.setColor(TFT_RED);
                    gfx.fillCircle(70, 100, 40);
                    break;
                case 1: // 赤2点灯
                    gfx.setColor(TFT_RED);
                    gfx.fillCircle(180, 100, 40);
                    break;
                case 2: // 赤3点灯
                    gfx.setColor(TFT_RED);
                    gfx.fillCircle(290, 100, 40);
                    break;
                case 3: // 緑点灯
                    gfx.setColor(gfx.color888(0, 180, 100));
                    gfx.fillCircle(70, 100, 40);
                    gfx.fillCircle(180, 100, 40);
                    gfx.fillCircle(290, 100, 40);
                    break;
                default:
                    return; // すべての処理が完了したら終了
            }

            signal_step++; // 次のステップへ進む
            if (signal_step > 3) {
                systemState.race.signalDrawing = false; // 描画終了
                signal_step = 0;        // ステップをリセット
                signal_init = true;     // 初期化フラグをリセット
            }
        }
    }
}


/* ********************************************************* */
/* *********** Utility関数ここから ****************************/
/* ********************************************************* */
void clearDisplay(){
    gfx.setColor(TFT_BLACK);
    gfx.fillRect(0,0,gfx.width(),gfx.height() - STATUSBAR_HEIGHT);
}

void setFontNormal(double fontSize){
    gfx.setFont(&fonts::AsciiFont8x16);
    gfx.setTextSize(fontSize);
    gfx.setTextColor(TFT_WHITE,TFT_BLACK);
}
void setFontJapan(double fontSize){
    gfx.setFont(&fonts::efontJA_16_b);
    gfx.setTextSize(fontSize);
    gfx.setTextColor(TFT_WHITE,TFT_BLACK);
}

void printCentering(int x,int y,String printText){
    int printX = gfx.width() / 2 - gfx.textWidth(printText) / 2;  //センタリング表示
    printX + x;//ちょっと動かす用
    gfx.setCursor(printX, y);
    gfx.println(printText);
}

void updateMovingBars() {
    static int barPositions[3] = {20, 50, 70};    // 各バーの初期位置
    static int barDirections[3] = {1, -1, 1};    // 各バーの方向（1: 下方向, -1: 上方向）
    static int barSpeeds[3] = {2, 3, 4};         // 各バーの速度
    static unsigned long lastUpdateTime[3] = {0, 0, 0}; // バーごとの最終更新時間

    const int barX[3] = {2, 12, 22};             // 各バーのX座標
    const int barWidth = 9;                     // 各バーの幅
    const int barHeight = 20;                    // 各バーの高さ
    const int minBarY = 10;                       // バーの最小Y座標
    const int maxBarY = 200 - barHeight;         // バーの最大Y座標（高さを考慮）
    const uint16_t barColors[3] = {TFT_PINK, TFT_SKYBLUE, TFT_GREENYELLOW}; // バーの色

    // 各バーの動きを処理
    for (int i = 0; i < 3; i++) {
        // 一定時間経過したら更新
        if (millis() - lastUpdateTime[i] > 33) {
            lastUpdateTime[i] = millis();

            // 現在のバーを消去（黒で塗りつぶす）
            gfx.fillRect(barX[i], barPositions[i], barWidth, barHeight, barColors[i]);

            // 次の位置を計算
            barPositions[i] += barDirections[i] * barSpeeds[i];

            // 画面端での動作
            if (barPositions[i] < minBarY) {
                barPositions[i] = minBarY;       // 最小Yを超えないように
                barDirections[i] = 1;           // 方向を下に変更
                barSpeeds[i] = random(3, 8);    // ランダムな速度
            } else if (barPositions[i] > maxBarY) {
                barPositions[i] = maxBarY;      // 最大Yを超えないように
                barDirections[i] = -1;          // 方向を上に変更
                barSpeeds[i] = random(3, 10);    // ランダムな速度
            }

            // 新しいバーを描画（指定の色で描画）
            gfx.fillRect(barX[i], barPositions[i], barWidth, barHeight,TFT_BLACK);
        }
    }
}