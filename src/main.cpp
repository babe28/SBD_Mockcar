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
* ------------ 今後の ----------------
* センサー関連          4つつなぐ
* シリアル関数関連      関数整備・外部からのコントロール可能にする
* シリアル通信がブロッキングのため、変更する必要あり
* 設定画面・履歴など    チラツキ防止・設定項目日本語化・英語選択
* 内蔵RTC？
* 
***************************************************** */

//割り込みハンドラ内の変数はvolatileで最適化防止
volatile bool resetFlag = false;                  //リセットボタン押されたかどうかの判定

bool firstRun = true;                 //起動後初回実行かどうかを判定
int goalcount = 0;                    //ゴール通過台数
int raceTotalCount = 0;               //起動後何回レースしたか
RingbufHandle_t buffer = NULL;        // IR用リングバッファ
RingbufHandle_t IRbuffer=NULL;
Button buttonStates[3];               //ボタン設定用

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
      cfg.output_level = 140; // 初期値128
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
LGFX_Sprite sprite2(&gfx);  //スプライト作成
LGFX_Sprite sprite3(&gfx);  //スプライト作成

BluetoothSerial SerialBT;     //Bluetoothシリアルのインスタンス作成
Sensor startSensor;


std::string IRcmd = "";
  /* シリアルデバッグを有効化するならここ */
bool SerialDebug = true;                        //シリアルデバッグモード

/* ********************************************************* */
/* *********** ボードセットアップここから **********************/
/* ********************************************************* */

uint8_t send_buf[8]= {0x7E, 0xFF, 0x06, 0x01, 0x00, 0x00, 0x00, 0xEF};

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
  //attachInterrupt(digitalPinToInterrupt(RESET_BUTTON_PIN),handleResetButton,FALLING);

  Serial.begin(115200);                   // Start Serial at 115200bps
  Serial2.begin(9600, SERIAL_8N1, 18, 19);
  Serial.printf("Serial2 Initializing...\n");
  delay(3000);
    send_buf[3] = 0x06;
    send_buf[5] = 0;
    send_buf[6] = 5;
    Serial2.write(send_buf,8);
    delay(500);

  Wire.begin(I2C_SDA,I2C_SCL);            // Start I2C library

  Serial.printf("heap_caps_get_free_size(MALLOC_CAP_DMA):%d\n", heap_caps_get_free_size(MALLOC_CAP_DMA) );                      //メモリ確認
  Serial.printf("heap_caps_get_largest_free_block(MALLOC_CAP_DMA):%d\n", heap_caps_get_largest_free_block(MALLOC_CAP_DMA) );    //メモリ確認
  gfx.init();           // Start LovyanGFX
  gfx.fillScreen(TFT_BLACK);
  sprite1.setPsram(true);

  SerialBT.begin("MockcarRACETimer");               //この名前でBluetoothの一覧に出てくる
  Serial.println("Bluetooth Start!");
  Serial.println("I/O complete");

  //初期化もろもろ
  systemState.race.raceFlag = false;          //レース中判定
  systemState.race.startTime = 0;             //スタートタイム保持
  systemState.race.goalCount = 0;             //ゴールカウンター（１レース毎）
  int boardOPmode = 1;                            //ボード動作モード　0=NORMAL,1=LEGACY,2=OPTIONAL（当分レガシーモードのみ）
  
    for (int i = 0; i < 3; i++) {
        systemState.race.timers[i].stopTime = 0;    //レーン毎の停止時間
        systemState.race.timers[i].isTiming = false;    //タイマー稼働中判定
        systemState.race.goalSensors[i].isActive = false;     //ゴールセンサーアクティブ判定
        systemState.race.goalSensors[i].lastTriggerTime = 0;  //ゴールセンサーの時間
    }
  systemState.config.setupMode = false;       //設定モードにいるか判定
  systemState.config.selectedMenuItem = 0;    //設定モードのメニュー選択用

  rmt_config_t rmtConfig;                         //赤外線受信クラス定義
  rmtConfig.rmt_mode = RMT_MODE_RX;               //受信モード
  rmtConfig.channel = RMT_CHANNEL_0;              //０で初期化
  rmtConfig.clk_div = 80;                         //RMTのクロック分周
  rmtConfig.gpio_num = GPIO_NUM_21;               //
  rmtConfig.mem_block_num = 4;                     // メモリブロック数(1-255:1ブロックあたり64ペアの送受信)

  rmtConfig.rx_config.filter_en = true;           //フィルターEnable
  rmtConfig.rx_config.filter_ticks_thresh = 100;  //1ticks 255us 以下の信号を除外
  rmtConfig.rx_config.idle_threshold = 10000;     //10000us以上の信号を除外

  rmt_config(&rmtConfig);
  rmt_driver_install(rmtConfig.channel,2048,0);     //RMT Ring Buffer 1024byte
  rmt_get_ringbuf_handle(RMT_CHANNEL_0, &IRbuffer); //リングバッファ設定
  rmt_rx_start(RMT_CHANNEL_0, true);


  //VL6180Xセンサー初期化してたところ

  SerialBT.println("setup function finished.");
  Serial.println("setup function finished");
  
  resetRaceState();               //変数初期化
  delay(100);                     //
  eeprom_initialize();              //ブート回数記録・EEPROM読み出し/書き込み
  displaySplashScreen();          //起動時画面読み込み

  Serial.printf("heap_caps_get_free_size(MALLOC_CAP_DMA):%d\n", heap_caps_get_free_size(MALLOC_CAP_DMA) );
  Serial.printf("heap_caps_get_largest_free_block(MALLOC_CAP_DMA):%d\n", heap_caps_get_largest_free_block(MALLOC_CAP_DMA) );

  delay(500);                    //デバッグ用　取ってOK
  digitalWrite(BULTIN_LED,LOW);  //内蔵LED OFF
  time_t time_booted;
  struct tm* tm_local;
  char s_time[100];
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
        gfx.fillScreen(TFT_BLACK);  
        drawIdleScreen();           // 初期画面を描画
        initializeHistory();        //レースヒストリー初期化
        firstRun = false;           // 初期フラグを解除
        Serial.println("First Run Complete.");
            digitalWrite(LED_GREEN,LOW);
    }

      digitalWrite(LED_GREEN,HIGH);
    // センサーやボタンの監視
    checkStartSensor();     // スタートセンサーの状態を確認
    checkResetButton();     // リセットボタンが押されたかどうか・ボタン自体は割り込み
    updateButtonStates();   // ボタンの状態を更新（設定ボタン・上・下）

    updateDisplay();        //タイマーと描画関連全般

    if(buttonStates[0].isLongPressed){    // 長押し検知でセットアップモードに遷移
      systemState.config.setupMode = true;
    }

