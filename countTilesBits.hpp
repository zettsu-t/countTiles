/*
 * 出題元
 * http://www.itmedia.co.jp/enterprise/articles/1004/03/news002_2.html
 */

namespace TileSetSolver {
    using TileMap = uint64_t;   // 複数の牌を格納するビット列
    using TileIndex = TileMap;  // 牌の番号 1..9
    using SizeType = size_t;    // コレクションなどのサイズ一般
    using StrArray = std::vector<std::string>;  // 文字列の配列

    constexpr SizeType SizeOfTileSet = 5;  // 対子 + 刻子または順子 * 4 で5組
    constexpr SizeType SizeOfCompleteTiles = 14;  // あがり形の牌の数
    constexpr SizeType SizeOfOneTile = 4;         // 1種は4牌
    constexpr SizeType SizeOfBitsPerTile = 5;     // 1種は4牌だが、パディングを1bit用意する
    constexpr SizeType TileMin = 1;        // 牌の数字の最小値
    constexpr SizeType TileMax = 9;        // 牌の数字の最大値

    // 一スレッドで、待ち形を列挙して、解いた結果をresultに格納する
    // indexOffset番目(先頭は0)から、stepSize個間隔で、待ち形を列挙する
    extern void EnumerateAll(SizeType indexOffset, SizeType stepSize, StrArray& result);
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
