/*
 * 出題元
 * http://www.itmedia.co.jp/enterprise/articles/1004/03/news002_2.html
 *
 * ビルド方法
 *   make (引数無し)
 * でコンパイル、リンク、実行、実行結果計測を行う
 * 清一色の全組み合わせについて、すべての待ちを4秒台で結果を出力する
 */

#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

// virtualをなくすと速くなるときはそうする
#ifdef DISABLE_VIRTUAL
#define VIRTUAL_FUNC
#define NO_INHERIT final
#else // DISABLE_VIRTUAL
#define VIRTUAL_FUNC virtual
#define NO_INHERIT
#endif // DISABLE_VIRTUAL

namespace CountTiles {
    // 牌の種類を示す型
    using TileType = int;
    // 牌の数を示す型
    using TileSize = size_t;
    // 牌の集合を並べ替えるためのキーとその生成関数
    using TileSetKey = uint64_t;
    // 牌の種類 -> 個数数の対応表
    using TileTable = std::unordered_map<TileType, TileSize>;
    // 13牌 -> 文字列の対応表
    using TileStrTable = std::unordered_map<TileSetKey, std::string>;

    // 牌の数字の最小最大値
    constexpr TileType TileMin = 1;
    constexpr TileType TileMax = 9;
    constexpr TileType TileSpecial = 10;  // keyを重くするための特別な値
    // 牌の種類の数
    constexpr TileSize KindOfTiles = TileMax + 1 - TileMin;
    // 同種の牌の数
    static constexpr TileSize SizeOfOneTile = 4;
    // あがるときの牌の数
    constexpr TileSize SizeOfCompleteTiles = 14;

    // 牌を各桁とみなしてkeyを作る時の基数
    constexpr TileSetKey RadixOfTiles = TileSpecial + 1;
    constexpr TileSetKey MaxRadixOfThreeTiles = RadixOfTiles * RadixOfTiles * RadixOfTiles;
    // 1..3牌の組に対するkeyに、待ち形は完成形とは異なるkeyを割り当てるので、下駄を履かせる
    constexpr TileSetKey RadixOfThreeTilesOpen = MaxRadixOfThreeTiles + 1;
    // 1..3牌の組に対するkeyを連結するときの基数(シフト演算で掛けられるよう2^nにする)
    constexpr TileSetKey RadixOfThreeTiles = 1 << 12;
    static_assert(RadixOfThreeTiles > (RadixOfThreeTilesOpen * 2), "Too small RadixOfThreeTiles");

    // 最大のkey(収まりきらないならwarningが出るはず)
    constexpr TileSetKey MaxKeyNumber __attribute__((unused)) =
                                                  RadixOfThreeTiles * RadixOfThreeTiles * RadixOfThreeTiles *
                                                  RadixOfThreeTiles * RadixOfThreeTiles;

    // 文字表示
    constexpr char LeftBracketClosed  = '(';  // 待ちを含まない左かっこ
    constexpr char RightBracketClosed = ')';  // 待ちを含まない右かっこ
    constexpr char LeftBracketOpen    = '[';  // 待ちの左かっこ
    constexpr char RightBracketOpen   = ']';  // 待ちの右かっこ

    // '0'..'9'が連番であると仮定している
    static_assert((static_cast<int>('0') + 1) == static_cast<int>('1'), "Must be consecutive");
    static_assert((static_cast<int>('1') + 1) == static_cast<int>('2'), "Must be consecutive");
    static_assert((static_cast<int>('2') + 1) == static_cast<int>('3'), "Must be consecutive");
    static_assert((static_cast<int>('3') + 1) == static_cast<int>('4'), "Must be consecutive");
    static_assert((static_cast<int>('4') + 1) == static_cast<int>('5'), "Must be consecutive");
    static_assert((static_cast<int>('5') + 1) == static_cast<int>('6'), "Must be consecutive");
    static_assert((static_cast<int>('6') + 1) == static_cast<int>('7'), "Must be consecutive");
    static_assert((static_cast<int>('7') + 1) == static_cast<int>('8'), "Must be consecutive");
    static_assert((static_cast<int>('8') + 1) == static_cast<int>('9'), "Must be consecutive");

