## 概要

この PR は `master` にある変更を `release` へ集約するためのリリース PR です。

## リリースバージョン

- リリース予定: `v0.X.X`

## 含める内容

- `master` に取り込まれた変更の集約
- リリース対象として含める修正の確認

## 確認事項

- [ ] base branch が `release` になっている
- [ ] head branch が `master` になっている
- [ ] PR タイトルが `v0.X.Xのreleaseまとめ` 形式になっている
- [ ] リリースに含めたくない変更が混ざっていない
- [ ] `release` merge 後に version bump、tag、build、GitHub Release が走る前提で問題ない

## メモ

- `VERSION` は `release` merge 後の workflow で patch bump されます
- 個別機能の説明ではなく、今回のリリースに含める変更群のまとまりとして扱います
