/*
 * 出題元
 * http://www.itmedia.co.jp/enterprise/articles/1004/03/news002_2.html
 */

#include <algorithm>
#include <array>
#include <iostream>
#include <nmmintrin.h>
#include "countTilesBits.hpp"

using namespace TileSetSolver;

// 対子、刻子または順子
class TileSet {
public:
    inline TileSet(void) : tileMap_(0), open_(false) {
        return;
    }

    inline TileSet(TileMap tileMap) : tileMap_(tileMap), open_(false) {
        return;
    }

    inline TileSet(TileMap tileMap, bool open) : tileMap_(tileMap), open_(open) {
        return;
    }

    inline TileMap GetValue() const {
        return tileMap_;
    }

    inline TileKey GetKey(void) {
        TileKey tileKey = 0;

        asm (
            ".set  RegPattern,  r13 \n\t"
            ".set  RegPos,      r14 \n\t"
            ".set  RegMask,     r15 \n\t"

            // openならビットを立てる
            "mov     RegPattern, 1 << 5 \n\t"
            "xor     %0, %0 \n\t"
            "or      %2, %2 \n\t"
            "cmovnz  %0, RegPattern \n\t"

            // ビットマスク 10000100111bで、順子と刻子を同時に取り出す
            "mov     RegPattern, 0x427 \n\t"
            "tzcnt   RegPos, %1  \n\t"
            // 全bitが0ということはない

            "shlx    RegMask, RegPattern, RegPos \n\t"
            "pext    RegMask, %1, RegMask \n\t"
            // 最右のbit位置を4で割る。1牌5ビットなので、表で調整する。
            "shr     RegPos, 2 \n\t"
            // メモリの範囲内に収める
            "and     RegPos,  0xf \n\t"
            "and     RegMask, 0x1f \n\t"
            "or      %0, RegMask \n\t"
            "shl     %0, 4 \n\t"         // 0x0-0xf
            "or      %0, RegPos \n\t"
            :"=&r"(tileKey):"r"(tileMap_),"r"(open_):"r13","r14","r15");

        return tileKey;
    }

    inline static SizeType Print(TileKey tileKey, ResultString& str, SizeType currentLength) {
        TileMap parentheses = (tileKey & OpenKey) ? (']' + '[' * 256) : (')' + '(' * 256);
        TileMap strAsTileMap = tileMapToString(tileKey, parentheses);
        SizeType length = 0;

        asm volatile (
            // 8文字単位のビット数 = 文字数を探す
            "mov  r15, %1 \n\t"
            "bsr  %0, %1 \n\t"
            "shr  %0, 3  \n\t"
            "add  %0, 1  \n\t"

            // 32bitアラインメントは考慮しない(64bitアラインメントは必要)
            "mov  dword ptr [%2], r15d \n\t"
            "shr  r15, 32 \n\t"
            "mov  dword ptr [%2 + 4], r15d \n\t"
            :"=&r"(length):"r"(strAsTileMap),"r"(str.value + currentLength):"r15","memory");

        return (length + currentLength);
    }