    TileType ConvertToTileType(char c) {
        return ((c >= '1') && (c <= '9')) ? (static_cast<TileType>(c) - static_cast<TileType>('0')) : 0;
    }

    char ConvertToChar(TileType n) {
        return ((n >= 1) && (n <= 9)) ? static_cast<char>(n + '0') : '.';
    }

    size_t GetArrayIndex(TileType n) {
        // 1..9に対して0..8を返す
        return (n - 1);
    };

    // 牌を各桁とみなしたときのkeyを返す
    TileSize GetKeyValue(TileType n) {
        // n = 0 だとkeyで牌の順序を区別できないが、1以上なので区別できる
        return n;
    };

    // 刻子または順子のバックトラッキング状態
    enum class Status {
        NOT_SEARCHED,       // まだ探していない
        TRIPLE_SEARCHED,    // 刻子を探した
        SEQUENCE_SEARCHED,  // 順子を探した
    };

    // 牌を各桁とみなしたときのkeyを返す
    // 待ちの場合はopen = true, 完成形の場合はopen = false
    template <typename... RemainingTiles>
    auto CalculateKeyImpl(TileType tile, RemainingTiles... tiles) {
        return GetKeyValue(tile) + CalculateKeyImpl(tiles...) * RadixOfTiles;
    }

    // 最上位の桁
    template <>
    auto CalculateKeyImpl(TileType tile) {
        return GetKeyValue(tile);
    }

    // 待ち形のkeyを完成形より重くする
    template <typename... TilesType>
    auto CalculateKey(bool open, TilesType... tiles)
        -> typename std::enable_if_t<(sizeof...(tiles) == 3), TileSetKey> {
        return ((open) ? RadixOfThreeTilesOpen : 0) + CalculateKeyImpl(tiles...);
    }

    template <typename... TilesType>
    auto CalculateKey(bool open, TilesType... tiles)
        -> typename std::enable_if_t<(sizeof...(tiles) < 3), TileSetKey> {
        // 対子が後に表示されるようにkeyを大きくする
        return CalculateKey(open, tiles..., TileSpecial);
    }

    // 次に探す最も数字の小さい牌を探す
    auto FindMinTile(const TileTable& table, TileType head) {
        for(auto tile = head; tile <= TileMax; ++tile) {
            if (table.at(tile)) {
                return tile;
            }
        }
        return head;
    }

    // 対子、刻子または順子
    class TileSet {
    public:
        TileSet(void) = default;
        virtual ~TileSet(void) = default;
        virtual TileSetKey GetKey(bool open, TileType extraTile) const = 0;
        virtual bool HasTile(TileType tile) const = 0;
        virtual const std::string& ToString(bool open, TileType extraTile) const = 0;
    };

    // 既に現れた対子、刻子または順子
    class KeyTileSetTable NO_INHERIT {
    public:
        KeyTileSetTable(void) {
            for(auto tile = TileMin; tile <= TileMax; ++tile) {
                zeroTable_[tile] = 0;
            }

            clearTable(currentTable_);
            return;
        }

        VIRTUAL_FUNC ~KeyTileSetTable(void) = default;
        KeyTileSetTable(const KeyTileSetTable&) = delete;
        KeyTileSetTable& operator=(const KeyTileSetTable&) = delete;

        VIRTUAL_FUNC void Reset(void) {
            clearTable(currentTable_);
            return;
        }

        // 初期牌を足す
        VIRTUAL_FUNC void AddTiles(TileType tile, TileSize n) {
            currentTable_[tile] += n;
            return;
        }

        // 牌を戻す
        VIRTUAL_FUNC void PushTiles(TileType tile, TileSize n) {
            currentTable_[tile] += n;
            return;
        }

        // 牌を抜き取る
        VIRTUAL_FUNC void PopTiles(TileType tile, TileSize n) {
            currentTable_[tile] -= n;
            return;
        }

        // ある種の牌が何枚あるか返す
        VIRTUAL_FUNC TileSize GetRemainingSize(TileType tile) const {
            return currentTable_.at(tile);
        }

        // 残り牌のうち最小の番号を返す
        VIRTUAL_FUNC TileType GetMinTile(TileType head) const {
            return FindMinTile(currentTable_, head);
        }

