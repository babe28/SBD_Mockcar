/* ********************************************
*  ©日本ソープボックスダービー協会 2025.1.7
*  Last modified 2025.1.13
*  --------------------------------------------
*  メインヘッダーファイル：変数・構造体・クラスを定義しています。
*　本体で設定可能な項目  
*　DFPlayerの音量／アイドル時に音量を下げるかどうか
*　センサーディレイ
*　Bluetoothの有効／非有効化
*　時計情報の設定
*　
*********************************************** */

#ifndef INCLUDED_MAIN_HPP
#define INCLUDED_MAIN_HPP

#include <Arduino.h>                //
#include <esp32-hal.h>              //割り込み関連
#include <BluetoothSerial.h>        //
#include <Preferences.h>            //内蔵EEPROM保存用
#include <Wire.h>                   //I2C
#include <SparkFun_VL6180X.h>       //センサー用
#include <driver/rmt.h>             //赤外線モジュール用
#include <time.h>                   //

#include <DFRobotDFPlayerMini.h>    //シリアルは9600ボー

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#define START_SENS 36         //START模擬ボタン(ignore_PULLUP)
#define GOAL_SENS_1 33        //GOAL模擬ボタン1(ignore_PULLUP)
#define GOAL_SENS_2 34        //GOAL模擬ボタン2(ignore_PULLUP)
#define GOAL_SENS_3 35        //GOAL模擬ボタン3(ignore_PULLUP)

#define ADC_PIN 21              //ADコンバーターテスト用
#define BULTIN_LED 2            //ボード内蔵LED
#define RESET_BUTTON_PIN 32     //reset button
#define SETUP_BUTTON_PIN 13     //setup button(longpress)
#define UP_BUTTON_PIN 12        //setup button up(A)
#define DOWN_BUTTON_PIN 14      //setup button down(B)
#define LONG_PRESS_THR 1500     //長押しスレッショルドタイム
#define RW_MODE false           //データ保存用 bool
#define RO_MODE true            //データ保存用ReadOnly
#define MAX_HISTORY 5           //保存する履歴の最大数
#define STATUSBAR_HEIGHT 18     //ステータスバーの高さ
#define I2C_SDA 22              //センサー用
#define I2C_SCL 23
#define SERIAL_OVR_RX 3
#define SERIAL_OVE_TX 1
#define RMT_CHANNEL_TX RMT_CHANNNEL_0
#define CARRIER_FREQ 38000


#define SCREEN_WIDTH 360        //LovyanGFX用画面サイズ横
#define SCREEN_HEIGHT 240

#define VL6180X_ADDRESS 0x29

// タイマー状態を管理
struct Timer {
    unsigned long stopTime = 0;         // ゴール時刻
    bool isTiming = false;              // タイマー動作中か
};
struct TimerHistory {
    unsigned long times[3];             // 各車両のタイム
    int raceNumber;                     // レース番号
};

// センサー状態を管理
struct Sensor {
    unsigned long lastTriggerTime = 0;  // 最後にトリガーされた時刻
    bool isActive = false;              // センサーがアクティブか
    bool isSense = false;               //きたかどうか
};

// ボタン状態を管理
struct Button {
    unsigned long pressStartTime = 0; // ボタンが押された時刻
    bool isPressed = false;           // ボタンが押されているか
    bool isLongPressed = false;       // 長押しされているか
};


// レース全体の状態
struct Race {
    bool raceFlag = false;           // レース中フラグ
    bool resetFlag = false;         //追加　リセットフラグ
    unsigned long startTime = 0;     // スタート時刻
    int goalCount = 0;               // ゴールした車両数
    Timer timers[3];                 // 各車両のタイマー
    Sensor startSensor;              // スタートセンサー
    Sensor goalSensors[3];           // ゴールセンサー
    int totalRaceCount = 0;         //レース回数
};

// 設定メニューの状態
struct Config {
    bool setupMode = false;          // 設定モード中フラグ
    bool HistoryMode = false;       //ヒストリー
    bool isEditing = false;          // 現在の項目を編集中かどうか
    bool BTmode = false;
    int selectedMenuItem = 0;        // 現在選択中のメニュー項目
    float sensorGainStart = 1.0f;    // スタートセンサーのゲイン
    float sensorGainGoal = 1.0f;     // ゴールセンサーのゲイン
    int boardOPMode = 0;            //ボード動作モード
    int bgmVolume = 0;              //Volume
    bool bgmDucking  = false;       //BGMをアイドル時にダッキングするかどうか。
    unsigned long bestTime = 0;      // ボード最速タイム
    int oncycle = 0;
};


