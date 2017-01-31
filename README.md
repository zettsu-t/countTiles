# ITmediaに掲載されているプログラミング課題

出題元は以下の通りです。

http://www.itmedia.co.jp/enterprise/articles/1004/03/news002_2.html

## ビルド環境

当方で実行した環境は以下の通りです。

* Windows 10 Anniversary Update 64bit Edition
* Cygwin 64bit version (2.6.0)
* LLVM + clang (3.8.1 : Cygwin)
* GCC (6.3.0 : MinGW / 5.4.0 Cygwin)
* Ruby (2.2.5p319 : Cygwin)
* GHC 8.0.1

Cygwinターミナルから、makeを引数なしで実行すると、ビルド、実行、実行結果の検査を行います。

```bash
make
```

## ファイル一覧

バックトラッキングで解きます。異なる言語で実装すると、コードの見た目も実行速度も大きく変わることが分かります。

Haskell版のコードは、できるだけ短く書いたり、コードに縦読みを仕込んでみたり、実行速度を無視したお遊びです。私の環境ですと、一問解くのに40秒も掛かります。

|ファイル名|説明|
|:------|:------|
|countTiles.rb|Rubyでバックトラッキングして解く|
|countTiles.cpp|C++14でバックトラッキングして解く|
|countTilesBits*|C++11 + インラインアセンブリで解く : [説明](countTiles.md) |
|countTiles.hs|Haskellでバックトラッキングして解く|
|countTilesSlow.hs|Haskellで総当たりで解く(とても遅い)|
|countTilesShort.hs|ソースコードをできる限り短くしたもの(515 Bytes)|
|countTilesEx.hs|ソースコードに縦読みを仕込んだもの|
|countTilesCompareLog.rb|countTiles.* の実行結果を調べる|
|countTilesCheckHs.rb|*.hsの実行結果を調べる|
|Makefile|ビルドする|

countTilesBitsをMinGW でビルドする場合は、Boost C++ Librariesが必要です。Distro 14.1でビルドできます。詳しくは[こちら](countTiles.md) をご参照ください。

## ライセンス

本ソフトウェアはMITライセンスのもとで配布しています。詳しくは[LICENSE.txt](LICENSE.txt)をご覧ください。
