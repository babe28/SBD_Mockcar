#include <Arduino.h>
#include "main.hpp"

/* ****************************************************
* モックカーレース用プログラム v0.8a
* v0.2 2025/1/3   プログラム作成・基本関数作成
* v0.5 2025/1/6   関数完成
* v0.6 2025/1/7   関数整理・センサー入力
* v0.7 2025/1/8   関数整理・構造体整理・クラス整理・新規追加・外部シリアル
* v0.8 2025.1.9   Initilize整理・履歴が正しく表示されるように
* v0.8a 2025.1.13  赤外線受信機能・画面表示整理
* v0.8c 2025.1.28  接続ピン整理・DFPlayer機能追加・赤外線受信関数調整・RTC追加
* ------------ 今後の ----------------
* センサー関連          4つつなぐ
* シリアル関数関連      関数整備・外部からのコントロール？
* 設定画面・履歴など    チラツキ防止・設定項目日本語化・英語選択
* 
***************************************************** */

//割り込みハンドラ内の変数はvolatileで最適化防止
volatile bool resetFlag = false;                  //リセットボタン押されたかどうかの判定

bool firstRun = true;                 //起動後初回実行かどうかを判定
int goalcount = 0;                    //ゴール通過台数　MAX2(0-2)
int raceTotalCount = 0;               //起動後何回レースしたか
RingbufHandle_t buffer = NULL;        // IR用リングバッファ
RingbufHandle_t IRbuffer=NULL;        //赤外線受信バッファ
Button buttonStates[3];               //ボタン設定用
uint8_t REG_table[7];                 //時間テーブルRTC
const char *week[] = {"SUN","MON","TUE","WED","THR","FRI","SAT"};
struct tm timeinfo;                   //内蔵RTC用時刻構造体


// 描画用ステータス
unsigned long lastUpdateTime = 0;     //前回更新時刻格納用
const int refreshRate = 15;           //リフレッシュレート（Hz）
const unsigned long updateInterval = 1000 / refreshRate; // 更新間隔（ms）

//設定保存用の変数（EEPROMの後継ライブラリPreferences）まずは変数定義
int on_cycle = 0; //起動回数
unsigned long best_time_onboard = 0; //ボード最速タイム
float sensor_gain_start = 1.00; //センサーのゲイン（スタート）
float sensor_gain_goal = 1.00; //センサーゲイン（ゴール）

SystemState systemState;

  LGFX::LGFX(void) {
    { // 表示パネル制御の設定
      auto cfg = _panel_instance.config();    // 表示パネル設定用の構造体を取得
      // 出力解像度を設定;
      cfg.memory_width  = SCREEN_WIDTH; // 出力解像度 幅
      cfg.memory_height = SCREEN_HEIGHT; // 出力解像度 高さ
      // 実際に利用する解像度を設定;
      cfg.panel_width  = SCREEN_WIDTH;  // 実際に使用する幅   (memory_width と同値か小さい値を設定する)
      cfg.panel_height = SCREEN_HEIGHT;  // 実際に使用する高さ (memory_heightと同値か小さい値を設定する)
      // 表示位置オフセット量を設定;
      cfg.offset_x = 0;       // 表示位置を右にずらす量 (初期値 0)
      cfg.offset_y = 0;       // 表示位置を下にずらす量 (初期値 0)
      _panel_instance.config(cfg);
    }

    {
      auto cfg = _panel_instance.config_detail();
      // 出力信号の種類を設定;
      // cfg.signal_type = cfg.signal_type_t::NTSC;
      cfg.signal_type = cfg.signal_type_t::NTSC_J;
      // 出力先のGPIO番号を設定;
      cfg.pin_dac = 26;       // DAC25 または 26 のみが選択できます;
      // PSRAMメモリ割当の設定;
      cfg.use_psram = 1;      // 0=PSRAM不使用 / 1=PSRAMとSRAMを半々使用 / 2=全部PSRAM使用;
      // 出力信号の振幅の強さを設定;
      cfg.output_level = 130; // 初期値128
      // ※ GPIOに保護抵抗が付いている等の理由で信号が減衰する場合は数値を上げる。;
      // ※ M5StackCore2 はGPIOに保護抵抗が付いているため 200 を推奨。;
      // 彩度信号の振幅の強さを設定;
      cfg.chroma_level = 128; // 初期値128
      // 数値を下げると彩度が下がり、0で白黒になります。数値を上げると彩度が上がります。;

      // バックグラウンドでPSRAMの読出しを行うタスクの優先度を設定;
      // cfg.task_priority = 25;
      // バックグラウンドでPSRAMの読出しを行うタスクを実行するCPUを選択 (APP_CPU_NUM or PRO_CPU_NUM);
      // cfg.task_pinned_core = PRO_CPU_NUM;
      _panel_instance.config_detail(cfg);
    }
    setPanel(&_panel_instance);
  };