    private:
        void clearTable(TileTable& table) {
            table = zeroTable_;
        }

        void printTable(TileTable& table) {
            for(auto tile = TileMin; tile <= TileMax; ++tile) {
                std::cout << table[tile];
            }
            std::cout << "...\n";
        }

        TileTable  zeroTable_;     // 牌の数が0のテーブル
        TileTable  currentTable_;  // 残りの牌
    };

    // 対子
    class TilePair : public TileSet {
    public:
        virtual ~TilePair(void) = default;
        TilePair(const TilePair&) = delete;
        TilePair& operator=(const TilePair&) = delete;

        // 未登録ならインスタンスを作る
        static TilePair* GetInstance(TileType tile) {
            const auto keyClosed = getKeyClosed(tile);
            if (instanceSet_.find(keyClosed) == instanceSet_.end()) {
                instanceSet_[keyClosed] = std::unique_ptr<TilePair>(new TilePair(keyClosed, tile));
            }

            return instanceSet_.at(keyClosed).get();
        }

        virtual TileSetKey GetKey(bool open, TileType extraTile) const override {
            return (open) ? keyOpen_ : keyClosed_;
        }

        virtual bool HasTile(TileType tile) const override {
            return (tile_ == tile);
        }

        // 完成形または待ちを返す
        virtual const std::string& ToString(bool open, TileType extraTile) const override {
            return (open) ? strAsOpen_ : strAsClosed_;
        }

        // どの牌か取得する
        VIRTUAL_FUNC TileType Get(void) const {
            return tile_;
        }

    private:
        TilePair(TileSetKey keyClosed, TileType tile) :
            keyClosed_(keyClosed),
            keyOpen_(CalculateKey(true, tile)),
            tile_(tile),
            strAsClosed_({LeftBracketClosed, ConvertToChar(tile), ConvertToChar(tile), RightBracketClosed}),
            strAsOpen_({LeftBracketOpen, ConvertToChar(tile), RightBracketOpen}) {
            return;
        }

        static TileSetKey getKeyClosed(TileType tile) {
            return CalculateKey(false, tile, tile);
        }

        using Table = std::unordered_map<TileSetKey, std::unique_ptr<TilePair>>;
        const TileSetKey  keyClosed_;    // 完成形のkey
        const TileSetKey  keyOpen_;      // 待ち形のkey
        const TileType    tile_;         // 2つとも同じ牌なのだから一つだけ保存する
        const std::string strAsClosed_;  // 待ち形の文字列
        const std::string strAsOpen_;    // 完成形の文字列
        static Table instanceSet_;       // すでに生成したインスタンス
    };

    TilePair::Table TilePair::instanceSet_;

    // 刻子または順子
    class ThreeTiles : public TileSet {
    public:
        // コンストラクタが引数をコピーできるよう3牌の内部データ構造形式を公開する
        using Data = std::array<TileType, 3>;

        virtual ~ThreeTiles(void) = default;
        ThreeTiles(const ThreeTiles&) = delete;
        ThreeTiles& operator=(const ThreeTiles&) = delete;

        // 未登録ならインスタンスを作る
        static ThreeTiles* GetInstance(const Data& tiles) {
            const auto keyClosed = CalculateKey(false, tiles[0], tiles[1], tiles[2]);
            if (instanceSet_.find(keyClosed) == instanceSet_.end()) {
                instanceSet_[keyClosed] = std::unique_ptr<ThreeTiles>(new ThreeTiles(keyClosed, tiles));
            }

            return instanceSet_.at(keyClosed).get();
        }

        virtual TileSetKey GetKey(bool open, TileType extraTile) const override {
            return (open) ? tileAttrSet_[GetArrayIndex(extraTile)].keyOpen_ : keyClosed_;
        }

        virtual bool HasTile(TileType tile) const override {
            return (tileAttrSet_[tile].count_ != 0);
        }

        // 完成形または待ちを返す
        virtual const std::string& ToString(bool open, TileType extraTile) const override {
            return (open) ? tileAttrSet_[GetArrayIndex(extraTile)].strOpen_ : strClosed_;
        };