// レース履歴
struct History {
    unsigned long times[3] = {0};      // 各車両のタイム
};

// システム全体
struct SystemState {
    public:
    Race race;                       // レース関連
    Config config;                   // 設定関連
    TimerHistory history[8];         // 最大8回分の履歴
    int currentHistoryIndex = 0;     // 最新履歴のインデックス
    Button buttons[5];               // ボタン
};



// グローバルシステム状態
extern SystemState systemState;

// ゴールセンサーのピン番号を配列で管理
const int GOAL_SENS_PINS[3] = {GOAL_SENS_1, GOAL_SENS_2, GOAL_SENS_3};
//ボタンピン設定
const int buttonPins[3] = {SETUP_BUTTON_PIN, UP_BUTTON_PIN, DOWN_BUTTON_PIN};
extern Button buttonStates[3];

extern bool SerialDebug;               //シリアルデバッグモード


class LGFX : public lgfx::LGFX_Device {
public:
  LGFX();
  lgfx::Panel_CVBS _panel_instance;
};

extern LGFX gfx;                //インスタンス名gfx
extern LGFX_Sprite sprite1;     //スプライト作成

extern VL6180x start_Sensor;
extern DFRobotDFPlayerMini mp3;


extern const lgfx::U8g2font myFont;

//graphic.cpp
void displaySplashScreen();     //起動時の画面処理
void updateDisplay();           //どの画面を描画するかを選択する
void drawRaceScreen();          //レース画面の下地
void drawIdleScreen();          //前回レースの結果表示または待機状態
void updateTimers();                // タイマーの更新と表示
void drawRaceHistory();         //
void drawStatusBar(int racecount,String message,String statusMode,unsigned long time_fastest);
void updateMovingBars();
void clearDisplay();
void setFontNormal(double fontSize = 1.0);     //フォントサイズいける色も一応初期化される
void setFontJapan(double fontSize = 1.0);
void printCentering(int x,int y,String printText);
void raceSignal();              //レースシグナル


//io_others.cpp
void stopTimer(int timerId);
void updateButtonStates();
bool isSensorTriggered(Sensor &sensor);
void disableGoalSensorInterrupts();
void enableGoalSensorInterrupts();

//main.cpp
void checkStartSensor();            // スタートセンサーの監視
void checkGoalSensors();            // ゴールセンサーの監視
void updateRaceTimers();
void checkResetButton();
void clearRaceHistory();
void startRace();
void resetTimers();
void addRaceHistory(unsigned long carTimes[], int raceNumber);
void endRace();
void printIdentification(struct VL6180xIdentification *temp);
void initializeHistory();
void ReceiveIR();


// 設定メニュー関連
void handleConfigMenu();       // 設定メニューの管理
void clearRaceHistory();        //履歴削除

// ディスプレイ更新
void updateDisplay();          // ディスプレイの更新

//IRAM main.cpp

void IRAM_ATTR goalSensorISR1();
void IRAM_ATTR goalSensorISR2();
void IRAM_ATTR goalSensorISR3();
void IRAM_ATTR handleResetButton();


void eeprom_initialize();
void resetRaceState();


enum class DisplayState {
    Idle,    // 待機画面
    Setup,   // 設定画面
    Racing,   // レース画面
    History     //履歴画面（設定画面内の想定）
};

extern const unsigned char img[];


#endif

extern bool firstrun;                 //起動後初回か判定
extern int goalcount;                    //ゴール通過台数
extern int raceTotalCount;               //起動後何回レースしたか
extern int menuSelector;


extern unsigned long buttonPressStart;   // ボタンが押された時刻を記録（長押し判定用）
extern bool isButtonPressed;         // ボタンが押されているか
extern bool inSetupMode;             // セットアップモードかどうか

extern int boardOPmode;                  //ボード動作モード　0=NORMAL,1=LEGACY,2=OPTIONAL（当分レガシーモードのみ）

// 描画用ステータス
extern unsigned long lastUpdateTime; // 前回更新時刻
extern const int refreshRate;       // リフレッシュレート（Hz）
extern const unsigned long updateInterval; // 更新間隔（ms）


  //設定保存用（EEPROMの後継ライブラリPreferences）
  //まずは変数定義
  extern int on_cycle; //起動回数
  extern unsigned long best_time_onboard; //ボード最速タイム
  extern float sensor_gain_start; //センサーのゲイン（スタート）
  extern float sensor_gain_goal; //センサーゲイン（ゴール）