LGFX gfx;                   //インスタンス名gfx
LGFX_Sprite sprite1(&gfx);  //スプライト作成

BluetoothSerial SerialBT;     //Bluetoothシリアルのインスタンス作成
Sensor startSensor;       //スタートセンサー

  /* シリアルデバッグを有効化するならここ */
bool SerialDebug = true;                        //シリアルデバッグモード


/* ********************************************************* */
/* *********** ボードセットアップここから **********************/
/* ********************************************************* */

void setup(void)
{
  pinMode(RESET_BUTTON_PIN,INPUT);  //リセットボタン pins39
  pinMode(START_BUTTON_PIN,INPUT);  //スタートボタン pins27
  pinMode(LED_BLUE,OUTPUT);                //LED pins13
  pinMode(LED_GREEN,OUTPUT);               //LED pins15
  pinMode(BULTIN_LED,OUTPUT);         //内蔵LED pins2
  pinMode(START_SENS,INPUT);          //暫定処理
  pinMode(GOAL_SENS_1, INPUT);        //暫定処理
  pinMode(GOAL_SENS_2, INPUT);        //暫定処理
  pinMode(GOAL_SENS_3, INPUT);        //暫定処理

  digitalWrite(BULTIN_LED,HIGH);  //内蔵LED ON

  pinMode(SERIAL_MP3_RX,INPUT);      //MP3モジュール  RX18
  pinMode(SERIAL_MP3_TX,OUTPUT);     //MP3モジュール  TX19
  analogReadResolution(12);           //ADコンバーター12ビット def12bit
  analogSetAttenuation(ADC_11db);      //ADコンバーターゲイン設定 def11db

  attachInterrupt(digitalPinToInterrupt(GOAL_SENS_1), goalSensorISR1, FALLING);
  attachInterrupt(digitalPinToInterrupt(GOAL_SENS_2), goalSensorISR2, FALLING);
  attachInterrupt(digitalPinToInterrupt(GOAL_SENS_3), goalSensorISR3, FALLING);
  //attachInterrupt(digitalPinToInterrupt(RESET_BUTTON_PIN),handleResetButton,FALLING); //不安定だから無効化

  Serial.begin(115200);                         // Start Serial at 115200bps(デバッグ)
  Serial2.begin(9600, SERIAL_8N1, 18, 19);      // Start Serial2 at 9600bps（DFPlayer Mini）
  printf("Serial2 Initializing...\n");
  Wire.begin(I2C_SDA,I2C_SCL);                  // Start I2C library

  rmt_config_t rmtConfig;                           //赤外線受信クラス定義
  rmtConfig.rmt_mode = RMT_MODE_RX;                 //受信モード
  rmtConfig.channel = RMT_CHANNEL_0;                //CHANNEL 0で初期化
  rmtConfig.clk_div = 80;                           //RMTのクロック分周
  rmtConfig.gpio_num = GPIO_NUM_21;                 //赤外線受信ポート
  rmtConfig.mem_block_num = 4;                      //メモリブロック数(1-255:1ブロックあたり64ペアの送受信) 1だとオーバーフローする
  rmtConfig.rx_config.filter_en = true;             //フィルターEnable
  rmtConfig.rx_config.filter_ticks_thresh = 100;    //1ticks 255us 以下の信号を除外
  rmtConfig.rx_config.idle_threshold = 10000;       //10000us以上の信号を除外
  rmt_config(&rmtConfig);                           //RMT設定
  rmt_driver_install(rmtConfig.channel,2048,0);     //RMT Ring Buffer 1024byte
  rmt_get_ringbuf_handle(RMT_CHANNEL_0, &IRbuffer); //リングバッファ設定
  rmt_rx_start(RMT_CHANNEL_0, true);


  gfx.init();                      // Start LovyanGFX
  gfx.fillScreen(TFT_BLACK);       // 画面初期化
  sprite1.setPsram(true);          // PSRAMにスプライトを配置

  SerialBT.begin("MockcarRACETimer");               //この名前でBluetoothの一覧に出てくる
  Serial.println("Bluetooth Start!");
  Serial.println("I/O complete");

  //変数初期化
  int boardOPmode = 1;                        //ボード動作モード　0=NORMAL,1=LEGACY,2=OPTIONAL（当分レガシーモードのみ）

  systemState.race.startTime = 0;             //スタートタイム初期化
  for (int i = 0; i < 3; i++) {
      systemState.race.timers[i].stopTime = 0;              //レーン毎の停止時間
      systemState.race.timers[i].isTiming = false;          //タイマー稼働中判定
      systemState.race.goalSensors[i].isActive = false;     //ゴールセンサーアクティブ判定
      systemState.race.goalSensors[i].lastTriggerTime = 0;  //ゴールセンサーの時間
  }
  systemState.config.setupMode = false;       //設定モードにいるか判定
  systemState.config.selectedMenuItem = 0;    //設定モードのメニュー選択用

  SerialBT.println("setup function finished.");
  Serial.println("setup function finished");
  
  resetRaceState();                   //変数初期化
  delay(100);                         //
  eeprom_initialize();                //ブート回数記録・EEPROM読み出し/書き込み
  //rtcTimeSet();                     //RTC強制時間設定
  displaySplashScreen();              //起動時画面読み込み

  Serial.printf("heap_caps_get_free_size(MALLOC_CAP_DMA):%d\n", heap_caps_get_free_size(MALLOC_CAP_DMA) );
  Serial.printf("heap_caps_get_largest_free_block(MALLOC_CAP_DMA):%d\n", heap_caps_get_largest_free_block(MALLOC_CAP_DMA) );

  delay(500);                    //デバッグ用　取ってOK
  digitalWrite(BULTIN_LED,LOW);  //内蔵LED OFF

}