    // 左括弧、文字列、右括弧、NUL終端をつけた文字列を整数として返す
    inline static TileMap tileMapToString(TileKey tileKey, TileMap parentheses) {
        // 牌の並び(1,2,3番目) + 足りない牌のシフト回数(枚数*8)
        static constexpr uint8_t patternTable[] = {
            0, 0, 0, 0,  1, 0, 0, 16,  0, 0, 0, 0,  1, 1, 0, 8,
            0, 0, 0, 0,  0, 0, 0, 0,   0, 0, 0, 0,  1, 1, 1, 0,
            0, 0, 0, 0,  2, 1, 0, 8,   0, 0, 0, 0,  0, 0, 0, 0,
            0, 0, 0, 0,  0, 0, 0, 0,   0, 0, 0, 0,  0, 0, 0, 0,
            0, 0, 0, 0,  3, 1, 0, 8,   0, 0, 0, 0,  0, 0, 0, 0,
            0, 0, 0, 0,  0, 0, 0, 0,   0, 0, 0, 0,  0, 0, 0, 0,
            0, 0, 0, 0,  3, 2, 1, 0,   0, 0, 0, 0,  0, 0, 0, 0,
            0, 0, 0, 0,  0, 0, 0, 0,   0, 0, 0, 0,  0, 0, 0, 0
        };

        static constexpr uint32_t baseTable[] = {
            '0', '1', '2', '3', '3',  '4', '5', '6', '7', '7',  '8', 'a', 'a', 'a', 'a', 'a'};

        TileMap strAsTileMap = 0;
        asm (
            ".set  RegLeft,      r12 \n\t"
            ".set  RegPattern,   r13 \n\t"
            ".set  RegPatternD,  r13d \n\t"
            ".set  RegBaseChar,  r14 \n\t"
            ".set  RegBaseCharD, r14d \n\t"
            ".set  RegChar,      r15 \n\t"
            ".set  RegCharD,     r15d \n\t"

            "mov   %0, %2 \n\t"
            "and   %0, 0xff \n\t"       // 左括弧
            "shl   %0, 8 \n\t"
            "mov   RegLeft, %2 \n\t"
            "shr   RegLeft, 8 \n\t"
            "and   RegLeft, 0xff \n\t"  // 右括弧

            // open (1bit) : patternTableのインデックス (5bits) : RegPos / 4 (4bits)
            "mov   RegPattern, %1 \n\t"
            "shr   RegPattern, 4  \n\t"
            "and   RegPattern, 0x1f \n\t"
            "mov   RegPatternD, [%3 + RegPattern * 4] \n\t"

            "mov   RegBaseChar, %1 \n\t"
            "and   RegBaseChar, 0xf \n\t"
            "mov   RegBaseCharD, [%4 + RegBaseChar * 4] \n\t"

            // 1文字目
            "mov   RegCharD, RegPatternD \n\t"
            "and   RegCharD, 0xff \n\t"
            "add   RegCharD, RegBaseCharD \n\t"
            "or    %0, RegChar \n\t"
            "shl   %0, 8 \n\t"
            "shr   RegPatternD, 8 \n\t"

            // 2文字目
            "mov   RegCharD, RegPatternD \n\t"
            "and   RegCharD, 0xff \n\t"
            "add   RegCharD, RegBaseCharD \n\t"
            "or    %0, RegChar \n\t"
            "shl   %0, 8 \n\t"
            "shr   RegPatternD, 8 \n\t"

            // 3文字目
            "mov   RegCharD, RegPatternD \n\t"
            "and   RegCharD, 0xff \n\t"
            "add   RegCharD, RegBaseCharD \n\t"
            "or    %0, RegChar \n\t"
            "shr   RegPatternD, 8 \n\t"

            // 余分な文字を削除する
            "shrx  %0, %0, RegPattern \n\t"
            // 右括弧
            "shl   %0, 8 \n\t"
            "or    %0, RegLeft \n\t"
            :"=&r"(strAsTileMap):"r"(tileKey),"r"(parentheses),"r"(patternTable),"r"(baseTable):"r12","r13","r14","r15");

        return strAsTileMap;
    }

private:
    TileMap tileMap_;  // 1..3牌
    bool    open_;     // 待ち型
};

// 対子 + 刻子または順子 * 4
class TileFullSet {
public:
    using SetArray = std::vector<TileFullSet>;  // 13牌の集合

    inline void Set(TileMap tileMap, SizeType index) {
        TileSet tileSet(tileMap);
        tileSetArray_.at(index) = std::move(tileSet);
        return;
    }

    // tileSetを追加する。元のtileSetは以後使えない。
    inline void Set(TileSet& tileSet, SizeType index) {
        tileSetArray_.at(index) = std::move(tileSet);
        return;
    }

    inline void Filter(TileIndex extra, KeyArray& keyArray, ResultStringArray& stringArray) {
        TileMap mask = 0xf;
        mask <<= ((extra - 1) * SizeOfBitsPerTile);

        SizeType i = 0;
        for(auto& tileSet : tileSetArray_) {
            TileMap newTileMap;
            const auto oldTileSet = tileSet.GetValue();

            // oldTileSetにextraがあれば取り除いてnewTileMapに入れる
            asm (
                "mov  r15, %1 \n\t"
                "shr  r15, 1  \n\t"
                "and  r15, %2 \n\t"
                "andn %0,  %2, %1 \n\t"
                "or   %0,  r15 \n\t"
                :"=&r"(newTileMap):"r"(oldTileSet),"r"(mask):"r15");

            if (newTileMap != oldTileSet) {
                TileSet newSet(newTileMap, true);
                auto newFullSet = *this;
                newFullSet.Set(newSet, i);

                auto key = newFullSet.GetKey();
                if (std::find_if(keyArray.begin(), keyArray.end(),
                                 [&](TileKey arg) -> bool
                                 {return (arg == key); }) == keyArray.end()) {
                    keyArray.push_back(key);
                    stringArray.push_back(TileFullSet::Print(key));
                }
            }
            ++i;
        }

        return;
    }