        // 3つの牌を戻す
        VIRTUAL_FUNC void ReturnToTable(KeyTileSetTable& table) {
            for(auto tile : tiles_) {
                table.PushTiles(tile, 1);
            }
            return;
        }

    private:
        // tilesは昇順にソート済であること
        ThreeTiles(TileSetKey keyClosed, const Data& tiles) :
            keyClosed_(keyClosed),
            tiles_(tiles),
            strClosed_({LeftBracketClosed, ConvertToChar(tiles[0]), ConvertToChar(tiles[1]),
                        ConvertToChar(tiles[2]), RightBracketClosed}) {

            // indexが待ち牌の場合の文字列を返す
            registerThreeTiles(tiles[0], tiles[1], tiles[2]);

            // 刻子に対しては上記と待ちが同じなので作らない
            if (tiles[0] != tiles[1]) {
                registerThreeTiles(tiles[1], tiles[0], tiles[2]);
                registerThreeTiles(tiles[2], tiles[0], tiles[1]);
                tileAttrSet_[tiles[0]].count_ += 1;
                tileAttrSet_[tiles[1]].count_ += 1;
                tileAttrSet_[tiles[2]].count_ += 1;
            } else {
                tileAttrSet_[tiles[0]].count_ += 3;
            }

            return;
        }

        void registerThreeTiles(TileType tileExtra, TileType tile1, TileType tile2) {
            const auto index = GetArrayIndex(tileExtra);
            tileAttrSet_[index].keyOpen_ +=  CalculateKey(true, tile1, tile2);
            tileAttrSet_[index].strOpen_ = std::string({LeftBracketOpen, ConvertToChar(tile1),
                        ConvertToChar(tile2), RightBracketOpen});
            return;
        }

        struct TileAttribute {
            TileSize    count_   {0};  // 何個あるか
            TileSetKey  keyOpen_ {0};  // key
            std::string strOpen_;      // 待ち形の文字列
        };

        using Table = std::unordered_map<TileSetKey, std::unique_ptr<ThreeTiles>>;
        const TileSetKey  keyClosed_;  // 完成形のkey
        const Data        tiles_;      // 3つの牌
        const std::string strClosed_;  // 完成形の文字列
        std::array<TileAttribute, KindOfTiles> tileAttrSet_;  // 各牌の持ち方
        static Table instanceSet_;
    };

    ThreeTiles::Table ThreeTiles::instanceSet_;

    // 対子 + 3 * 4
    class TilesWithPair NO_INHERIT {
    public:
        // 対子と13牌を指定する
        TilesWithPair(TileType extraTile, TileType pair, KeyTileSetTable& table,
                      TileStrTable& localStrTable, TileStrTable& allStrTable) :
            extraTile_(extraTile), pPair_(TilePair::GetInstance(pair)), table_(table),
            localStrTable_(localStrTable), allStrTable_(allStrTable), numberOfThreeTiles_(0) {
            tileSetArray_[0] = TileSetElement {pPair_, nullptr, Status::NOT_SEARCHED, table_.GetMinTile(TileMin)};
            table_.PopTiles(pair, 2);
        }

        VIRTUAL_FUNC ~TilesWithPair(void) {
            // 対子を元に戻す
            table_.PushTiles(pPair_->Get(), 2);
        }

        TilesWithPair(const TilesWithPair&) = delete;
        TilesWithPair& operator=(const TilesWithPair&) = delete;

        // 探し終わった
        VIRTUAL_FUNC bool IsComplete(void) const {
            return (numberOfThreeTiles_ >= MaxNumberOfThreeTiles);
        }

        // 刻子を探す
        // 見つかったら格納してtrueを返す、見つからなかったらfalseを返す
        VIRTUAL_FUNC bool SearchTriple(void) {
            const auto oldIndexThreeTiles = numberOfThreeTiles_;
            constexpr TileSize size = 3;

            for(auto tile = tileSetArray_[numberOfThreeTiles_].minTile_; tile <= TileMax; ++tile) {
                if (table_.GetRemainingSize(tile) >= size) {
                    auto pThreeTiles = ThreeTiles::GetInstance(ThreeTiles::Data {{tile, tile, tile}});
                    ++numberOfThreeTiles_;

                    // 先頭は対子
                    tileSetArray_[numberOfThreeTiles_] = TileSetElement
                        {pThreeTiles, pThreeTiles, Status::NOT_SEARCHED, table_.GetMinTile(tile)};
                    table_.PopTiles(tile, size);
                    break;
                }
            }

            return (numberOfThreeTiles_ > oldIndexThreeTiles);
        }

