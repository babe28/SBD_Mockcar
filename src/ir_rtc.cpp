
#include "main.hpp"


volatile uint8_t REG_table[8] = {0};                 //時間テーブルRTC
struct tm timeinfo;                   //内蔵RTC用時刻構造体
const char *week[] = {"SUN","MON","TUE","WED","THR","FRI","SAT","NON"};


void rtc_initialize(){
  //RTC初期化
  byte err;
  char data_read_buf[16];
  Wire.beginTransmission(DS1307_ADDRESS);
  delay(1);
  Wire.write(0x00); //START_REGISTOR
  delay(1);
  err  = Wire.endTransmission();
  if(err != 0){
    Serial.printf("RTC Read Error:%d\n",err);
    return;
  }else{
  Wire.requestFrom(DS1307_ADDRESS,8);

  for (int ii = 0; ii < 8; ii++) {
      while (Wire.available() == 0 ) {}
      data_read_buf[ii] = Wire.read();
    }
    for(int i = 0; i < 7; i++){
      REG_table[i]=data_read_buf[i];
    }
  }
/*    Serial.printf("DS1307 is 20%02X/%02X/%02X (%s) %02X:%02X:%02X\n",
                  REG_table[6],              // 年 (16進数形式)
                  REG_table[5],              // 月 (16進数形式)
                  REG_table[4],              // 日 (16進数形式)
                  week[REG_table[3]],    // 曜日 (インデックス調整)
                  REG_table[2],              // 時 (16進数形式)
                  REG_table[1],              // 分 (16進数形式)
                  REG_table[0]);             // 秒 (16進数形式)
  }*/
     if(REG_table[6] == 0x00 || REG_table[6] == 0xFF){
      Serial.println("RTC is not working.");
    }
 Serial.printf("DS1307 is 20%02d/%02d/%02d (%s) %02d:%02d:%02d\n",
              bcdToDec(REG_table[6]),   // 年
              bcdToDec(REG_table[5]),   // 月
              bcdToDec(REG_table[4]),   // 日
              week[REG_table[3]],       // 曜日
              bcdToDec(REG_table[2]),   // 時
              bcdToDec(REG_table[1]),   // 分
              bcdToDec(REG_table[0]));  // 秒

}

void rtcTimeSet(){
  byte err;
  //DS1307RTC 強制時間設定
  Wire.beginTransmission(DS1307_ADDRESS);

  Wire.write(0x00); //START_REGISTOR

  Wire.write(0x00); //秒
  Wire.write(0x45); //分
  Wire.write(0x11); //時
  Wire.write(0x01); //週(SUN 00 MON 01 TUE 02)
  Wire.write(0x03); //日
  Wire.write(0x02); //月
  Wire.write(0x25); //年 20xx年
//  delay(1);
  err = Wire.endTransmission();
  if(err!=0){
    Serial.printf("RTC Write Error:%d\n",err);
  }else{
    Serial.printf("\nRTC DATE WRITE\n");
  }
}

bool rtc_read(){
  byte err;
  //RTC読み出し
  Wire.beginTransmission(DS1307_ADDRESS);
  delay(1);
  Wire.write(0x00); //START_REGISTOR
  err  = Wire.endTransmission();
  delay(1);
  if(err != 0){
    Serial.printf("RTC Read Error:%d\n",err);
    return false;
  }

  Wire.requestFrom(DS1307_ADDRESS,7);
  delay(10);
  REG_table[0] = Wire.read(); //秒
  REG_table[1] = Wire.read(); //分
  REG_table[2] = Wire.read(); //時
  REG_table[3] = Wire.read(); //曜日
  REG_table[4] = Wire.read(); //日
  REG_table[5] = Wire.read(); //月
  REG_table[6] = Wire.read(); //年

  Serial.printf("ExternalRTC:20%02X/%02X/%02X  %02X:%02X:%02X\n",
                REG_table[6], // 年 (16進数形式)
                REG_table[5], // 月 (16進数形式)
                REG_table[4], // 日 (16進数形式)
                REG_table[2], // 時 (16進数形式)
                REG_table[1], // 分 (16進数形式)
                REG_table[0]); // 秒 (16進数形式)

  // RTCが動作していない場合の処理
  if(REG_table[6] == 0x00 || REG_table[6] == 0xFF){
    Serial.println("RTC is not working.");
    return false;
  }

  // 年が 2025 (0x25) 未満の場合、RTCを2025年1月1日に設定
  if(REG_table[6] < 0x25){
    Serial.println("RTC year is before 2025. Setting to 2025/01/01.");
    rtcTimeSet();
    return false;  // RTC設定後は再度読み込む必要があるためfalseを返す
  }

  return true;
}

