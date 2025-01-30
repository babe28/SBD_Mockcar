# ESP32-mockcar-racetimer
### ESP32：ソープボックスダービー協会モックカーレース用レースタイマープログラム


## 概要
このプロジェクトは、ESP32を使ったタイマーアプリケーションのプロトタイプです。
ミニカーレースのタイム計測や記録を簡単に行うことを目的としています。

### 注意
このプログラムは、ESP32-WROVERでの使用を想定しています。
・DAC内蔵であること（NTSC出力使用のため）
・PSRAMが使用できること（画面バッファ用 LovyanGFX様ライブラリ）


### ピン設定
- ESP32 WROVERの場合
- VPピン(36) リセットボタン

- 27 スタートボタン　プルアップ抵抗
- 13 LED　青ステータス表示用
- 15 LED　緑メイン関数監視用
- 14 リセットボタン
- 26 NTSC出力（ビデオ）・DAC出力・75Ω終端抵抗
- 33 スタートセンサー
- 32 ゴールセンサー1
- 35 ゴールセンサー2
- 34 ゴールセンサー3

- 18 シリアル（DF Player接続用）
- 19 シリアル（DF Player接続用）
- 21 IR受信（設定画面用・リモコンコードはapple製）
- 22 I2C（RTC用）
- 23 I2C（RTC用）

---

### **5. 使い方 (Usage)**
- 準備中
- モックカーレースのキットに付属します。
- センサーで３台の車両のタイムを計測します

```markdown
## 使い方
1. ESP32にコードをアップロードします。
2. デバイスを起動すると、NTSCディスプレイにタイマーが表示されます。
3. ボタン操作でタイマーをスタート・ストップできます。


## 作者
- 名前: Hiroaki Take
- GitHub: [@babe28](https://github.com/babe28)
- Email: 

## 参考資料
- [ESP32 Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [PlatformIO](https://platformio.org/)