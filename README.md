# RagiShark
Web版WireShark  
WebSocket接続しているクライアントに対し、
TSharkによるキャプチャデータを送信しまくる実験

## Client
- TypeScript, React

## Server
- TShark
- C#, dotnetcore
  - 簡易(手抜き)WebSocketサーバ実装
    - 独自コマンド受信
    - データ送信
- C++17 (今はWindows専用)
  - 簡易(手抜き)WebSocketサーバ実装
    - データ送信
- Rust
  - 簡易(手抜き)WebSocketサーバ実装
    - データ送信