/* ********************************************************* */
/* *********** セットアップ関数ここまで ************************/
/* ********************************************************* */

/* ********************************************************* */
/* *********** メインloop関数ここから **************************/
/* ********************************************************* */


void loop() {
    // 起動時の初期化処理
    if (firstRun) {
        resetRaceState();           // レース状態の初期化・変数の初期化
        initializeHistory();        //レースヒストリー初期化
        initializeDFPlayer();       //DFPlayer初期化
        gfx.fillScreen(TFT_BLACK);  
        drawIdleScreen();               //初期画面を描画
        rtc_read();                     //RTC読み込みして
        setInternalRTC();               //内蔵RTC外部RTCの時刻をセット
        digitalWrite(LED_GREEN,LOW);
        firstRun = false;               //初期フラグを解除
        Serial.println("First Run Complete.");

    }

    digitalWrite(LED_GREEN,HIGH);   //LED点灯（メインルーチン速度測定用）
    checkStartSensor();     // スタートセンサーの状態を確認
    checkResetButton();     // リセットボタンが押されたかどうか・ボタン自体は割り込み
    updateDisplay();        //タイマーと描画関連全般

    if(buttonStates[0].isLongPressed){    // 長押し検知でセットアップモードに遷移
      systemState.config.setupMode = true;
    }
    digitalWrite(LED_GREEN,LOW);   //LED点灯（メインルーチン速度測定用）



      //printf("reset=%d",digitalRead(RESET_BUTTON_PIN));

      delay(5);

    if(!systemState.race.raceFlag){
      ReceiveIR(systemState);
      //Analyze_IR();
    }

    if(systemState.ir_state.isReceived){ //IR受信があったら
      if(systemState.ir_state.menuButton){
        systemState.ir_state.menuButton = false;
        if(!systemState.config.setupMode){
          handleConfigMenu();
          systemState.config.setupMode = true;

        }
      }
      if(systemState.ir_state.playButton){
        systemState.ir_state.playButton = false;
        stopMP3();
      }
      if(systemState.ir_state.rightButton){
        systemState.ir_state.rightButton = false;

      }
      if(systemState.ir_state.leftButton){
        systemState.ir_state.leftButton = false;

      }
    systemState.ir_state.isReceived = false;
    }



      checkReadyButton(); //スタート(レディ）ボタンが押されたかどうか
/*
    Serial.printf("STARTSENSOR:%d\n",digitalRead(START_SENS));
    Serial.printf("GOALSENSOR1:%d\n",digitalRead(GOAL_SENS_1));
    Serial.printf("GOALSENSOR2:%d\n",digitalRead(GOAL_SENS_2));
    Serial.printf("GOALSENSOR3:%d\n",digitalRead(GOAL_SENS_3));

    delay(100);
    */
}


