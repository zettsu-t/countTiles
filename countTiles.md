# 麻雀の待ち形を速く列挙する

マルチスレッドとビットボードで、C++版より速くなります。

## ビルドする

CygwinターミナルまたはMinGW (cmd)から、makeを以下の引数で実行すると、ビルド、実行、実行時間の計測を行います。countTilesBitsの起動オプション-Nは、CPUの論理スレッドと同じ数のスレッドで実行するという指示です(つけなければシングルスレッド)。

```bash
make checkfastest
```

## マルチスレッドを使う

C++11のstd::futureを使うと、異なる問題を並行して解けます。ただしCygwinでマルチスレッドを使うと、シングルスレッドより遅くなるので、使わない方がよいです。

MinGW + GCCではstd::futureが使えないので、代わりにboost::unique_futureを使います。

注意点として、boost::unique_future を使う(boost/thread/future.hppをインクルードする) .cppファイルで、Intel Syntaxのインラインアセンブリを使うと、アセンブラがエラーを出します。boost::unique_future を使う処理と、インラインアセンブリを記述する処理で、.cppファイルを分ける必要があります。

## ビットボードで手牌を表現する

あがり形14牌を、64ビットレジスタに収まるビット列として表現すると速く解けます。

### 表現形式

ある牌(例えば一索)が何枚あるかを、その枚数分だけビット1を並べて表現します。つまり1進数です。

|同種の牌の数|0|1|2|3|4|
|:-----|:-----|:-----|:-----|:-----|:-----|
|ビット列|00000|00001|00011|00111|01111|

麻雀には同種の牌は4枚しかないので、4ビットで表現できます。これを5ビットで表現し、先頭(左側、MSB)を常に0にすることで、9種類の牌を並べて表現できます。

|*索の番号|九|八|七|六|五|四|三|二|一|
|:-----|:-----|:-----|:-----|:-----|:-----|:-----|:-----|:-----|:-----|
|ビット位置|44..40|39..35|34..30|29..25|24..20|19..15|14..10|9..5|4..0|

n索が何枚あるか調べるのは、ビット[(n - 1) * 5, n * 5 - 1]にある1を数えれば分かります。九索より左(ビット63..45)は常に0にしておきます。

### 牌の組

n索があるかどうか調べるために、ビット1を数えるのは煩雑なので、ビット列中の1は常に右側(LSB側)に寄せるように正規化します。そうすることで、対子、刻子、順子をパターンマッチングで検索できます。

|牌の組|対子|刻子|順子|
|:-----|:-----|:-----|:-----|
|ビット列|11|111|10000100001|

* 対子は同種の牌が2枚以上、刻子は3枚以上にマッチします。異なる種類の牌とは0で隔てられているので、異なる種類の牌とは一致しません。
* 順子は123, .. , 789とは一致しますが、範囲外つまり0-1-2や8-9-10とは一致しません。これは0索がなく、10索以上は常にビットが0だからです。

### 牌を増減する

n索に対応する5bitについて、ビット演算を行うことで牌を増減できます。同様の操作で、対子、刻子、順子をまとめて取り除くこともできます。

* 牌を増やす : 左に1回シフトして、LSBを1にする。
* 牌を減らす : 右に1回シフトする。元のLSBは捨てる。

最近のx86プロセッサは、PEXT/PDEP命令があるので、実行時にビットマスクを与えてビット群を取り出したりはめ込んだりすることができます。ANDN命令を使うとビットマスクを速く掛けられる場合があります。

## 手牌を列挙する

清一色の全組み合わせを列挙します。同種の牌は4枚以下、という制限がつきます。

### 類題

十進数で、8の次は9、19の次は20、199の次は200ですが、これを操作で表現すると以下の通りです。

* 最も下の桁が9でなければ、最も下の桁に1を足す
* 最も下の桁から9が連続するときは、そのまま調べて行くと最初に9でない桁があるので(先頭の0を含む)、その数に1を足し、その右の桁から最も下の桁まですべて0にする

### 例題

例えば1111222233334という手牌があったとして、順番に列挙したとき、その次の手牌は何か、を考えます。先に答えを書くと、1111222233335です。1111222233339の次は1111222233344、1111222233999の次は1111222234444です。

* 最も右の牌を調べて9でなければ、1足す
* 最も右の牌を調べて9のときは、そのまま調べて行くと最初に9でない牌がある。その牌の数を1足し、その牌を最も右の牌までコピーする

### 解法

9索は4枚しかないので、右から左に調べて行くにつれて、調べる対象を8,7,6索と変えていく必要があります。こうすることで、1111222889999の次を1111223333444にできます。最も右の牌を0枚目として、左に向かって以下のように牌を調べると、次の手牌が得られます。

* i枚目の牌が9 - (i / 4)索だったらその左の牌を調べる。つまりiを1増やす。
* 最も左の牌が上記を満たす、つまり6777788889999なら、すべて列挙し終わった(この次はない)。
* i枚目の牌が上記を満たさずn索であれば、i枚目を含めて右4枚をn+1索にする。最も右の牌に届いていなければ、さらに右4枚をn+2索にする、必要ならさらにその右4枚をn+3索にする。これを最も右の牌を置き換えるまで行う。

Nスレッドで実行するときは、各スレッドが0..(N-1)番目の手牌を先頭として、N番間隔で手牌を選び出します。i番目からi+N番目の手牌を直接導ければよいのですが、そのような方法を思いつかないので、i+1 .. i+N-1番目は選んでから捨てます。

### 文字列表示

