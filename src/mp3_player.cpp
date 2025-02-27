/* *******************************
 * DFPlayer用ルーチン
 ******************************** */

#include "main.hpp"

#define PLAY_CMD 0x03
#define STOP_CMD 0x16
#define PAUSE_CMD 0x0E
#define VOLUME_CMD 0x06
#define NEXT_CMD 0x01
#define PREV_CMD 0x02
#define PLAY_RPT 0x00
//以下は再生モード内
#define PLAY_ONE_RPT
#define PLAY_FOLDERONE_RPT 0x01
#define PLAY_FOLDER_RPT 0x02
#define PLAY_ALL_RONDOM 0x03
#define PLAY_MODE_CMD 0x08

//DFPlayerの初期化
void initializeDFPlayer() {
    Serial.println("DFPlayer initializing...");
    sendCommand(0x0C,0); //Reset DFPlayer
    delay(3900); // DFPlayerの初期化には時間がかかる
    if(checkForACK(5000)){
        Serial.println("DFPlayer initializeComplete!!");
    }
    delay(500);
    setVolumeMP3(5); // ボリュームを設定
}

uint16_t calculateChecksum(uint8_t *data, size_t length) {
  uint16_t sum = 0;
  for (size_t i = 0; i < length; i++) {
    sum += data[i];
  }
  return 0xFFFF - sum + 1;
}

void sendCommand(uint8_t command, uint16_t parameter = 0) {
    //コマンドとパラメータを指定すると、DFPlayerに送信
  uint8_t packet[10] = {0x7E, 0xFF, 0x06, command, 0x00,
                        (uint8_t)(parameter >> 8), (uint8_t)(parameter & 0xFF),
                        0x00, 0x00, 0xEF};

  // チェックサムを計算してパケットに追加
  uint16_t checksum = calculateChecksum(&packet[1], 6); // データ部分だけを計算
  packet[7] = (uint8_t)(checksum >> 8);   // チェックサム上位バイト
  packet[8] = (uint8_t)(checksum & 0xFF); // チェックサム下位バイト

  // パケットを送信
  for (int i = 0; i < 10; i++) {
    Serial2.write(packet[i]);
    delayMicroseconds(5);
  }
  Serial.println("DF Player Command sent!");
}

void playMP3(int track = 0) {
    sendCommand(PLAY_CMD, track);
    systemState.race.bgmFlag = true;
    if(SerialDebug){
      Serial.printf("Play Command:%d \n",track);
    }
}

void stopMP3() {
    sendCommand(STOP_CMD);
    systemState.race.bgmFlag = false;
}

void pauseMP3() {
    sendCommand(PAUSE_CMD);
    systemState.race.bgmFlag = false;
}

void resumeMP3() {
    sendCommand(PAUSE_CMD);
    systemState.race.bgmFlag = true;
}

void nextMP3() {

}

void prevMP3() {

}

void isPlaying() {
  sendCommand(0x42);  // 演奏状態を問い合わせる
  delay(100);
  if (Serial2.available()) {
    uint8_t byte = Serial2.read();
    if (byte == 0x01) {// 演奏中
      Serial.println("Playing.");
    } else {
      Serial.println("Not playing.");
    }
  }
}


void setVolumeMP3(int volume = 5) {
    sendCommand(VOLUME_CMD, volume);
    if(SerialDebug){
      Serial.printf("VolumeSet:%d \n",volume);
    }
//    if(checkForACK(1000)){
//        Serial.println("Volume set!");
//    }else{
//        Serial.println("Volume set failed.");
//    }
}


bool checkForACK(unsigned long timeout = 2000) {
  static uint8_t ackBuffer[16]; // ACK格納用バッファ
  static size_t ackIndex = 0;   // バッファ内の現在位置
  unsigned long startTime = millis();

  while (millis() - startTime < timeout) { // タイムアウト時間を監視
    if (Serial2.available()) {
      uint8_t byte = Serial2.read();

      // ACKの開始バイトを見つけたらバッファをリセット
      if (byte == 0x7E) {
        ackIndex = 0;
      }

      // バッファにバイトを格納
      if (ackIndex < sizeof(ackBuffer)) {
        ackBuffer[ackIndex++] = byte;
      }

      // バッファオーバーフローをチェック
      if (ackIndex >= sizeof(ackBuffer)) {
        Serial.println("Buffer overflow detected, resetting.");
        ackIndex = 0; // バッファをリセット
      }

      // ACKの終了バイトを確認
      if (byte == 0xEF && ackIndex == 10) {
        // バッファに格納されたACKを確認
        if (validateACK(ackBuffer, ackIndex)) {
          Serial.println("ACK received and validated!");
          ackIndex = 0; // バッファをリセット
          return true;
        } else {
          Serial.println("Invalid ACK received.");
          ackIndex = 0; // バッファをリセット
          return false;
        }
      }
    }
  }
  return false;
}

void checkDFPlayerResponse() {
    static uint8_t responseBuffer[10]; // 応答バッファ
    static size_t responseIndex = 0;   // バッファ内の現在位置

    while (Serial2.available()) {
        uint8_t byte = Serial2.read();

        // 開始バイトを検出したらバッファをリセット
        if (byte == 0x7E) {
            responseIndex = 0;
        }

        // バッファに格納
        if (responseIndex < sizeof(responseBuffer)) {
            responseBuffer[responseIndex++] = byte;
        }

        // 終了バイトを検出
        if (byte == 0xEF && responseIndex == 10) {
            // コマンド種別を確認
            if (responseBuffer[3] == 0x3E) {
                Serial.println("Playback finished!");
                systemState.race.bgmFlag = false;
            } else {
                Serial.println("Unhandled response received.");
            }
            responseIndex = 0; // バッファをリセット
        }
    }
  // タイムアウトの場合
  Serial.println("ACK check timed out.");
  return;
}

bool validateACK(uint8_t *data, size_t length) {
  // データ長が期待通りであることを確認
  if (length != 10) {
    return false;
  }

  // チェックサムを検証
  uint16_t receivedChecksum = (data[7] << 8) | data[8];
  uint16_t calculatedChecksum = calculateChecksum(&data[1], 6);
  return receivedChecksum == calculatedChecksum;
}