/* ********************************************************* */
/* *********** メインloop関数ここまで **************************/
/* ********************************************************* */

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


/***********************************
 * ステート変更　メインループからのちほど避難させる
 ************************************/

void systemStateChange(){
  if(systemState.ir_state.enterButton){
    systemState.ir_state.enterButton = false;
    
    if(!systemState.config.setupMode){
      systemState.config.setupMode = true;
      handleConfigMenu();
    }
  }

}


/* **************************************************
 * タイマー関連関数
 **************************************************** */
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

void startRace() {
    // レースがすでに開始していれば何もしない
    if (systemState.race.raceFlag) {
        Serial.println("[DEBUG] Race already in progress.");
        return;
    }
    if(systemState.config.setupMode){           //もし設定画面なら
        systemState.config.setupMode = false;     //設定画面を抜ける
    }
  
    // レース開始処理
    systemState.race.startTime = millis();
    systemState.race.raceFlag = true;
    systemState.race.startSensor.isActive = true;

    // 全タイマーを初期化して開始
    for (int i = 0; i < 3; i++) {
        systemState.race.timers[i].isTiming = true;
        systemState.race.timers[i].stopTime = 0; // 停止時間をリセット
        Serial.printf("[DEBUG] Timer %d started.\n", i + 1);
    }

    systemState.race.signalDrawing = false; //シグナル描画フラグをOFF
    systemState.race.signalFlag = false;    //シグナルフラグをOFF


    Serial.println("Race started!");

}


void resetRaceState() {
    systemState.race.raceFlag = false;
    systemState.race.goalCount = 0;
    for (int i = 0; i < 3; i++) {
        systemState.race.timers[i].isTiming = false;
        systemState.race.timers[i].stopTime = 0;
    }
    //updateDisplay(); // 初期画面を描画
    
    Serial.println("[DEBUG] Race state reset!");
    
}

void endRace() {
    unsigned long carTimes[3] = {0};
    for (int i = 0; i < 3; i++) {
        // タイム計算（そのまま stopTime を保存）
        carTimes[i] = systemState.race.timers[i].stopTime;
    }
    
    addRaceHistory(carTimes, systemState.race.totalRaceCount);    // 履歴追加
    for( int i=0;i<3;i++){
    //SerialBT.printf("R[%d]:%02lu.%03lu \n",i,systemState.race.timers[i].stopTime/1000,
    //    systemState.race.timers[i].stopTime%1000);
    }

    systemState.race.raceFlag = false;
    systemState.race.goalCount = 0;
    for (int i = 0; i < 3; i++) {
        systemState.race.timers[i].isTiming = false;
        systemState.race.timers[i].stopTime = 0;
    }

    //resetRaceState();                                             // 状態リセットをやめて上記にした。
    systemState.race.totalRaceCount++;                            // レース回数をインクリメント
    Serial.printf("[DEBUG] Race %d ended and history saved.\n", systemState.race.totalRaceCount);
    
}


/*********************************
 * レース履歴
********************************* */