13牌をそれぞれ1..9で表現し、区切り文字(:)、改行文字(LF)、C文字列終端文字(NUL = 0)を加えると16 bytesです。そのため文字列の生成を、XMMレジスタ上で行うことができます。XMMレジスタで生成した文字列を、16 bytesアラインメントした文字列バッファに転送して、std::stringに与えます。

13牌は1牌=4bitの並びで表現しているので、文字列にするには8bitごとに分割します。0xf0..fと0xf0..0でマスクして、奇数桁の集まりと偶数桁の集まりに分けます。

|分割前(64bitレジスタ)|-|-|-|12|11|10|9|8|7|6|5|4|3|2|1|0|
|:-----|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|
|分割後(64bitレジスタ*2)|-|12|10|8|6|4|2|0|-|-|11|9|7|5|3|1|

次にそれぞれの牌を、数字を表す文字'1'..'9'に変換します。それぞれの牌に'0' (0x30) を足せばよいのですが、VPADDB命令を使うと16文字分を一度に足すことができます。XMMレジスタの各byteを0x30で埋めるには、XMMレジスタの最下位byteに0x30を転送してから、VPSHUFB命令にパターンとして値0を与えてすべてのbyteに0x30をコピーします。

牌以外の文字も、あらかじめこれらのASCIIコードから0x30をあらかじめ引いておくことで、上記のVPADDB命令を使ってXMMレジスタに組み込むことができます。例えば0x30を足して0x0aにするには、足される数を0xdaにします。VPADDB命令の加算はwraparoundしますが、ある文字の桁あふれは他の文字に伝搬しないのでその点は気にしなくてよいです。

最後に、0x30を足して得られるXMMレジスタの値を、エンディアンを考慮してVPSHUFB命令で以下の通り並び替え、VMOVDQA命令で文字列バッファに格納します。牌の数字は上記の表の通りで、牌以外の文字は空いているところ(*)に入れておきます。

下表の各数字は文字列の何文字目か(先頭は0)、右から何桁目か(先頭は0)、およびLSB側から順に0..15 (8 bytes単位)です。転送後の文字について、真下(転送前の桁)に書いてある数字が示す文字の位置を見つけて、その文字の最下行に転送後の何文字目かを書き込みます。例えば転送後の1文字目は転送前の9桁目ですので、9文字目の列の一番下に1を書き込みます。このようにして得られる転送後の位置が、VPSHUFB命令に与えるパターンです。

|転送後の文字|15(NUL)|14('\n')|13(':')|12|11|10|9|8|7|6|5|4|3|2|1|0|
|:-----|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|
|転送前の桁|15|0|2|4|6|8|10|12|14|13|1|3|5|7|9|11|
|転送後の位置|15|7|6|8|0|9|1|10|2|11|3|12|4|13|5|14|

## 手牌から待ち形を探す

1. あがり牌1..9索を決め打ちし、手牌に加えて14牌にする。5*n bit目がすべて0であると調べることで、同種の牌が5枚ないことを確認する。
1. すべての対子について、バックトラッキングを用いて残りの12牌を(刻子|順子)*4に分解する
1. バックトラッキングでは、まず刻子を探す。刻子がなければ、以後は順子だけ探す。刻子があれば、刻子にするときとしないときの両方を引き続き探索する。
1. 対子+(刻子|順子)*4から、決め打ちしたあがり牌1..9索を抜く。抜き方は5通り以下である(同種の牌が4枚なので本当は4通り以下)。
1. あがり牌を抜いた後の対子+(刻子|順子)*4について並べ替えを考慮して一意にしたものが、手牌に対する待ち形のすべてである

## 手牌を文字列に変える

対子、刻子、順子を文字列にします。

* LSBからビットスキャン命令を用いて、最初に見つかるビット1を調べる。これをiビット目とする(LSBが0番目)。
* n = i / 5ならn索が少なくとも一つある、と言える。5で割ると遅いので、4で割ってn索を求める方が速い。

|*索の番号|九|--|八|七|六|五|--|四|三|二|一|
|:-----|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|
|ビット位置/4|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|

iビット目を最右とするパターンマッチングで、対子、刻子、順子があるかどうか検索する。あれば文字列に変換して手牌から取り除く、なければ何もしない。ビット列10000100111を用いてPEXT命令を実行し、5ビット32通りのパターンを取り出すと、待ち形がわかる。32通り中、下記以外のビットパターンは無視する。

|ビットパターン|待ち形|完成形|
|:-----|:----|:----|
|00001|単騎|---|
|00011|シャボ|対子|
|00111|---|刻子|
|01001|両面/辺張|---|
|10001|嵌張|---|
|11001|---|順子|

## 手牌を並び替えて得られる待ちを一意にする

対子 + (刻子|順子) * 4 それぞれに10ビットのキーをつけることで(下記)、1-3牌の待ち形と完成形を一意に特定できます。この10bit * 5組について、先頭は待ち形、それ以外の完成形はソートし、これらを連結して50ビットのキーを作ることで、13牌の待ち形を一意に特定できます。なお最長64bit * 4組しかないので、ソートはstd::sortではなくインラインアセンブリで挿入ソートを記述しています。

|1ビット|5ビット|4ビット|
|:-----|:----|:----|
|待ちなら1, 完成形なら0|待ち/完成形のビットパターン|最小の*索のビット位置/4|

この10ビットのキーから、待ち形および完成形の文字列を生成することができます。3牌+括弧で高々5文字なので、64ビットレジスタ上で文字列を作れます。
* 先頭をn索として、残り2牌を(それぞれもしあれば)n索、n+1索またはn+2索にする。これらをそれぞれ印字可能文字にする。
* 待ち形および完成形が3牌未満なら、余分な1または2牌分の文字を取り除く
* 待ち形は[]、完成形は()で囲む
