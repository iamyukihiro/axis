# Axis

JUCE/C++で実装した、M/Sベースの音像コントロール用ステレオVST3プラグインです。

## スクリーンショット

![Axis GUI](plugin-gui.png)

## 機能

- `Input` で M/S 処理前の入力レベルを調整
- ステレオ入力 / ステレオ出力
- `Mid Gain` で Mid 成分を前に出す
- `Side Gain` で Side 成分のレベルを調整
- `Side Density` で Side 成分へ RMS 追従のパラレルコンプレッションを適用
- `Side Spark` で Side 成分のアタックに短い帯域制限付き Spark を加算
- `Spark Send` で Side から Spark 検出へ入る前のレベルを調整
- `Spark Gain` で最終的に加算する Spark 成分のレベルを大きく持ち上げ可能
- `Spark Width` で最終段の Spark の広がりを調整
- `Spark Pitch` で最終段の Spark のピッチを `-24` から `+24 st` で調整
- `Output` で最終レベルを調整
- `Auto Gain` で処理前後のレベル差を自動補正
- `Soft Clip` で最終段のソフトクリップを on/off

## パラメータ

- Input: `-24` から `+12 dB`
- Mid Gain: `-24` から `+12 dB`
- Side Gain: `-24` から `+12 dB`
- Side Density: `0` から `100 %`
- Side Spark: `0` から `150 %`
- Spark Send: `-24` から `+24 dB`
- Spark Gain: `0` から `+36 dB`
- Spark Duck: `0` から `100 %`
- Spark Threshold: `0` から `100 %`
- Spark Width: `0` から `200 %`
- Spark Pitch: `-24` から `+24 st`
- Output: `-24` から `+12 dB`
- Auto Gain: on/off
- Soft Clip: on/off
- Bypass: on/off

`Spark Send` の既定値は `-9 dB` です。`Trigger` が常時高くなりすぎないよう、安全側から始めています。

## ライセンス

現時点で、このリポジトリのソースコードはオープンソースライセンスでは提供していません。

- ソースコードの著作権は作者に帰属します
- 明示的な許可なく、再配布、派生物の配布、商用利用を行わないでください
- 将来の製品化方針に応じて、ライセンス条件を別途定める可能性があります

JUCE の利用条件は JUCE 8 のライセンスに従います。将来クローズドソース製品として配布する場合は、配布時点の JUCE 商用ライセンス条件を確認してください。

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
