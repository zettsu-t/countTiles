/*
 * 出題元
 * http://www.itmedia.co.jp/enterprise/articles/1004/03/news002_2.html
 */

#include <algorithm>
#include <array>
#include <iostream>
#include <vector>
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

    inline void Print(std::string& str) {
        const char left = (open_) ? '[' : '(';
        const char right = (open_) ? ']' : ')';
        str += left;

        TileMap mask = 1;
        mask <<= SizeOfBitsPerTile;

        TileMap strAsTileMap[2] = {0, 0};
        strAsTileMap[0] = tileMapToString(tileMap_);
        str += reinterpret_cast<char*>(&strAsTileMap);
        str += right;
        return;
    }

    // right + 0 で終端する
    TileMap tileMapToString(TileMap tileMap) {
        TileMap strAsTileMap = 0;
        static constexpr char baseTable[] = {'0', '0', '1', '2', '2', '3', '4', '5', '6', '7', '7', '8', 'a', 'a', 'a', 'a'};
        static constexpr uint8_t patternTable[] = {
            0, 0, 0, 0,  0, 0, 0, 1,  0, 0, 0, 0,  0, 0, 1, 1,
            0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 1, 1, 1,
            0, 0, 0, 0,  0, 0, 2, 1,  0, 0, 0, 0,  0, 0, 0, 0,
            0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
            0, 0, 0, 0,  0, 0, 3, 1,  0, 0, 0, 0,  0, 0, 0, 0,
            0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
            0, 0, 0, 0,  0, 3, 2, 1,  0, 0, 0, 0,  0, 0, 0, 0,
            0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0
        };

        asm volatile (
            "mov   r8, 0x427 \n\t"
            "tzcnt r9, rbx  \n\t"
            "jz    2f \n\t"

            "shlx  r10, r8, r9 \n\t"
            "pext  r10, rbx, r10 \n\t"
            "shr   r9, 2 \n\t"
            "and   r9, 0xf \n\t"
            "and   r10, 0x1f \n\t"
            "xor   r8, r8 \n\t"
            "mov   r8b, [rsi + r9] \n\t"
            "mov   r9d, [rdi + r10 * 4] \n\t"
            "mov   r15, r9 \n\t"

            "xor   rax, rax \n\t"
            "mov   rcx, 4 \n\t"

            "1: \n\t"
            "mov   r10, r9 \n\t"
            "and   r10, 0xff \n\t"
            "mov   r11, r10 \n\t"
            "add   r11, r8 \n\t"
            "xor   r12, r12 \n\t"
            "or    r10, r10 \n\t"
            "cmovnz  r12, r11 \n\t"
            "shl   rax, 8 \n\t"
            "or    rax, r12 \n\t"
            "shr   r9, 8 \n\t"
            "loop  1b \n\t"

            "2: \n\t"
            :"=&a"(strAsTileMap):"b"(tileMap),"S"(baseTable),"D"(patternTable):"rcx","r8","r9","r10","r11","r12","r13","r14","r15","memory");

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

    inline void Filter(TileIndex extra, SetArray& newFullSetArray) {
        TileMap mask = 0xf;
        mask <<= (extra * SizeOfBitsPerTile);

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
                newFullSetArray.push_back(std::move(newFullSet));
            }
            ++i;
        }

        return;
    }

    inline void Print(StrArray& strArray) {
        std::sort(tileSetArray_.begin(), tileSetArray_.end(),
                  [](const TileSet& lhs, const TileSet& rhs) -> bool
                  { return (lhs.GetValue() < rhs.GetValue()); });

        std::string str;
        for(auto& tileSet : tileSetArray_) {
            tileSet.Print(str);
        }
        str += "\n";
        strArray.push_back(std::move(str));
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
    inline void Filter(TileIndex extra) {
        decltype(fullSetArray_) newFullSetArray;
        for(auto& fullSet : fullSetArray_) {
            fullSet.Filter(extra, newFullSetArray);
        }

        fullSetArray_.swap(newFullSetArray);
    }

    inline void Print(StrArray& strArray) {
        for(auto& fullSet : fullSetArray_) {
            fullSet.Print(strArray);
        }
    }

private:
    TileFullSet::SetArray fullSetArray_;
};

class Puzzle {
public:
    inline Puzzle(TileMap src) : src_(src) {}

    inline std::string Find(void){
        StrArray strArray;
        findAll(src_, strArray);

        // テスト用に「待ち無し」を返す
        if (strArray.empty()) {
            std::string noneResult {"(none)\n"};
            return noneResult;
        }

        std::string result;
        for(auto str : strArray) {
            result += str;
        }
        return result;
    }

private:
    // tileMapの待ちを調べる
    inline void findAll(TileMap tileMap, StrArray& strArray) {
        constexpr TileMap mask5th = 0x2108421084200ull;  // 1..9のいずれかに5牌目がある
        TileMap lowerMask = 1;
        TileMap fullMask = 0x1f;

        // extraを待ちと決め打ちして調べる
        for(TileIndex extra=1; extra<=TileMax; ++extra) {
            lowerMask <<= SizeOfBitsPerTile;
            fullMask <<= SizeOfBitsPerTile;
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
                findWithExtra(newTileMap, extra, strArray);
            }
        }