/*
    if(isSensorTriggered()){
      if (SerialDebug)
      {
      Serial.println("sensor TRIG!!");
      }
      
      startRace();
    }
*/
      //printf("reset=%d",digitalRead(RESET_BUTTON_PIN));

      digitalWrite(LED_GREEN,LOW);
      if(digitalRead(RESET_BUTTON_PIN) == LOW){
        digitalWrite(LED_BLUE,HIGH);
      }else{
        digitalWrite(LED_BLUE,LOW);
      }

      delay(50);

    if(!systemState.race.raceFlag){
      ReceiveIR(systemState);
      //Analyze_IR();
    }

    if(systemState.ir_state.isReceived){
      if(systemState.ir_state.enterButton){
        systemState.ir_state.enterButton = false;
        Serial.println("Enter Button Pressed");
        if(!systemState.config.setupMode){
          handleConfigMenu();
        }
      }
      if(systemState.ir_state.menuButton){
          systemState.ir_state.menuButton = false;
        Serial.println("Random Play");

      }
      systemState.ir_state.isReceived = false;
    }

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
/*
void ReceiveIR(SystemState &systemState) {
    size_t rxSize = 0;
    rmt_data_t *item = (rmt_data_t *)xRingbufferReceive(IRbuffer, &rxSize, 500); // タイムアウト短縮

    if (item) {
        uint8_t receive_data[64];
        uint8_t byte_count = 0;

        for (uint16_t i = 0; i < rxSize / sizeof(rmt_data_t); i++) {
            if (i == 0) continue; // 無効データはスキップ

            int duty = (item[i].duration1 * 1000) / item[i].duration0; // 固定小数点
            int8_t bit0 = (duty >= 700 && duty <= 1300) ? 0 :
                          (duty >= 2100 && duty <= 3600) ? 1 : -1;

            if (bit0 < 0) continue;

            uint16_t bit_position = (i - 1) % 8;
            receive_data[byte_count] |= bit0 << bit_position;

            if (bit_position == 7) {
                byte_count++;
                if (byte_count >= sizeof(receive_data)) break;
            }
        }

        if (byte_count >= 4 && receive_data[0] == 0xEE && receive_data[1] == 0x87) {
            const uint8_t command = receive_data[2];
            systemState.ir_state.isReceived = true;
            systemState.ir_state.lastReceiveTime = millis();

            switch (command) {
                case 0x04: systemState.ir_state.enterButton = true; break;
                case 0x08: systemState.ir_state.leftButton = true; break;
                case 0x07: systemState.ir_state.rightButton = true; break;
                case 0x0B: systemState.ir_state.upButton = true; break;
                case 0x0D: systemState.ir_state.downButton = true; break;
                case 0x02: systemState.ir_state.menuButton = true; break;
                case 0x5E: systemState.ir_state.enterButton = true; break;
                default: printf("[IR] Unknown command: 0x%02X\n", command); break;
            }
        }
        vRingbufferReturnItem(IRbuffer, (void *)item);
    }

    // タイムアウトによるリセット処理
    if (systemState.ir_state.isReceived &&
        millis() - systemState.ir_state.lastReceiveTime > 300) {
        systemState.ir_state = {}; // 状態をリセット
        printf("[IR] Button states reset.\n");
    }
}

*/