    inline TileKey GetKey(void) {
        TileKey openKey = 0;
        TileKey key1 = 0;
        TileKey key2 = 0;
        TileKey key3 = 0;
        TileKey key4 = 0;
        SizeType i = 0;

        for(auto& tileSet : tileSetArray_) {
            TileKey tileKey = tileSet.GetKey();

            if ((tileKey & OpenKey) != 0) {
                openKey = tileKey;
                continue;
            }

            // 挿入ソート
            switch(i) {
            case 1:
                key2 = tileKey;
                asm (
                    "mov    r15, %0 \n\t"
                    "cmp    %1,  %0 \n\t"
                    "cmovg  %0,  %1 \n\t"
                    "cmovg  %1, r15 \n\t"
                    :"+r"(key1),"+r"(key2)::"r15");
                break;
            case 2:
                key3 = tileKey;
                asm (
                    "mov    r15, %1 \n\t"
                    "cmp    %2,  %1 \n\t"
                    "cmovg  %1,  %2 \n\t"
                    "cmovg  %2,  r15 \n\t"

                    "mov    r15, %0 \n\t"
                    "cmp    %1,  %0 \n\t"
                    "cmovg  %0,  %1 \n\t"
                    "cmovg  %1, r15 \n\t"
                    :"+r"(key1),"+r"(key2),"+r"(key3)::"r15");
                break;
            case 3:
                key4 = tileKey;
                asm (
                    "mov    r15, %2 \n\t"
                    "cmp    %3,  %2 \n\t"
                    "cmovg  %2,  %3 \n\t"
                    "cmovg  %3,  r15 \n\t"

                    "mov    r15, %1 \n\t"
                    "cmp    %2,  %1 \n\t"
                    "cmovg  %1,  %2 \n\t"
                    "cmovg  %2,  r15 \n\t"

                    "mov    r15, %0 \n\t"
                    "cmp    %1,  %0 \n\t"
                    "cmovg  %0,  %1 \n\t"
                    "cmovg  %1, r15 \n\t"
                    :"+r"(key1),"+r"(key2),"+r"(key3),"+r"(key4)::"r15");
                break;
            default:
                key1 = tileKey;
                break;
            }
            ++i;
        }

        TileKey totalTileKey = openKey;
        totalTileKey <<= SizeOfKeyBits;
        totalTileKey |= key1;
        totalTileKey <<= SizeOfKeyBits;
        totalTileKey |= key2;
        totalTileKey <<= SizeOfKeyBits;
        totalTileKey |= key3;
        totalTileKey <<= SizeOfKeyBits;
        totalTileKey |= key4;
        return totalTileKey;
    }

    inline static ResultString Print(TileKey tileKey) {
        ResultString str;
        SizeType length = 0;

        constexpr TileKey mask = (1 << SizeOfKeyBits) - 1;
        TileKey rest = tileKey;
        for(SizeType i = 0; i < SizeOfTileSet; ++i) {
            length = TileSet::Print(rest & mask, str, length);
            rest >>= SizeOfKeyBits;
        }

        str.value[ResultStringLength - 1] = '\n';
        str.value[ResultStringLength] = 0;
        return str;
    }

private:
    std::array<TileSet, SizeOfTileSet> tileSetArray_;
};

// 対子 + 刻子または順子 * 4 の組
class Solution {
public:
    // fullSetを追加する。元のfullSetは以後使えない。
    inline void Add(TileFullSet& fullSet) {
        fullSetArray_.push_back(std::move(fullSet));
    }

    // 決め打ちした牌を除いて解を作る
    inline void Filter(TileIndex extra, KeyArray& keyArray, ResultStringArray& stringArray) {
        for(auto& fullSet : fullSetArray_) {
            fullSet.Filter(extra, keyArray, stringArray);
        }
    }

private:
    std::vector<TileFullSet> fullSetArray_;
};