void setInternalRTC() {
    timeinfo.tm_year = bcdToDec(REG_table[6]) + 2000 - 1900; // 年（1900年基準）
    timeinfo.tm_mon = bcdToDec(REG_table[5]) - 1;            // 月（0-11）
    timeinfo.tm_mday = bcdToDec(REG_table[4]);               // 日
    timeinfo.tm_hour = bcdToDec(REG_table[2]);               // 時
    timeinfo.tm_min = bcdToDec(REG_table[1]);                // 分
    timeinfo.tm_sec = bcdToDec(REG_table[0]);                // 秒

    timeinfo.tm_isdst = -1;                                  // サマータイム情報を無効化
    /*
    timeinfo.tm_year = REG_table[6] + 2000 - 1900;
    timeinfo.tm_mon = REG_table[5] - 1;
    timeinfo.tm_mday = REG_table[4];
    timeinfo.tm_hour = REG_table[2];
    timeinfo.tm_min = REG_table[1];
    timeinfo.tm_sec = REG_table[0];
    */
    Serial.println("RTC SET Internal");
    struct timeval now = {mktime(&timeinfo), 0};
    settimeofday(&now, NULL); // ESP32の内蔵RTCに時刻を設定
}
void updateInternalRtc(struct tm* tm) {
    time_t t = mktime(tm); // struct tm を time_t に変換
    struct timeval now = { .tv_sec = t, .tv_usec = 0 };
    settimeofday(&now, NULL); // ESP32内蔵RTCを更新
    Serial.println("Internal RTC updated!");
}

void updateExternalRtc(struct tm* tm) {
   // 月は 0-11 なので +1 する
    int adjustedMonth = tm->tm_mon + 1;
    // 0日は無効なので、1日に補正
    int adjustedDay = (tm->tm_mday < 1) ? 1 : tm->tm_mday;
    int adjustedWday = (tm->tm_wday == 0) ? 7 : tm->tm_wday; // tm_wday(0=日曜)をDS1307形式(1=日曜)に変換


    // **デバッグ出力（送信するデータを確認）**
    Serial.println("=== Updating External RTC (DS1307) ===");
    Serial.printf("Internal RTC: %04d/%02d/%02d %02d:%02d:%02d\n",
                  tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                  tm->tm_hour, tm->tm_min, tm->tm_sec);
    Serial.printf("Writing to DS1307: %04d/%02d/%02d %02d:%02d:%02d\n",
                  tm->tm_year + 1900, adjustedMonth, adjustedDay,
                  tm->tm_hour, tm->tm_min, tm->tm_sec);

    Wire.beginTransmission(DS1307_ADDRESS);
    Wire.write(0x00); // レジスタの先頭アドレス
    Wire.write(decToBcd(tm->tm_sec));   // 秒
    Wire.write(decToBcd(tm->tm_min));   // 分
    Wire.write(decToBcd(tm->tm_hour));  // 時 (BCD format)
        Wire.write(decToBcd(adjustedWday)); // **曜日（0=日曜→1=日曜に変換済み）**
    Wire.write(decToBcd(adjustedDay));  // 日
    Wire.write(decToBcd(adjustedMonth)); // 月（tm_monは0-11）
    Wire.write(decToBcd(tm->tm_year - 100)); // 年 (1900年基準から2000年基準に変換)

    int error = Wire.endTransmission(); // 送信終了 & エラーチェック
    if (error == 0) {
        Serial.println("OK External RTC updated successfully!");
    } else {
        Serial.printf("ERR I2C Write Error! Code: %d\n", error);
    }
    Serial.println("External RTC updated!");
}

int bcdToDec(uint8_t val) {
    return ((val >> 4) * 10) + (val & 0x0F);
}
int decToInt(int dec){
  int data;
  data = (dec >> 4 ) * 10 + (dec & 0x0F);
  return data;
}
byte decToBcd(int val) {
    return (val / 10 * 16) + (val % 10);
}