void ReceiveIR(SystemState &systemState) {
    size_t rxSize = 0;
    rmt_data_t *item = (rmt_data_t *)xRingbufferReceive(IRbuffer, &rxSize, 500);

    if (item) {
        uint8_t receive_data[64];
        uint8_t byte_count = 0;

        for (uint16_t i = 0; i < rxSize / sizeof(rmt_data_t); i++) {
            // 赤外線信号をデコード
            uint8_t byte_data;
            float duty = (float)item[i].duration1 / item[i].duration0;
            int8_t bit0 = (duty >= 0.7 && duty <= 1.3) ? 0 :
                          (duty >= 2.1 && duty <= 3.6) ? 1 : -1;

            if (i == 0 || bit0 < 0) continue;

            uint16_t bit_position = (i - 1) % 8;
            if (bit_position == 0) {
                byte_data = 0;
            }
            byte_data |= bit0 << bit_position;

            if (bit_position == 7) {
                receive_data[byte_count++] = byte_data;
            }
        }

        // 赤外線データをシステムの状態に反映
        if (byte_count >= 4) {
        // receive_data[0] と receive_data[1] の共通部分を確認
        if (receive_data[0] == 0xEE && receive_data[1] == 0x87) {
            systemState.ir_state.isReceived = true;
            systemState.ir_state.lastReceiveTime = millis(); // 受信時刻を記録
            // receive_data[2] の値に応じて分岐
            switch (receive_data[2]) {
                case 0x04:
                    printf("[IR] Execute\n");
                    systemState.ir_state.enterButton = true;
                    break;
                case 0x08:
                    printf("[IR] LEFT BUTTON\n");
                    systemState.ir_state.leftButton = true;
                    break;
                case 0x07:
                    printf("[IR] RIGHT BUTTON\n");
                    systemState.ir_state.rightButton = true;
                    break;
                case 0x0B:
                    printf("[IR] UP BUTTON\n");
                    systemState.ir_state.upButton = true;
                    break;
                case 0x0D:
                    printf("[IR] DOWN BUTTON\n");
                    systemState.ir_state.downButton = true;
                    break;
                case 0x02:
                    printf("[IR] MENU BUTTON\n");
                    systemState.ir_state.menuButton = true;
                    break;
                case 0x5E:
                    printf("[IR] ENTER BUTTON\n");
                    systemState.ir_state.enterButton = true;
                    break;
                default:
                    printf("[IR] Unknown command: 0x%02X\n", receive_data[2]);
                    break;
            }
        } else {
            printf("[IR] Invalid header: 0x%02X 0x%02X\n", receive_data[0], receive_data[1]);
        }

        }
      vRingbufferReturnItem(IRbuffer, (void *)item);
    } else {
        systemState.ir_state.isReceived = false; // 信号が受信されていない
        
    }

        const unsigned long timeout = 300; // ボタン状態のリセットまでの時間 (ミリ秒)
        unsigned long currentTime = millis();
     if (systemState.ir_state.isReceived &&
        (currentTime - systemState.ir_state.lastReceiveTime > timeout)) {
        // 各ボタンをリセット
        systemState.ir_state.isReceived = false;
        systemState.ir_state.enterButton = false;
        systemState.ir_state.leftButton = false;
        systemState.ir_state.rightButton = false;
        systemState.ir_state.upButton = false;
        systemState.ir_state.downButton = false;

        Serial.printf("[IR] All button states reset due to timeout.\n");
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

