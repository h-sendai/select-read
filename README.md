# epoll-read

複数台のSiTCP機器の読み取りテスト用プログラムです。

- 1秒置きに読んだバイト数、read()した回数を表示します。
- Ctrl-Cで終了します。

## 使い方

    ./epoll-read 192.168.10.16:24 [ more IP address and port ]
