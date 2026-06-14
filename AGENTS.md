# AGENTS.md

このファイルは、このリポジトリで作業する Codex 系エージェント向けの運用規約です。

## 基本方針

- 変更は最小差分で行う
- 既存の意図が明確な実装は尊重する
- ビルドや検証が可能なら、変更後に確認する
- ユーザーの明示指示がない限り、不要なリネームや大規模整理は行わない
- ユーザー向けの説明、提案、要約、ドキュメントは日本語を必須とする
- コミットメッセージの説明部、PR タイトル、PR 本文も日本語を必須とする

## コミット規約

このリポジトリでは、コミットメッセージに [Conventional Commits 1.0.0](https://www.conventionalcommits.org/ja/v1.0.0/) を必須とする。

コミットメッセージは次の形式に従うこと。

```text
<type>[optional scope]: <description>
```

例:

```text
feat: Input と Auto Gain を追加
fix(gui): Axis ロゴを復帰
ci: release まとめ PR を自動作成
docs: Codex 用ガイドラインを追加
```

### 必須ルール

- すべてのコミットは `type:` で始める
- `type` には少なくとも `feat` `fix` `docs` `ci` `chore` `refactor` `test` を使用可能とする
- `type:` の後ろの説明文は日本語を必須とする
- 破壊的変更は `!` または `BREAKING CHANGE:` を使って明示する
- あいまいなコミットメッセージを禁止する

禁止例:

```text
update
fix stuff
changes
wip
```

## Codex フッター規約

Codex が実質的に変更作業へ関与したコミットでは、コミットフッターに次を追加すること。

```text
Co-authored-by: codex <codex@openai.com>
```

複数フッターを使う場合も、git trailer 形式を保つこと。

例:

```text
feat: add input and auto gain controls

Co-authored-by: codex <codex@openai.com>
```

## PR と変更の扱い

- 1つの PR では 1つの目的に集中する
- release 運用、CI、DSP、GUI の変更が混ざる場合は、可能なら分割を検討する
- 既存の open PR と重複する変更は避ける
- PR タイトルは日本語を必須とする
- PR 本文は日本語を必須とする

## 検証

- UI や DSP を変更した場合は、可能な範囲でビルド確認を行う
- ビルドできた場合は、その事実をユーザーへ明示する
- ビルドしていない場合や確認できない場合も、その事実を明示する
