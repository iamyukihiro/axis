# axis 仕様まとめ

## 概要

`axis` は、JUCE / C++ で実装したステレオ VST3 プラグインです。  
センター成分を前へ出しつつ、サイドの密度と広がりを少数のマクロで操作することを目的にしています。

- フォーマット: `VST3`
- 入出力: `Stereo In / Stereo Out`
- 実装基盤: `JUCE`
- パラメータ管理: `AudioProcessorValueTreeState`

## 内部処理

入力信号 `L / R` を Mid / Side に変換して処理します。

```text
Mid  = (L + R) * 0.5
Side = (L - R) * 0.5
```

その後、各パラメータを適用します。

```text
Mid  *= Center
Side -> Density -> Width
```

`Density` は Side 成分に対するパラレルコンプレッションです。  
内部では RMS に追従するしきい値を使い、Ratio `4:1`、Attack `10 ms`、Release `100 ms`、Soft Knee で圧縮します。

最後に L/R へ戻します。

```text
L = Mid + Side
R = Mid - Side
```

その後、`Auto Trim` で過度なラウドネス上昇を抑え、最終段で `Output` と `Soft Clip` を適用します。

## パラメータ

### 1. Center
- 範囲: `-24.0 dB` から `+12.0 dB`
- デフォルト: `0.0 dB`
- 役割: Mid 成分のみを増減

### 2. Density
- 範囲: `0 %` から `100 %`
- デフォルト: `0 %`
- 役割: Side 成分へ RMS 追従のパラレルコンプレッションを適用して空間の密度を増す

### 3. Width
- 範囲: `0 %` から `200 %`
- デフォルト: `100 %`
- 役割: Side 成分の量をスケール
- 備考:
  - `0 %` = 実質モノ
  - `100 %` = 原音相当
  - `200 %` = Side 成分を 2 倍

### 4. Output
- 範囲: `-24.0 dB` から `+12.0 dB`
- デフォルト: `0.0 dB`
- 役割: 最終出力ゲイン

## GUI

GUI はシンプルな縦スライダー + ボタン構成です。

### スライダー
- Center
- Density
- Width
- Output

### ボタン
- Reset

### Reset
- 全パラメータをデフォルト値へ戻す
- ホスト通知付きでリセット

## メーター

出力メーターを 2 本搭載しています。

- 表示対象: `Output L / R`
- 種類: ピークメーター
- 更新: GUI タイマーで追従
- 表示色:
  - `-12 dBFS` 未満: 緑
  - `-12 dBFS` 以上: 黄
  - `-1 dBFS` 以上: 赤

## 表示仕様

- dB 系パラメータは `-6.0 dB` のように表示
- `Density` と `Width` は `%` 表示

## ビルド / インストール

### ビルド
```bash
cmake -B build -DJUCE_DIR=/Users/iamyukihiro/work/axis/JUCE
cmake --build build --config Release
```

### 生成物
```text
build/AxisCenter_artefacts/VST3/axis.vst3
```

### 自動インストール
macOS ではビルド完了後、自動で以下へ上書き配置されます。

```text
~/Library/Audio/Plug-Ins/VST3/axis.vst3
```

## 現在の補足

- macOS 前提構成
- Docker 構成なし
- `center`, `density`, `width`, `output` の 4 パラメータ構成
- `Density` は Side のみを処理し、モノラル和算時には Side 成分が打ち消される