        // 順子を一組探す
        // 見つかったら格納してtrueを返す、見つからなかったらfalseを返す
        VIRTUAL_FUNC bool SearchSequenceThree(void) {
            const auto oldIndexThreeTiles = numberOfThreeTiles_;

            for(auto tile = tileSetArray_[numberOfThreeTiles_].minTile_; tile <= TileMax; ++tile) {
                if ((tile <= TileMax - 2) && table_.GetRemainingSize(tile) &&
                    table_.GetRemainingSize(tile + 1) && table_.GetRemainingSize(tile + 2)) {
                    // 順子
                    auto pThreeTiles = ThreeTiles::GetInstance(ThreeTiles::Data{{tile, tile + 1, tile + 2}});
                    ++numberOfThreeTiles_;

                    // 先頭は対子
                    tileSetArray_[numberOfThreeTiles_] = TileSetElement
                        {pThreeTiles, pThreeTiles, Status::NOT_SEARCHED, table_.GetMinTile(tile)};

                    table_.PopTiles(tile, 1);
                    table_.PopTiles(tile + 1, 1);
                    table_.PopTiles(tile + 2, 1);
                    break;
                }
            }

            return (numberOfThreeTiles_ > oldIndexThreeTiles);
        }

        // 刻子または順子を一組探す。
        // 見つかったら格納してtrueを返す、見つからなかったらfalseを返す
        VIRTUAL_FUNC bool SearchNext(void) {
            // 探し終わった
            if (numberOfThreeTiles_ >= MaxNumberOfThreeTiles) {
                return false;
            }

            auto found = false;
            auto& status = tileSetArray_[numberOfThreeTiles_].status_;
            if (status == Status::NOT_SEARCHED) {
                found = SearchTriple();
                status = Status::TRIPLE_SEARCHED;
            }

            if (!found && (status == Status::TRIPLE_SEARCHED)) {
                found = SearchSequenceThree();
                status = Status::SEQUENCE_SEARCHED;
            }

            return found;
        }

        // 最後に見つかった刻子または順子をなかったことにする
        VIRTUAL_FUNC void Revert(void) {
            const auto oldIndexThreeTiles = numberOfThreeTiles_;
            --numberOfThreeTiles_;
            // 先頭は対子
            tileSetArray_[oldIndexThreeTiles].pThreeTiles_->ReturnToTable(table_);
        }

        // 結果を文字列として取得する
        VIRTUAL_FUNC const std::string ToString(void) {
            struct KeyIndex {
                TileSize   index;     // 何組目か
                TileSet*   pTileSet;  // 対子、刻子または順子
                TileSetKey key;       // key
            };

            std::string strAll;

            for(decltype(numberOfThreeTiles_) i = 0; i <= numberOfThreeTiles_; ++i) {
                // 同種の牌が複数あっても、i番目の対子,刻子,順子以外からは抜かない
                if (!tileSetArray_[i].pTiles_->HasTile(extraTile_)) {
                    continue;
                }

                std::array<KeyIndex, MaxNumberOfThreeTiles+1> keyIndexArray;
                for(decltype(numberOfThreeTiles_) j = 0; j <= numberOfThreeTiles_; ++j) {
                    keyIndexArray[j] = KeyIndex{j, tileSetArray_[j].pTiles_,
                                                tileSetArray_[j].pTiles_->GetKey(i == j, extraTile_)};
                }

                std::sort(keyIndexArray.begin(), keyIndexArray.end(),
                          [&](const KeyIndex& l, const KeyIndex& r) { return (l.key > r.key); } );
                TileSetKey key = 0;
                for(decltype(numberOfThreeTiles_) j = 0; j <= numberOfThreeTiles_; ++j) {
                    key = key * RadixOfThreeTiles + keyIndexArray[j].key;
                }

                if (localStrTable_.find(key) != localStrTable_.end()) {
                    continue;
                }

                std::string str;
                auto strIndex = numberOfThreeTiles_ + 1;
                while(strIndex > 0) {
                    --strIndex;
                    auto& keyIndex = keyIndexArray[strIndex];
                    auto open = (i == keyIndex.index);
                    str += keyIndex.pTileSet->ToString(open, extraTile_);
                }

                localStrTable_[key] = str;
                if (allStrTable_.find(key) == allStrTable_.end()) {
                    allStrTable_[key] = str;
                }

                strAll += str;
                strAll += "\n";
            }

            return strAll;
        }