void readInternalRTC() {
    Serial.println("readInternalRTC exec.");
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return;
    }
    Serial.printf("Internal RTC: %04d/%02d/%02d %02d:%02d:%02d\n",
                  timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                  timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}




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
                case 0x5D:
                    if(SerialDebug){Serial.printf("[IR] Enter Button\n");}
                    systemState.ir_state.enterButton = true;
                    break;
                case 0x08:
                    if(SerialDebug){printf("[IR] LEFT BUTTON\n");}
                    systemState.ir_state.leftButton = true;
                    break;
                case 0x07:
                    if(SerialDebug){printf("[IR] RIGHT BUTTON\n");}
                    systemState.ir_state.rightButton = true;
                    break;
                case 0x0B:
                    if(SerialDebug){printf("[IR] UP BUTTON\n");}
                    systemState.ir_state.upButton = true;
                    break;
                case 0x0D:
                    if(SerialDebug){printf("[IR] DOWN BUTTON\n");}
                    systemState.ir_state.downButton = true;
                    break;
                case 0x02:
                    printf("[IR] MENU BUTTON\n");
                    systemState.ir_state.menuButton = true;
                    break;
                case 0x5E:
                    printf("[IR] PLAY BUTTON\n");
                    systemState.ir_state.playButton = true;
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

/*

void Analyze_IR(){
  uint8_t receive_data[64],byte_count=0;
  size_t rxSize = 0;
  rmt_data_t *item = (rmt_data_t *)xRingbufferReceive(IRbuffer, &rxSize,2000);

  if(item){
    uint16_t t_base = item[0].duration0/8;
    //Serial.printf("%d items received.\n",rxSize/sizeof(rmt_data_t));
    //Serial.printf("T = %d\n",t_base);

    //Serial.printf("#  :{dur0,L,dur1,L} duty   :bit\n");
    for(uint16_t i=0;i<rxSize/sizeof(rmt_data_t); i++){
      uint8_t byte_data;
      float duty = (float)item[i].duration1 / item[i].duration0;
      int8_t bit0 = 1;
      if((duty >= 0.7) && (duty <= 1.3)){
          bit0=0;
      }
      else if((duty >= 2.1) &&(duty <= 3.6)){
          bit0 = 1;
      }
      //Serial.printf("%3d : {%4d,1,%4d,0} %4.2f : %2d \n",i,item[i].duration0,item[i].duration1,duty,bit0);

      if(i == 0) continue;
      if(bit0 < 0){
        if(item[i].duration1 !=0){
          Serial.printf("Receive illegular Signal.\n");
        }
        break;
      }

    uint16_t bit_possition = (i-1) % 8;
    if(bit_possition == 0){
      byte_data = 0;
    }
    byte_data += bit0 << bit_possition;
      if(bit_possition == 7){
        receive_data[byte_count++] = byte_data;
        byte_data = 0;
      }
    }

    Serial.printf("Decode data:");
    for(uint8_t i=0; i<byte_count; i++){
      Serial.printf("%02X \n",receive_data[i]);
    }

        // 一致するパターンを判定
        if (byte_count >= 4) {  // パターン比較に必要なデータがある場合のみ
            if (receive_data[0] == 0xEE && receive_data[1] == 0x87 &&
                receive_data[2] == 0x04 && receive_data[3] == 0x65) {
                printf("[IR] Execute\n");
            }
            if (receive_data[0] == 0xEE && receive_data[1] == 0x87 &&
                receive_data[2] == 0x08 && receive_data[3] == 0x65) {
                printf("[IR] LEFT BUTTON\n");
            }
            if(receive_data[0] == 0xEE && receive_data[1] == 0x87 &&
                receive_data[2] == 0x07 && receive_data[3] == 0x65) {
                printf("[IR] RIGHT BUTTONn\n");
            }
            if(receive_data[0] == 0xEE && receive_data[1] == 0x87 &&
                receive_data[2] == 0x0B && receive_data[3] == 0x65) {
                    printf("[IR] UP BUTTON\n");
            }
            if(receive_data[0] == 0xEE && receive_data[1] == 0x87 &&
                receive_data[2] == 0x0D && receive_data[3] == 0x65) {
                    printf("[IR] DOWN BUTTON\n");
            } 
         }else {
            printf("[IR] Insufficient data for pattern matching\n");
        }
    printf("\n");
// MENU BUTTON
//EE 87 02 65
//saisei buttton
//EE 87 04 65
//再生と決定ボタンは２回送信されている]
//押しっぱなしでリピート信号が出る
    vRingbufferReturnItem(IRbuffer, (void*) item);
  }//if(item)end

}//RECEIVEIR end
*/