class Puzzle {
public:
    inline Puzzle(TileMap src) : src_(src) {}

    inline std::string Find(void){
        ResultStringArray stringArray;
        findAll(src_, stringArray);

        // テスト用に「待ち無し」を返す
        if (stringArray.empty()) {
            std::string noneResult {"(none)\n"};
            return noneResult;
        }

        std::string result;
        for(auto str : stringArray) {
            result += str.value;
        }
        return result;
    }

private:
    // tileMapの待ちを調べる
    inline void findAll(TileMap tileMap, ResultStringArray& stringArray) {
        constexpr TileMap mask5th = 0x108421084210ull;  // 1..9のいずれかに5牌目がある
        TileMap lowerMask = 1;
        TileMap fullMask = 0x1f;
        KeyArray keyArray;

        // extraを待ちと決め打ちして調べる
        for(TileIndex extra=1; extra<=TileMax; ++extra) {
            TileMap newTileMap = 0;

            asm (
                // 1牌増やす
                "mov   r14, %1  \n\t"
                "and   r14, %3  \n\t"
                "andn  r15, %3, %1 \n\t"
                "shl   r14, 1   \n\t"
                "or    r14, %2  \n\t"
                "or    r15, r14 \n\t"

                // 5牌目があったら0を返す
                "xor   %0, %0 \n\t"
                "test  %4, r15 \n\t"
                "cmovz %0, r15 \n\t"
                :"=&r"(newTileMap):"r"(tileMap),"r"(lowerMask),"r"(fullMask),"r"(mask5th):"r14","r15");

            if (newTileMap != 0) {
                findWithExtra(newTileMap, extra, keyArray, stringArray);
            }

            lowerMask <<= SizeOfBitsPerTile;
            fullMask <<= SizeOfBitsPerTile;
        }

        return;
    }

    // tileMapに待ちextraを決め打ちして待ちを調べる
    inline void findWithExtra(TileMap tileMap, TileMap extra,
                              KeyArray& keyArray, ResultStringArray& stringArray) {
        TileMap lowerMask = 3;
        TileMap fullMask = 0x1f;

        // 1..9 の対子について調べる
        for(TileIndex i=TileMin; i<=TileMax; ++i) {
            TileMap tilePair = 0;
            TileMap rest = 0;

            asm (
                // 対子を取り除いた残り
                "andn   %1, %4, %2  \n\t"
                "mov    r15, %2  \n\t"
                "and    r15, %4  \n\t"
                "shr    r15, 2   \n\t"
                "and    r15, %4  \n\t"
                "or     %1, r15  \n\t"

                // 対子
                "mov    r15, %2  \n\t"
                "and    r15, %3  \n\t"

                // 対子があれば返す
                "xor    %0, %0   \n\t"
                "cmp    r15, %3  \n\t"
                "cmovz  %0, %3   \n\t"
                :"=&r"(tilePair),"=&r"(rest):"r"(tileMap),"r"(lowerMask),"r"(fullMask):"r15");

            lowerMask <<= SizeOfBitsPerTile;
            fullMask <<= SizeOfBitsPerTile;

            if (tilePair) {
                TileFullSet fullSet;
                Solution solution;
                TileSet tileSet(tilePair);
                fullSet.Set(tileSet, 0);
                splitTileMap(rest, fullSet, 1, false, solution);
                solution.Filter(extra, keyArray, stringArray);
            }
        }
    }