    private:
        static constexpr TileSize MaxNumberOfThreeTiles = 4;

        struct TileSetElement {
            TileSet*    pTiles_;        // 対子、刻子または順子
            ThreeTiles* pThreeTiles_;   // 見つかった刻子または順子
            Status      status_;        // 刻子と順子のどちらを探すか
            TileType    minTile_;       // 残りの牌のうち、最小の番号
        };

        TileType   extraTile_;    // 決め打ちしたあがり牌
        TilePair*  pPair_;        // 対子
        KeyTileSetTable& table_;  // 残りの牌
        TileStrTable& localStrTable_;    // ある14牌に対するすべての組み合わせを表現する文字列
        TileStrTable& allStrTable_;      // すべての14牌に対するすべての文字列
        TileSize   numberOfThreeTiles_;  // 刻子または順子の数

        // 先頭は対子
        std::array<TileSetElement, MaxNumberOfThreeTiles + 1> tileSetArray_;
    };

    // 手牌
    class TileFullSet NO_INHERIT {
    public:
        TileFullSet(const std::string& line, TileStrTable& allStrTable) :
            allStrTable_(allStrTable), fail_(false) {
            auto found = false;
            size_t count = 0;
            for(auto c : line) {
                if (!std::isdigit(c)) {
                    if (found) {
                        // 数字の後は無視する
                        break;
                    } else {
                        // まだ数字を見つけていない
                        continue;
                    }
                }

                auto tile = ConvertToTileType(c);
                initialTable_[tile] += 1;
                ++count;
            }

            fail_ |= ((count + 1) != SizeOfCompleteTiles);
        }

        TileFullSet(const TileTable& initialTable, TileStrTable& allStrTable) :
            initialTable_(initialTable), allStrTable_(allStrTable), fail_(false) {
        }

        VIRTUAL_FUNC ~TileFullSet(void) = default;
        TileFullSet(const TileFullSet&) = delete;
        TileFullSet& operator=(const TileFullSet&) = delete;

        // 待ちをすべて探して文字列として返す
        VIRTUAL_FUNC const std::string SearchAll(void) {
            std::string str;

            for(const auto& e : initialTable_) {
                table_.AddTiles(e.first, e.second);
            }

            // 待ち牌を決め打ちして、14牌があがり型を成しているかどうか調べる
            for(auto extraTile = TileMin; extraTile <= TileMax; ++extraTile) {
                // 同種の5牌目はない
                if (table_.GetRemainingSize(extraTile) >= SizeOfOneTile) {
                    continue;
                }

                table_.PushTiles(extraTile, 1);
                for(auto pairTile = TileMin; pairTile <= TileMax; ++pairTile) {
                    if (table_.GetRemainingSize(pairTile) >= 2) {
                        // 同種の牌が2,3,4枚あったら対子として扱う
                        TilesWithPair tiles(extraTile, pairTile, table_, localStrTable_, allStrTable_);
                        str += search(tiles);
                    }
                }
                table_.PopTiles(extraTile, 1);
            }

            table_.Reset();
            return str;
        }

    private:
        const std::string search(TilesWithPair& tiles) {
            if (tiles.IsComplete()) {
                // 上がり形
                return tiles.ToString();
            }

            std::string str;
            while(tiles.SearchNext()) {
                // 上がり形に近づける
                str += search(tiles);
                tiles.Revert();
            }

            return str;
        }

        KeyTileSetTable table_;
        TileTable initialTable_;
        TileStrTable& allStrTable_;
        TileStrTable  localStrTable_;
        bool fail_;
    };

