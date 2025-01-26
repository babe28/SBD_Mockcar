# ESP32-mockcar-racetimer
### ESP32を使った、ソープボックスダービー協会のモックカーレース用レースタイマープログラム


## 概要
このプロジェクトは、ESP32を使ったタイマーアプリケーションのプロトタイプです。
ミニカーレースのタイム計測や記録を簡単に行うことを目的としています。

### ピン設定
- ESP32 WROVERの場合
- VPピン(36) リセットボタン

- 27 スタートボタン
- 13 LED
- 15 LED
- 12 ピンヘッダ出力
- 14 ピンヘッダ出力

- 26 NTSC出力（ビデオ）
- 33 スタートセンサー
- 32 ゴールセンサー1
- 35 ゴールセンサー2
- 34 ゴールセンサー3

- 18 シリアル
- 19 シリアル
- 21 IR受信
- 22 I2C
- 23 I2C

---

### **5. 使い方 (Usage)**
- ユーザーがプロジェクトをどのように利用するかを説明します。
- スクリーンショットやGIFを追加すると分かりやすいです。

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