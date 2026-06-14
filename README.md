# Axis

JUCE/C++で実装した、M/Sベースの音像コントロール用ステレオVST3プラグインです。

## スクリーンショット

![Axis GUI](plugin-gui.png)

## 機能

- `Input` で M/S 処理前の入力レベルを調整
- ステレオ入力 / ステレオ出力
- `Center` で Mid 成分を前に出す
- `Side Gain` で Side 成分のレベルを調整
- `Side Density` で Side 成分へ RMS 追従のパラレルコンプレッションを適用
- `Width` でステレオ幅を調整
- `Output` で最終レベルを調整
- `Auto Gain` で処理前後のレベル差を自動補正
- `Soft Clip` を内部で常時適用

## パラメータ

- Input: `-24` から `+12 dB`
- Center: `-24` から `+12 dB`
- Side Gain: `-24` から `+12 dB`
- Side Density: `0` から `100 %`
- Width: `0` から `200 %`
- Output: `-24` から `+12 dB`
- Auto Gain: on/off
- Bypass: on/off

## ビルド

macOSでJUCEを使ってVST3をビルドします。ローカルにJUCEを持っている場合は、`/path/to/JUCE` を実際のJUCEパスに置き換えて次を実行します。

```bash
cmake -B build -DJUCE_DIR=/path/to/JUCE
cmake --build build --config Release
```

または環境変数を使えます:

```bash
export JUCE_DIR=/path/to/JUCE
cmake -B build
cmake --build build --config Release
```

JUCEをローカル配置せず、CMakeに取得させることもできます。

`JUCE_DIR` を指定しない場合、CMakeはJUCEをGitHubから取得します。

```bash
cmake -B build
cmake --build build --config Release
```

VST3は `build/AxisCenter_artefacts` 以下に生成されます。

macOSでは、`AxisCenter_VST3` のビルド完了後に `~/Library/Audio/Plug-Ins/VST3/Axis.vst3` へ自動で上書き配置されます。

## 開発

`clang-format` 用に `.clang-format`、`clang-tidy` 用に `.clang-tidy` を置いています。

```bash
cmake -B build -DJUCE_DIR=/path/to/JUCE
cmake --build build --target format
```

`clang-tidy` が入っている環境では、次も使えます。

```bash
cmake -B build -DJUCE_DIR=/path/to/JUCE
cmake --build build --target tidy
```

macOS の Command Line Tools 環境では、`clang-format` は `xcrun clang-format` 経由でも実行できます。
