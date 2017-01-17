/*
 * 出題元
 * http://www.itmedia.co.jp/enterprise/articles/1004/03/news002_2.html
 */

#include <vector>

namespace TileSetSolver {
    using TileMap = uint64_t;   // 複数の牌を格納するビット列
    using TileIndex = TileMap;  // 牌の番号 1..9
    using SizeType = size_t;    // コレクションなどのサイズ一般
    using TileKey  = uint64_t;  // 待ちを一意に定めるキー
    using StrArray = std::vector<std::string>;    // 文字列の配列

    constexpr SizeType SizeOfTileSet = 5;  // 対子 + 刻子または順子 * 4 で5組
    constexpr SizeType SizeOfCompleteTiles = 14;  // あがり形の牌の数
    constexpr SizeType SizeOfOneTile = 4;         // 1種は4牌
    constexpr SizeType SizeOfBitsPerTile = 5;     // 1種は4牌だが、パディングを1bit用意する
    constexpr SizeType TileMin = 1;        // 牌の数字の最小値
    constexpr SizeType TileMax = 9;        // 牌の数字の最大値
    constexpr SizeType SizeOfKeyBits = 10; // 待ちを一意に定めるキーの、組みあたりビット数
    constexpr TileKey  OpenKey = 1 << (SizeOfKeyBits - 1);  // 待ち形のキー

    // 一スレッドで、待ち形を列挙して、解いた結果をresultに格納する
    // indexOffset番目(先頭は0)から、stepSize個間隔で、待ち形を列挙する
    extern void EnumerateAll(SizeType indexOffset, SizeType stepSize, StrArray& result);

    // 結果の文字列
    // 関数の返り値になるように、構造体でboxingする
    struct ResultString {
        char value[32];
    };

    // すべての牌 + 区切り記号 + 終端文字の長さ
    constexpr SizeType ResultStringLength = SizeOfCompleteTiles + SizeOfTileSet * 2;
    // 1..3牌 + 区切り記号を8byte単位でコピーするので、余分に5文字書き込むことがある
    static_assert((sizeof(char) * (ResultStringLength + 5)) <= sizeof(ResultString), "Too small");

    using KeyArray = std::vector<TileKey>;                // キーの配列
    using ResultStringArray = std::vector<ResultString>;  // 文字列の配列
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
