/* *******************************
 * DFPlayer用ルーチン
 ******************************** */

#include "main.hpp"

DFRobotDFPlayerMini mp3;

//DFPlayerの初期化
void initializeDFPlayer() {

    Serial.println("DFPlayer initializing...");
    delay(1000); // DFPlayerの初期化には時間がかかる
    if (!mp3.begin(Serial2)) {
        Serial.println("DFPlayer error");
        delay(2500);
        if (!mp3.begin(Serial2)) {
        Serial.println("DFPlayer error 2nd");
        return;
    }else{
        Serial.println("DFPlayer OK");
    }
        return;
    }else{
        Serial.println("DFPlayer OK");
    }    
    mp3.setTimeOut(800); //タイムアウト設定
    mp3.volume(10); //音量設定
    mp3.EQ(DFPLAYER_EQ_NORMAL); //イコライザ設定
    delay(1000); //初期化待機
    mp3.outputDevice(DFPLAYER_DEVICE_SD); //出力先設定
    delay(1000); //初期化待機
    mp3.sleep(); //スリープモード
}

void playMP3(int track) {
    mp3.playMp3Folder(track);
    delay(1000); //再生開始まで待機
    mp3.start(); //再生開始
}

void stopMP3() {
    mp3.stop(); //再生停止
    mp3.sleep(); //スリープモード
}

void pauseMP3() {
    mp3.pause(); //一時停止
}

void resumeMP3() {
    mp3.start(); //再生再開
}

void nextMP3() {
    mp3.next(); //次の曲へ
}

void prevMP3() {
    mp3.previous(); //前の曲へ
}

void volumeUpMP3() {
    mp3.volumeUp(); //音量アップ
}

void volumeDownMP3() {
    mp3.volumeDown(); //音量ダウン
}

void setVolumeMP3(int volume) {
    mp3.volume(volume); //音量設定
}