    // すべての牌の組み合わせ
    class AllTileSet {
    public:
        AllTileSet(void) = default;
        virtual ~AllTileSet(void) = default;
        AllTileSet(const AllTileSet&) = delete;
        AllTileSet& operator=(const AllTileSet&) = delete;

        // 例題に失敗したらtrue, 成功したらfalse
        virtual bool ExecuteTest(void) {
            bool failed = false;
            failed |= executeTest("1112224588899", "(111)(222)(888)(99)[45]\n");

            failed |= executeTest("1122335556799",
                                  "(123)(123)(567)(99)[55]\n(123)(123)(555)(99)[67]\n(123)(123)(567)(55)[99]\n");

            failed |= executeTest("1112223335559", "(111)(222)(333)(555)[9]\n(123)(123)(123)(555)[9]\n");

            failed |= executeTest("1223344888999",
                                  "(234)(234)(888)(999)[1]\n(123)(888)(999)(44)[23]\n(123)(234)(888)(999)[4]\n");

            failed |= executeTest("1112345678999",
                                  "(123)(456)(789)(99)[11]\n(111)(456)(789)(99)[23]\n(111)(345)(678)(999)[2]\n"
                                  "(345)(678)(999)(11)[12]\n(123)(678)(999)(11)[45]\n(111)(234)(789)(99)[56]\n"
                                  "(111)(234)(678)(999)[5]\n(123)(456)(999)(11)[78]\n(111)(234)(567)(99)[89]\n"
                                  "(111)(234)(567)(999)[8]\n(123)(456)(789)(11)[99]\n");

            return failed;
        }

        // 牌の組み合わせを数え上げて、結果を出力ストリームに格納する
        virtual void Enumerate(std::ostream& output) {
            TileTable table;
            std::string str;
            enumerate(str, TileMin, SizeOfCompleteTiles - 1, table, output);
        }

    private:
        // 例題を一つ解く。失敗したらtrue, 成功したらfalse
        bool executeTest(const std::string& input, const std::string& expected) {
            bool failed = false;

            TileFullSet s(input, strTable_);
            const auto actual = s.SearchAll();

            std::cout << "Testing " << input << " ... ";
            if (expected == actual) {
                std::cout << "passed\n";
            } else {
                std::cout << "failed\nexpected\n" << expected << "\nactual\n" << actual << "\n";
                failed = true;
            }

            return failed;
        }

        // 再帰的に牌を設定する
        // inputStr  : 手牌を表現する文字列("11223")
        // head      : ここで加える牌の番号
        // remaining : あと加えなければならない牌の数
        void enumerate(const std::string& inputStr, TileType head, TileSize remaining,
                       TileTable& table, std::ostream& output) {
            if (remaining == 0) {
                search(inputStr, table, output);
                return;
            }

            if (head > TileMax) {
                return;
            }

            // 小さい番号の牌からたくさん集める(0個を含む)
            TileSize i = std::min(remaining, SizeOfOneTile) + 1;
            do {
                --i;
                table[head] = i;
                std::string newStr = inputStr;
                for(TileSize j = 0; j < i; ++j) {
                    newStr += ConvertToChar(head);
                }
                enumerate(newStr, head + 1, remaining - i, table, output);
                table[head] = 0;
            } while (i > 0);

            return;
        }

        // ある牌の組み合わせについてを待ちを取得する
        void search(const std::string& inputStr, const TileTable& table, std::ostream& output) {
            TileFullSet s(table, strTable_);
            std::string str = s.SearchAll();

            if (str.empty()) {
                str += "(none)\n";
            }

            output << inputStr << ":\n" << str;
            return;
        }

        TileStrTable strTable_;
    };
}

// 引数を何かつけると、すべての牌の組み合わせについてまとめて標準出力に書き出す
// 引数がないときは、それぞれ牌の組み合わせについて標準出力に書き出す
int main(int argc, char* argv[]) {
    const bool printAtOnce = (argc > 1);
    CountTiles::AllTileSet allTileSet;

    // 例題が解けることを確認する
    if (allTileSet.ExecuteTest()) {
        return 1;
    }

    std::ostringstream oss;
    std::ostream& output = (printAtOnce) ? oss : std::cout;
    allTileSet.Enumerate(output);

    if (printAtOnce) {
        std::cout << oss.str();
    }

    return 0;
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
