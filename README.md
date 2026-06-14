# Phantom

JUCE/C++で実装した、M/Sベースの音像コントロール用ステレオVST3プラグインです。

## スクリーンショット

![Phantom GUI](plugin-gui.png)

## 機能

- ステレオ入力 / ステレオ出力
- `Center` で Mid 成分を前に出す
- `Density` で Side 成分へ RMS 追従のパラレルコンプレッションを適用
- `Width` でステレオ幅を調整
- `Output` で最終レベルを調整
- `Auto Trim` と `Soft Clip` を内部で常時適用

## パラメータ

- Center: `-24` から `+12 dB`
- Density: `0` から `100 %`
- Width: `0` から `200 %`
- Output: `-24` から `+12 dB`

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

macOSでは、`AxisCenter_VST3` のビルド完了後に `~/Library/Audio/Plug-Ins/VST3/Phantom.vst3` へ自動で上書き配置されます。