void addRaceHistory(unsigned long carTimes[], int raceNumber) {
    systemState.currentHistoryIndex = (systemState.currentHistoryIndex + 1) % 7;
    //最大の履歴が7になったら0に戻る

    TimerHistory &history = systemState.history[systemState.currentHistoryIndex];
    history.raceNumber = raceNumber;

    for (int i = 0; i < 3; i++) {
        history.times[i] = carTimes[i];

        // 最速タイムの更新（未走行タイムも含む）
        if (systemState.config.bestTime == 0 || carTimes[i] < systemState.config.bestTime) {
            systemState.config.bestTime = carTimes[i];
            Serial.printf("[DEBUG] Fastest Time Updated: %02lu.%03lu sec\n", 
                          systemState.config.bestTime / 1000, systemState.config.bestTime % 1000);
        }
    }

    Serial.printf("[DEBUG] Race %d added to history.\n", raceNumber);
}

//履歴初期化。ここの履歴は7回分まで。
void initializeHistory() {
    for (int i = 0; i < 7; i++) {
        systemState.history[i].raceNumber = 0;
        for (int j = 0; j < 3; j++) {
            systemState.history[i].times[j] = 0;
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

void rtc_initialize(){
  //RTC初期化

  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(0x00); //START_REGISTOR
  delay(1);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_ADDRESS,7);
  for(int i=0;i<=7;i++){
    REG_table[i]=Wire.read();
    delay(1);
  }
  Serial.printf("DS1307 is 20%02X/%02X/%02X (%s) %02X:%02X:%02X\n",
                REG_table[6],              // 年 (16進数形式)
                REG_table[5],              // 月 (16進数形式)
                REG_table[4],              // 日 (16進数形式)
                week[REG_table[3]],    // 曜日 (インデックス調整)
                REG_table[2],              // 時 (16進数形式)
                REG_table[1],              // 分 (16進数形式)
                REG_table[0]);             // 秒 (16進数形式)
}

void rtcTimeSet(){
  //DS1307RTC 強制時間設定
  Wire.beginTransmission(DS1307_ADDRESS);
  delay(1);
  Wire.write(0x00); //START_REGISTOR
  Wire.write(0x00); //秒
  Wire.write(0x14); //分
  Wire.write(0x08); //時
  Wire.write(0x02); //週(SUN 00 MON 01 TUE 02)
  Wire.write(0x28); //日
  Wire.write(0x01); //月
  Wire.write(0x25); //年 20xx年
  delay(1);
  Wire.endTransmission();
}

void rtc_read(){
  //RTC読み出し
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(0x00); //START_REGISTOR
  Wire.endTransmission();
  Wire.requestFrom(DS1307_ADDRESS,7);
  REG_table[0] = Wire.read();
  REG_table[1] = Wire.read();
  REG_table[2] = Wire.read();
  REG_table[3] = Wire.read();
  REG_table[4] = Wire.read();
  REG_table[5] = Wire.read();
  REG_table[6] = Wire.read();
  Serial.printf("20%02X/%02X/%02X (%s) %02X:%02X:%02X\n",
                REG_table[6],              // 年 (16進数形式)
                REG_table[5],              // 月 (16進数形式)
                REG_table[4],              // 日 (16進数形式)
                week[REG_table[3]],    // 曜日 (インデックス調整)
                REG_table[2],              // 時 (16進数形式)
                REG_table[1],              // 分 (16進数形式)
                REG_table[0]);             // 秒 (16進数形式)
}

void setInternalRTC() {
    timeinfo.tm_year = bcdToDec(REG_table[6]) + 2000 - 1900; // 年（1900年基準）
    timeinfo.tm_mon = bcdToDec(REG_table[5]) - 1;            // 月（0-11）
    timeinfo.tm_mday = bcdToDec(REG_table[4]);               // 日
    timeinfo.tm_hour = bcdToDec(REG_table[2]);               // 時
    timeinfo.tm_min = bcdToDec(REG_table[1]);                // 分
    timeinfo.tm_sec = bcdToDec(REG_table[0]);                // 秒
    timeinfo.tm_isdst = -1;                                  // サマータイム情報を無効化

    struct timeval now = {mktime(&timeinfo), 0};
    settimeofday(&now, NULL); // ESP32の内蔵RTCに時刻を設定
}

int bcdToDec(uint8_t val) {
    return ((val >> 4) * 10) + (val & 0x0F);
}

void readInternalRTC() {
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return;
    }
    Serial.printf("Internal RTC: %04d/%02d/%02d %02d:%02d:%02d\n",
                  timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                  timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}