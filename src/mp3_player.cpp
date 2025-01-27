/* *******************************
 * DFPlayer用ルーチン
 ******************************** */

#include "main.hpp"

uint8_t send_buffer[8]= {0x7E, 0xFF, 0x06, 0x01, 0x00, 0x00, 0x00, 0xEF};


//DFPlayerの初期化
void initializeDFPlayer() {

    Serial.println("DFPlayer initializing...");

    send_buffer[3] = 0x06;  //command
    send_buffer[5] = 0;     //data
    send_buffer[6] = 5;         //data
    Serial2.write(send_buffer,8);

    delay(1000); // DFPlayerの初期化には時間がかかる

     //タイムアウト設定

    delay(1000); //初期化待機

}

void playMP3(int track) {

    delay(1000); //再生開始まで待機

}

void stopMP3() {

}

void pauseMP3() {

}

void resumeMP3() {

}

void nextMP3() {

}

void prevMP3() {

}

void volumeUpMP3() {

}

void volumeDownMP3() {

}

void setVolumeMP3(int volume) {

}