    void splitTileMap(TileMap tileMap, TileFullSet fullSet, SizeType depth, bool noTriple, Solution& solution) {
        constexpr TileMap tripleLowerMask = 7;   //  111b を
        constexpr TileMap tripleFullMask  = 15;  // 1111b から取り出して
        constexpr TileMap tripleUpperMask = 8;   // 1000b を残す
        constexpr TileMap sequenceLowerMask =  0x421;  //     10000100001b を
        constexpr TileMap sequenceFullMask  = 0x3def;  // 011110111101111b から取り出して
        constexpr TileMap sequenceUpperMask = 0x7bde;  // 111101111011110b を残す
        constexpr auto finalSizeOfTileSet = SizeOfTileSet - 1;
        const auto newDepath = depth + 1;

        TileMap triple = 0;
        TileMap tripleRest = 0;
        if (!noTriple) {
            splitWithMask(tileMap, tripleLowerMask, tripleFullMask, tripleUpperMask, triple, tripleRest);
        }

        TileMap sequence = 0;
        TileMap sequenceRest = 0;
        splitWithMask(tileMap, sequenceLowerMask, sequenceFullMask, sequenceUpperMask, sequence, sequenceRest);

        noTriple |= (triple == 0);
        if (depth == finalSizeOfTileSet) {
            if (triple) {
                auto newFullSet = fullSet;
                TileMap tileMap(triple);
                newFullSet.Set(tileMap, depth);
                solution.Add(newFullSet);
            }

            if (sequence) {
                auto newFullSet = fullSet;
                TileMap tileMap(sequence);
                newFullSet.Set(tileMap, depth);
                solution.Add(newFullSet);
            }
        } else {
            if (triple) {
                auto newFullSet = fullSet;
                TileMap tileMap(triple);
                newFullSet.Set(tileMap, depth);
                splitTileMap(tripleRest, newFullSet, newDepath, noTriple, solution);
            }

            if (sequence) {
                auto newFullSet = fullSet;
                TileMap tileMap(sequence);
                newFullSet.Set(tileMap, depth);
                splitTileMap(sequenceRest, newFullSet, newDepath, noTriple, solution);
            }
        }

        return;
    }

    // tileMapからlowerMaskを取り出して、取り出せたらextractedに、残りをrestに入れる
    // lowerMask : 各桁から取り出すbitの集合
    // fullMask  : 各桁の全5bitsの集合
    // upperMask : 各桁から取り出した後の残りbitの集合
    inline void splitWithMask(TileMap tileMap, TileMap lowerMask, TileMap fullMask, TileMap upperMask,
                              TileMap& extracted, TileMap& rest) {
        asm (
            "mov  %1, %2  \n\t"
            "mov  r15, %2  \n\t"
            "and  r15, %3  \n\t"
            "cmp  r15, %3  \n\t"
            "jz   2f \n\t"

            "mov  ecx, 8 \n\t"
            "1: \n\t"
            // 次の牌の組を調べる
            "shl  %3, 5  \n\t"
            "shl  %4, 5  \n\t"
            "shl  %5, 5  \n\t"
            "mov  r15, %2  \n\t"
            "and  r15, %3  \n\t"
            "cmp  r15, %3  \n\t"
            "loopne 1b \n\t"
            "jnz    3f \n\t"

            // ビットパターンが見つかったので取り出す
            "2: \n\t"
            "pext  r15, %2,  %5  \n\t"
            "pdep  r15, r15, %4  \n\t"
            "andn  %1, %4, %1 \n\t"
            "or    %1, r15    \n\t"

            // ビットパターンが見つかったらその値、見つからなければ0を返す
            "3: \n\t"
            "xor  %0, %0  \n\t"
            "cmp  %1, %2  \n\t"
            "cmovnz  %0, %3  \n\t"
            :"=&r"(extracted),"=&r"(rest),"+r"(tileMap),"+r"(lowerMask),"+r"(fullMask),"+r"(upperMask)::"rcx","r15");

        return;
    }

    TileMap src_;
};