        // 重複を除く(要高速化)
        std::sort(strArray.begin(), strArray.end());
        strArray.erase(std::unique(strArray.begin(), strArray.end()), strArray.end());
        return;
    }

    // tileMapに待ちextraを決め打ちして待ちを調べる
    inline void findWithExtra(TileMap tileMap, TileMap extra, StrArray& strArray) {
        TileMap lowerMask = 3;
        TileMap fullMask = 0x1f;

        // 1..9 の対子について調べる
        for(TileIndex i=TileMin; i<=TileMax; ++i) {
            lowerMask <<= SizeOfBitsPerTile;
            fullMask <<= SizeOfBitsPerTile;
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

            if (!tilePair) {
                continue;
            }

            TileFullSet fullSet;
            Solution solution;
            TileSet tileSet(tilePair);
            fullSet.Set(tileSet, 0);
            splitTileMap(rest, fullSet, 1, false, solution);
            solution.Filter(extra);
            solution.Print(strArray);
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
            "mov  ecx, 10 \n\t"

            "1: \n\t"
            // 次の牌の組を調べる
            "shl  %3, 5  \n\t"
            "shl  %4, 5  \n\t"
            "shl  %5, 5  \n\t"
            "mov  r15, %2  \n\t"
            "and  r15, %3  \n\t"
            "cmp  r15, %3  \n\t"
            "loopne 1b \n\t"
            "jnz    2f \n\t"

            // ビットパターンが見つかったので取り出す
            "pext  r15, %2,  %5  \n\t"
            "pdep  r15, r15, %4  \n\t"
            "andn  %1, %4, %1 \n\t"
            "or    %1, r15    \n\t"

            // ビットパターンが見つかったらその値、見つからなければ0を返す
            "2: \n\t"
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
    // numberの待ち形をpatternStrを設定する場合は、enablePatternにfalseを、設定しないときはfalseを設定する
    inline TileMap enumerateOne(bool enablePattern, TileMap number,
                                TileMap& tileMap, TileMap& nextNumber, std::string& patternStr) {
        TileMap invalid = 0;
        TileMap enablePatternQ = enablePattern;

        // enablePattern をメモリ渡しする
        char patternCharSet[SizeOfCompleteTiles + 2] = {
            0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, ':', '\n', 0};

        asm (
            ".set  RegTileMap,    rax \n\t"
            ".set  RegNextNumber, rbx \n\t"
            ".set  RegInvalid,    rdi \n\t"
            ".set  RegInvalidD,   edi \n\t"
            ".set  RegNumber,     rdx \n\t"
            ".set  RegPatternCharSet, rsi \n\t"
            ".set  RegFour, r14 \n\t"
            ".set  RegOne,  r15 \n\t"

            "xor   RegInvalidD, RegInvalidD \n\t"  // falseにする
            "mov   RegFour, 4 \n\t"  // 定数4
            "mov   RegOne,  1 \n\t"  // 定数1

            "or    ecx, ecx \n\t"
            "jz    21f \n\t"

            // numberを文字列にする
            ".set  RegStrPtr, r8 \n\t"
            ".set  RegRest,   r9 \n\t"
            ".set  RegWork11, r10 \n\t"

            "mov   RegStrPtr, RegPatternCharSet \n\t"
            "sub   RegStrPtr, RegOne \n\t"
            "mov   RegRest,   RegNumber \n\t"
            "mov   ecx, 13 \n\t"

            "11: \n\t"
            "shr   RegRest, 4   \n\t"
            "mov   RegWork11, RegRest \n\t"
            "and   RegWork11, 0xf  \n\t"
            "add   RegWork11, 0x30 \n\t"  // '0'
            "mov   [RegStrPtr + rcx], r10b \n\t"
            "loop  11b\n\t"

            // numberをビットマップにする
            "21: \n\t"
            ".set  RegTilePos,    r8 \n\t"
            ".set  RegBitMask,    r9 \n\t"
            ".set  RegRest,       r10 \n\t"
            ".set  RegOutBitMask, r11 \n\t"
            ".set  RegWork21,     r12 \n\t"

            "mov   RegTilePos, RegFour \n\t"  // 同じ牌が4つある
            "mov   RegBitMask, 0xf \n\t"      // ビットマスク
            "mov   RegRest, RegNumber \n\t"
            "mov   RegOutBitMask, 0x1f \n\t"  // 11111b
            "xor   eax, eax \n\t"
            "mov   ecx, 13 \n\t"  // 牌の数

            "22: \n\t"
            "shr   RegRest, 4 \n\t"

            // 桁の数字を取り出して5倍にする
            "mov   RegTilePos, RegRest \n\t"
            "and   RegTilePos, RegBitMask \n\t"
            "mov   RegWork21,  RegTilePos \n\t"
            "shl   RegWork21,  2 \n\t"
            "add   RegTilePos, RegWork21 \n\t"

            "shlx  RegTilePos, RegOutBitMask, RegTilePos \n\t"
            "pext  RegWork21, rax, RegTilePos \n\t"
            // 111b -> 1111bに増やす
            "shl   RegWork21, 1 \n\t"
            "or    RegWork21, RegOne \n\t"
            "pdep  RegWork21, RegWork21, RegTilePos \n\t"
            "or    rax, RegWork21 \n\t"
            "loop  22b\n\t"

            // 次の候補を探す
            "31: \n\t"
            ".set RegDigit,        r10 \n\t"
            ".set RegPattern,      r11 \n\t"
            ".set RegCheckedDigit, r12 \n\t"
            ".set RegCountLow,     r13 \n\t"

            "mov  RegTilePos,  RegFour \n\t" // 同じ牌が4つある
            "mov  RegBitMask,  0xf0 \n\t"    // ビットマスク
            "mov  RegDigit,    9 \n\t"       // 下一桁を9から順に減らしていく
            "mov  ecx, 13 \n\t"  // ループ回数

            "32: \n\t"
            // 下の桁から上の桁に向かってRegDigitを探す
            "shlx  RegPattern, RegDigit, RegTilePos \n\t"
            "mov   RegCheckedDigit, RegNumber  \n\t"
            "and   RegCheckedDigit, RegBitMask \n\t"
            "cmp   RegCheckedDigit, RegPattern \n\t"
            "jnz   33f \n\t"

            // 桁が見つかった
            "xor   RegCheckedDigit, RegCheckedDigit \n\t"  // 見つける数字を減らすときは1, 減らさないときは0
            "mov   RegCountLow, rcx \n\t"  // 残り10, 6, 2桁になったら、見つける数字を減らす
            "and   RegCountLow, 3 \n\t"
            "cmp   RegCountLow, 2 \n\t"
            "cmovz RegCheckedDigit, RegOne \n\t"

            "sub   RegDigit, RegCheckedDigit \n\t"
            "shl   RegBitMask, 4 \n\t"
            "add   RegTilePos, RegFour \n\t"   // 一つ上の桁をみる
            "loop  32b \n\t"

            // すべての桁を探したが、繰り上げられる桁が見つからなかった
            "mov   RegInvalid, RegOne \n\t"
            "jmp   41f \n\t"

            "33: \n\t"
            "mov   RegDigit, rcx \n\t"  // 何桁目から下を繰り上げるか
            "mov   ecx, 14 \n\t"
            "sub   rcx, RegDigit \n\t"  // 何桁繰り上げるか
            "mov   RegCheckedDigit, RegNumber \n\t"
            "shrx  RegDigit, RegCheckedDigit, RegTilePos \n\t"  // 繰り上げた後の値
            "and   RegDigit, 0xf \n\t"
            "add   RegDigit, RegOne \n\t"

            ".set  RegCount,     r12 \n\t"
            ".set  RegDigitDiff, r13 \n\t"
            "xor   RegCount, RegCount \n\t"  // 何桁目で繰り上げる数字を1増やすか
            "mov   RegNextNumber, RegNumber \n\t"

            // 桁を繰り上げる
            "34: \n\t"
            "shlx  RegPattern, RegDigit, RegTilePos \n\t"
            "andn  RegNextNumber, RegBitMask, RegNextNumber \n\t"
            "or    RegNextNumber, RegPattern \n\t"

            // 同じ牌が続いたら、牌の番号を1増やす
            "xor   RegDigitDiff, RegDigitDiff \n\t"
            "add   RegCount, 1 \n\t"
            "test  RegCount, 3 \n\t"
            "cmovz r13, RegOne \n\t"
            "add   RegDigit, RegDigitDiff \n\t"
            // 一つ下の桁をみる
            "sub   RegTilePos, RegFour \n\t"
            "shr   RegBitMask, 4 \n\t"
            "loop  34b \n\t"

            "41: \n\t"
            :"=&a"(tileMap),"=&b"(nextNumber),"+c"(enablePatternQ),"=&D"(invalid):"d"(number),"S"(patternCharSet):"r8","r9","r10","r11","r12","r13","r14","r15","memory");

        patternStr = patternCharSet;
        return invalid;
    }
}

namespace TileSetSolver {
    // 各スレッドは、indexOffset番目(先頭は0)から、stepSize個間隔で、待ち形を求める
    void EnumerateAll(SizeType indexOffset, SizeType stepSize, StrArray& result) {
        decltype(indexOffset) patternIndex = 0;
        TileMap number = 0x11112222333340;  // 辞書順で一番小さいパターン
        TileMap tileMap = 0;
        TileMap nextNumber = 0;
        TileMap invalid = 0;

        do {
            bool enablePattern = (patternIndex == indexOffset);
            {
                std::string patternStr;
                invalid = enumerateOne(enablePattern, number, tileMap, nextNumber, patternStr);

                if (enablePattern) {
                    // ある牌の組み合わせについてを待ちを取得する
                    Puzzle puzzle(tileMap);
                    patternStr += puzzle.Find();
                    result.push_back(std::move(patternStr));
                }
            }

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