namespace {
    // 待ち形の順列で、numberの次を見つけて、nextNumberに設定する
    // これ以上待ち形がないときは非0を、あれば0返す
    // numberの待ち形をtileMapに設定する
    // numberの待ち形を解いて文字列を設定する場合は、enablePatternにfalseを、設定しないときはfalseを設定する
    inline TileMap enumerateOne(bool enablePattern, TileMap number,
                                TileMap& tileMap, TileMap& nextNumber, StrArray& result) {
        TileMap invalid = 0;
        TileMap enablePatternQ = enablePattern;

        // enablePattern をメモリ渡しする
        union PatternCharSet {
            char str[SizeOfCompleteTiles + 3];  // 文字列
            __m128 xReg[2];  // XMMレジスタのアラインメント用
        };
        static_assert((alignof(PatternCharSet) % 16) == 0, "Unexpected xmmRegister alignment");

        // XMMレジスタから文字を取り出すパターンを用意して、文字列を受け取る
        // 最後の一文字は受け取らないので、オーバラン防止用に0固定にする
        // 複数byte単位より、1byte単位の方が速い
        PatternCharSet patternCharSet = {{14, 5, 13, 4, 12, 3, 11, 2, 10, 1, 9, 0, 8, 6, 7, 15, 0}};

        union XmmPatternSet {
            uint8_t value[48];
            __m128  xReg[3];   // XMMレジスタのアラインメント用
        };

        static const XmmPatternSet xmmPatternSet = {{
            9,9,9,9, 8,8,8,8, 7,7,7,7, 6,0,0,0,            // これ以上は牌を列挙するパターンがない最後のパターン
            8,0,9,1, 10,2,11,3, 12,4,13,5, 14,255,255,255, // 奇数番目と偶数番目の桁をvpshufbで入れ替えるパターン
            0x34, 0x33, 0x23, 0x22, 0x12, 0x11, 0x01, 0,   // 0x1111222233334を各桁に足す
            0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 // vpcmpeqb の結果を取り出すビットマスク
        }};
        register auto pXmmValueSet asm ("r13") = &xmmPatternSet;  // asm実行後は破壊される

        asm (
            ".set  RegTileMap,    rax \n\t"
            ".set  RegNextNumber, rbx \n\t"
            ".set  RegInvalid,    rdi \n\t"
            ".set  RegInvalidD,   edi \n\t"
            ".set  RegNumber,     rdx \n\t"
            ".set  RegPatternCharSet, rsi \n\t"
            ".set  RegXmmPtr, r13  \n\t"  // 破壊するレジスタ変数
            ".set  RegFour,   r14  \n\t"
            ".set  RegFourD,  r14d \n\t"
            ".set  RegOne,    r15  \n\t"
            ".set  RegOneD,   r15d \n\t"

            "xor   RegInvalidD, RegInvalidD \n\t"
            "mov   RegFourD, 4 \n\t"  // 定数4
            "mov   RegOneD,  1 \n\t"  // 定数1

            // numberを文字列にする
            ".set  RegOddSet,    r8 \n\t"
            ".set  RegEvenSet,   r9 \n\t"
            ".set  RegDigitSet,  r10 \n\t"
            ".set  RegDigitSetD, r10d \n\t"
            ".set  XRegConst,     xmm12 \n\t"
            ".set  XRegCmpResult, xmm13 \n\t"
            ".set  XRegString,    xmm14 \n\t"
            ".set  XRegDigitSet,  xmm15 \n\t"
            ".set  ConstCharZero, '0'  \n\t"

            // 奇数番目の桁と偶数番目の桁を分ける
            "mov     RegDigitSet, 0xf0f0f0f0f0f0f \n\t"
            "mov     RegOddSet,  RegNumber \n\t"
            "shr     RegOddSet,  4 \n\t"
            "mov     RegEvenSet, RegNumber \n\t"
            "and     RegOddSet,  RegDigitSet \n\t"
            "and     RegEvenSet, RegDigitSet \n\t"

            // 奇数番目の桁と偶数番目の桁を順序通りに戻す
            "vpinsrq  XRegCmpResult, XRegCmpResult, RegOddSet,  0 \n\t"
            "vpinsrq  XRegCmpResult, XRegCmpResult, RegEvenSet, 1 \n\t"
            "vpshufb  XRegCmpResult, XRegCmpResult, [RegXmmPtr + 16] \n\t"
            // 最後のパターンと比較する
            "vpcmpeqb XRegCmpResult, XRegCmpResult, [RegXmmPtr] \n\t"
            // 後で使う
            "vmovdqa  XRegConst, [RegXmmPtr + 32] \n\t"
            // 以後RegXmmPtrは別用途に使う

            "or    ecx, ecx \n\t"
            "jz    31f \n\t"

            // xmmレジスタを'0'で埋める
            "mov      RegDigitSetD, ConstCharZero \n\t"
            "vpinsrd  XRegString, XRegString, RegDigitSetD, 0 \n\t"
            "vpxor    XRegDigitSet, XRegDigitSet, XRegDigitSet \n\t"
            "vpshufb  XRegString, XRegString, XRegDigitSet \n\t"

            // 最上位の2 bytesを"\n:"-"00"にする
            "mov      RegDigitSetD, 0xda0a \n\t"
            "shl      RegDigitSet, 48 \n\t"
            "or       RegOddSet, RegDigitSet \n\t"
            "vpinsrq  XRegDigitSet, XRegDigitSet, RegOddSet, 0 \n\t"

            // 最上位のbyteを-'0'にする
            "mov      RegDigitSetD, -ConstCharZero \n\t"
            "shl      RegDigitSet, 56 \n\t"
            "or       RegEvenSet, RegDigitSet \n\t"
            "vpinsrq  XRegDigitSet, XRegDigitSet, RegEvenSet, 1 \n\t"
            // 各byteはwrap aroundする
            "vpaddb   XRegString, XRegString, XRegDigitSet  \n\t"

            "vpshufb  XRegString, XRegString, [RegPatternCharSet]  \n\t"
            "vmovdqa  [RegPatternCharSet], XRegString  \n\t"

            // numberをビットマップにする
            ".set   RegTargetDigit,   r8   \n\t"
            ".set   RegTargetDigitD,  r8d  \n\t"
            ".set   RegRest,          r9   \n\t"
            ".set   RegRestD,         r9d  \n\t"
            ".set   RegPrevDigitD,    r10d \n\t"
            ".set   RegDigitBits,     r11  \n\t"
            ".set   RegDigitBitsD,    r11d \n\t"
            ".set   RegDigitBitMaskD, r12d \n\t"
            ".set   RegWork21,        r13  \n\t"
            ".set   RegWork21D,       r13d \n\t"

            "mov    RegRest, RegNumber \n\t"
            "xor    RegPrevDigitD, RegPrevDigitD \n\t"
            "xor    RegDigitBitsD, RegDigitBitsD \n\t"
            "mov    RegDigitBitMaskD, 0xf \n\t"
            "xor    eax, eax \n\t"
            "mov    ecx, 12 \n\t"  // 最後の桁は別処理

            "21: \n\t"
            // 一桁取り出す
            "mov    RegTargetDigit,  RegRest \n\t"
            "and    RegTargetDigitD, RegDigitBitMaskD \n\t"
            "shr    RegRest, 4 \n\t"

            // 桁の数字が前(隣)と同じだったら、何牌あるかはそのままにする
            // 異なるときは0に戻す
            "xor    RegWork21D,    RegWork21D  \n\t"
            "cmp    RegPrevDigitD, RegTargetDigitD \n\t"
            "cmovnz RegDigitBitsD, RegWork21D  \n\t"
            "shl    RegDigitBitsD, 1 \n\t"
            "or     RegDigitBitsD, RegOneD \n\t"
            "mov    RegPrevDigitD, RegTargetDigitD \n\t"

            // 桁の数字を5倍にする
            "sub    RegTargetDigitD, RegOneD \n\t"
            "mov    RegWork21D, RegTargetDigitD \n\t"
            "shl    RegWork21D, 2 \n\t"
            "add    RegTargetDigitD, RegWork21D \n\t"

            // .. -> ..1 に増やす
            "shlx   RegWork21, RegDigitBits, RegTargetDigit \n\t"
            "or     rax, RegWork21 \n\t"
            "loop   21b \n\t"

            // 13桁目
            "xor    RegWork21D,    RegWork21D \n\t"
            "cmp    RegPrevDigitD, RegRestD   \n\t"
            "cmovnz RegDigitBitsD, RegWork21D  \n\t"
            "shl    RegDigitBitsD, 1 \n\t"
            "or     RegDigitBitsD, RegOneD \n\t"

            "sub    RegRestD, RegOneD \n\t"
            "mov    RegWork21D, RegRestD \n\t"
            "shl    RegWork21D, 2 \n\t"
            "add    RegRestD, RegWork21D \n\t"

            "shlx   RegWork21, RegDigitBits, RegRest \n\t"
            "or     rax, RegWork21 \n\t"

            // 次の候補を探す
            "31: \n\t"
            ".set  RegTilePos,   rcx  \n\t"
            ".set  RegTilePosD,  ecx  \n\t"
            ".set  RegBitMask,   r8   \n\t"
            ".set  RegDigit,     r9   \n\t"
            ".set  RegDigitD,    r9d  \n\t"
            ".set  RegDigitEx,   r10  \n\t"
            ".set  RegDigitExD,  r10d \n\t"
            ".set  RegPattern,   r11  \n\t"
            ".set  RegCountLow,  r12  \n\t"

            // 等しい桁は0xff, 異なる桁は0 : MSBだけ取り出す
            "vpextrq  RegPattern, XRegConst,     1 \n\t"
            "vpextrq  RegDigit,   XRegCmpResult, 0 \n\t"
            "vpextrq  RegDigitEx, XRegCmpResult, 1 \n\t"

            // 各byteのLSBだけ取り出して、16bitにまとめる。上位3桁は元々0なので、111bになる。
            "pext   RegDigit, RegDigit, RegPattern \n\t"
            "pext   RegDigitEx, RegDigitEx, RegPattern \n\t"
            "shl    RegDigitExD, 8 \n\t"
            "or     RegDigitD, RegDigitExD \n\t"

            // すべての桁を探したが、繰り上げられる桁が見つからなかった
            "cmp    RegDigitD, 0xffff \n\t"
            "jz     41f \n\t"

            // 最下桁から何桁等しいか調べる
            "not    RegDigitD \n\t"
            "tzcnt  RegDigitExD, RegDigitD \n\t"
            "mov    RegTilePosD, RegDigitExD \n\t"
            "shl    RegTilePosD, 2 \n\t"   // 置き換える最も上の桁のbyte位置
            "shrx   RegDigit,  RegNumber, RegTilePos \n\t"
            "and    RegDigitD, 0xf \n\t"    // 繰り上げる前の値

            "add   RegTilePos, RegFour \n\t"
            "mov   RegBitMask, -1 \n\t"
            "shlx  RegBitMask, RegBitMask, RegTilePos \n\t"
            "mov   RegNextNumber, RegNumber \n\t"
            "and   RegNextNumber, RegBitMask  \n\t"  // 繰り上げた後の値以外

            "mov   RegDigitExD, RegDigitD \n\t"
            "shl   RegDigitExD, 4 \n\t"
            "or    RegDigitD, RegDigitExD \n\t"
            "mov   RegDigitExD, RegDigitD \n\t"
            "shl   RegDigitExD, 8 \n\t"
            "or    RegDigitD, RegDigitExD \n\t"
            "mov   RegDigitExD, RegDigitD \n\t"
            "shl   RegDigitExD, 16 \n\t"
            "or    RegDigitD, RegDigitExD \n\t"

            "mov   RegDigitEx, RegDigit \n\t"
            "shl   RegDigitEx, 32 \n\t"
            "or    RegDigit, RegDigitEx \n\t"
            "vpextrq  RegDigitEx, XRegConst, 0 \n\t"
            "add   RegDigit, RegDigitEx \n\t"

            "mov   RegDigitExD, 13 * 4 \n\t"
            "sub   RegDigitExD, RegTilePosD \n\t"
            "shrx  RegDigit, RegDigit, RegDigitEx \n\t"
            "andn  RegDigit, RegBitMask, RegDigit \n\t"
            "or    RegNextNumber, RegDigit \n\t"
            "sub   RegInvalid, RegOne \n\t"  // -1にする

            "41: \n\t"
            "add   RegInvalid, RegOne \n\t"  // 0または1にする

            "42: \n\t"
            :"=&a"(tileMap),"=&b"(nextNumber),"+c"(enablePatternQ),"=&D"(invalid),"+r"(pXmmValueSet):"d"(number),"S"(patternCharSet.str):"r8","r9","r10","r11","r12","r14","r15","memory");

        if (enablePattern) {
            std::string patternStr = patternCharSet.str;
            Puzzle puzzle(tileMap);
            patternStr += puzzle.Find();
            result.push_back(std::move(patternStr));
        }

        return invalid;
    }
}

namespace TileSetSolver {
    // 各スレッドは、indexOffset番目(先頭は0)から、stepSize個間隔で、待ち形を求める
    void EnumerateAll(SizeType indexOffset, SizeType stepSize, StrArray& result) {
        decltype(indexOffset) patternIndex = 0;
        TileMap number = 0x1111222233334;  // 辞書順で一番小さいパターン
        TileMap tileMap = 0;
        TileMap nextNumber = 0;
        TileMap invalid = 0;

        // while{}より速い
        do {
            invalid = enumerateOne((patternIndex == indexOffset), number, tileMap, nextNumber, result);
            ++patternIndex;
            patternIndex = (patternIndex >= stepSize) ? 0 : patternIndex;
            number = nextNumber;
        } while(!invalid);
    }
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
