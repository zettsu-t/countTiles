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

    inline TileMap GetValue() const {
        return tileMap_;
    }

    inline void SetOpen(void) {
        open_ = true;
        return;
    }

    inline void Print(std::string& str) {
        const char left = (open_) ? '[' : '(';
        const char right = (open_) ? ']' : ')';
        str += left;

        TileMap mask = 1;
        mask <<= SizeOfTileSet;

        TileMap strAsTileMap[2] = {0, 0};
        strAsTileMap[0] = tileMapToString(tileMap_);
        str += reinterpret_cast<char*>(&strAsTileMap);
        str += right;
        return;
    }

    // right + 0 で終端する
    TileMap tileMapToString(TileMap tileMap) {
        TileMap strAsTileMap = 0;
        const char baseTable[] = {'0', '0', '1', '2', '2', '3', '4', '5', '6', '7', '7', '8', 'a', 'a', 'a', 'a'};
        const uint8_t patternTable[] = {
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

    inline void Set(const TileSet& tileSet, SizeType index) {
        tileSetArray_.at(index) = tileSet;
        return;
    }

    inline void Filter(TileMap mask, SetArray& newFullSetArray) {
        SizeType i = 0;
        for(auto& tileSet : tileSetArray_) {
            TileMap newTileMap;
            asm volatile (
                "mov  r15, %1 \n\t"
                "shr  r15, 1  \n\t"
                "and  r15, %2 \n\t"
                "andn rax, %2, %1 \n\t"
                "or   rax, r15 \n\t"
                :"=&a"(newTileMap):"r"(tileSet.GetValue()),"r"(mask):"r15","memory");

            TileSet newSet {newTileMap};

            if (newSet.GetValue() != tileSet.GetValue()) {
                auto newFullSet = *this;
                newSet.SetOpen();
                newFullSet.Set(newSet, i);
                newFullSetArray.push_back(newFullSet);
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
        strArray.push_back(str);
    }

private:
    std::array<TileSet, SizeOfTileSet> tileSetArray_;
};

// 対子 + 刻子または順子 * 4 の組
class Solution {
public:
    inline void Add(const TileFullSet& arg) {
        fullSetArray_.push_back(arg);
    }

    // 決め打ちした牌を除いて解を作る
    inline void Filter(TileIndex extra) {
        decltype(fullSetArray_) newFullSetArray;
        TileMap mask = 0xf;
        mask <<= (extra * SizeOfTileSet);

        for(auto& fullSet : fullSetArray_) {
            fullSet.Filter(mask, newFullSetArray);
        }

        fullSetArray_.clear();
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
    inline void findAll(TileMap src, StrArray& strArray) {
        TileMap lowerMask = 1;
        TileMap fullMask = 0x1f;
        TileMap allMask = 0x2108421084200ull;

        for(TileIndex extra=1; extra<=TileMax; ++extra) {
            lowerMask <<= SizeOfTileSet;
            fullMask <<= SizeOfTileSet;
            TileMap newSrc = 0;

            asm volatile (
                "mov   r14, %1  \n\t"
                "and   r14, %3  \n\t"
                "andn  r15, %3, %1 \n\t"
                "shl   r14, 1   \n\t"
                "or    r14, %2  \n\t"
                "or    r15, r14 \n\t"
                "xor   rax, rax \n\t"
                "mov   r14, %4  \n\t"
                "and   r14, r15 \n\t"
                "cmovz rax, r15 \n\t"
                :"=&a"(newSrc):"r"(src),"r"(lowerMask),"r"(fullMask),"r"(allMask):"r14","r15","memory");

            if (newSrc != 0) {
                findWithExtra(newSrc, extra, strArray);
            }
        }

        std::sort(strArray.begin(), strArray.end());
        strArray.erase(std::unique(strArray.begin(), strArray.end()), strArray.end());
        return;
    }

    inline void findWithExtra(TileMap src, TileMap extra, StrArray& strArray) {
        TileMap lowerMask = 3;
        TileMap fullMask = 0x1f;
        for(TileIndex i=1; i<=TileMax; ++i) {
            lowerMask <<= SizeOfTileSet;
            fullMask <<= SizeOfTileSet;
            TileMap tilePair = 0;
            TileMap rest = 0;

            asm volatile (
                "andn  rbx, %4, %2 \n\t"
                "mov   r14, %2  \n\t"
                "and   r14, %4  \n\t"
                "shr   r14, 2   \n\t"
                "and   r14, %4  \n\t"
                "or    rbx, r14 \n\t"
                "mov   r15, %2  \n\t"
                "and   r15, %3  \n\t"
                "xor   rax, rax \n\t"
                "cmp   r15, %3  \n\t"
                "cmovz rax, %3  \n\t"
                :"=&a"(tilePair),"=&b"(rest):"r"(src),"r"(lowerMask),"r"(fullMask):"r14","r15","memory");

            if (tilePair) {
                TileFullSet fullSet;
                Solution solution;
                TileSet tileSet(tilePair);
                fullSet.Set(tileSet, 0);
                splitTileMap(rest, fullSet, solution, 1, false);
                solution.Filter(extra);
                solution.Print(strArray);
            }
        }
    }

    void splitTileMap(TileMap& src, TileFullSet& fullSet, Solution& solution, SizeType depth, bool noTriple) {
        TileMap triple = 0;
        TileMap tripleRest = 0;
        TileMap tripleLowerMask = 7;
        TileMap tripleFullMask = 0xf;
        TileMap tripleUpperMask = 8;
        auto foundTriple = splitWithMask(triple, tripleRest, src,
                                         tripleLowerMask, tripleFullMask, tripleUpperMask);

        TileMap sequence = 0;
        TileMap sequenceRest = 0;
        TileMap sequenceLowerMask = 0x421;
        TileMap sequenceFullMask = 0x3def;
        TileMap sequenceUpperMask = 0x7bde;
        auto foundSequence = splitWithMask(sequence, sequenceRest, src,
                                           sequenceLowerMask, sequenceFullMask, sequenceUpperMask);

        noTriple |= !foundTriple;
        if (depth == (SizeOfTileSet - 1)) {
            if (foundTriple) {
                auto newFullSet = fullSet;
                TileMap tileMap(triple);
                newFullSet.Set(tileMap, depth);
                solution.Add(newFullSet);
            }

            if (foundSequence) {
                auto newFullSet = fullSet;
                TileMap tileMap(sequence);
                newFullSet.Set(tileMap, depth);
                solution.Add(newFullSet);
            }
        } else {
            if (!noTriple && foundTriple) {
                auto newFullSet = fullSet;
                TileMap tileMap(triple);
                newFullSet.Set(tileMap, depth);
                splitTileMap(tripleRest, newFullSet, solution, depth + 1, noTriple);
            }

            if (foundSequence) {
                auto newFullSet = fullSet;
                TileMap tileMap(sequence);
                newFullSet.Set(tileMap, depth);
                splitTileMap(sequenceRest, newFullSet, solution, depth + 1, noTriple);
            }
        }

        return;
    }

    inline bool splitWithMask(TileMap& tileMap, TileMap& rest, TileMap& src,
                              TileMap& lowerMask, TileMap& fullMask, TileMap& upperMask) {
        asm volatile (
            "mov  r8,  rcx \n\t"
            "mov  ecx, 10  \n\t"
            "mov  rbx, rdx \n\t"
            "1: \n\t"
            "shl  rsi, 5   \n\t"
            "shl  rdi, 5   \n\t"
            "shl  r8,  5   \n\t"
            "mov  r9,  rdx \n\t"
            "and  r9,  rsi \n\t"
            "cmp  r9,  rsi \n\t"
            "loopne 1b \n\t"

            "jnz    2f \n\t"
            "pext  r9, rdx, r8 \n\t"
            "pdep  r9, r9, rdi \n\t"
            "andn  rbx, rdi, rbx \n\t"
            "or    rbx, r9 \n\t"
            "2: \n\t"
            "xor  rax, rax \n\t"
            "cmp  rbx, rdx \n\t"
            "cmovnz  rax, rsi \n\t"
            :"=&a"(tileMap),"=&b"(rest):"d"(src),"S"(lowerMask),"D"(fullMask),"c"(upperMask):"r8", "r9","memory");
        return (tileMap != 0);
    }

    TileMap src_;
};

namespace {
    // ある牌の組み合わせについてを待ちを取得する
    inline std::string solvePattern(const std::string& patternStr, TileMap patternTileMap) {
        std::string resultStr = patternStr;
        resultStr += ":\n";

        Puzzle puzzle(patternTileMap);
        resultStr += puzzle.Find();
        return resultStr;
    }

    // patternStrを設定する場合は、patternStr[0]が非0
    TileMap enumerateOne(TileMap& tileMap, TileMap& nextNumber, TileMap& number,
                         bool enablePattern, std::string& patternStr) {
        TileMap invalid = 0;
        char patternCharSet[SizeOfCompleteTiles + 2] = {enablePattern, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0};

        asm volatile (
            "xor  rdi, rdi \n\t"
            "mov  rbx, rdx \n\t"
            "mov  rcx, 13 \n\t"
            "mov  r8, 9 \n\t"
            "mov  r9, 4 \n\t"
            "mov  r10, (15 << 4) \n\t"
            "mov  r11, 9 \n\t"
            "mov  r15, 1 \n\t"

            "1: \n\t"
            "shlx  r11, r8, r9 \n\t"
            "mov   r12, rdx \n\t"
            "and   r12, r10 \n\t"
            "cmp   r12, r11 \n\t"
            "jnz   2f \n\t"

            "xor   r14, r14 \n\t"
            "add   r9, 4 \n\t"
            "mov   r13, rcx \n\t"
            "and   r13, 3 \n\t"
            "cmp   r13, 2 \n\t"
            "cmovz r14, r15 \n\t"
            "sub   r8, r14 \n\t"
            "shl   r10, 4 \n\t"
            "loop  1b \n\t"
            "mov   rdi, 1 \n\t"

            "2: \n\t"
            "mov   r8, rcx \n\t"
            "mov   rcx, 14 \n\t"
            "sub   rcx, r8 \n\t"
            "xor   r13, r13 \n\t"
            "mov   r12, rdx \n\t"
            "shrx  r8, r12, r9 \n\t"
            "and   r8, 15 \n\t"
            "add   r8, 1 \n\t"

            "3: \n\t"
            "shlx  r11, r8, r9 \n\t"
            "andn  rbx, r10, rbx \n\t"
            "or    rbx, r11 \n\t"

            "xor   r14, r14 \n\t"
            "add   r13, 1 \n\t"
            "test  r13, 3 \n\t"
            "cmovz r14, r15 \n\t"
            "add   r8, r14 \n\t"
            "sub   r9, 4 \n\t"
            "shr   r10, 4 \n\t"
            "loop  3b \n\t"

            "xor   rax, rax \n\t"
            "mov   r8, rdx \n\t"
            "mov   r9, 15 \n\t"
            "mov   r10, 31 \n\t"
            "mov   rcx, 13 \n\t"

            "4: \n\t"
            "shr   r8, 4 \n\t"
            "mov   r11, r8 \n\t"
            "and   r11, r9 \n\t"
            "mov   r12, r11 \n\t"
            "shl   r11, 2 \n\t"
            "add   r11, r12 \n\t"

            "shlx  r12, r10, r11 \n\t"
            "pext  r13, rax, r12 \n\t"
            "shl   r13, 1 \n\t"
            "or    r13, 1 \n\t"
            "pdep  r13, r13, r12 \n\t"
            "or    rax, r13 \n\t"
            "loop  4b\n\t"

            "41: \n\t"
            "or    byte ptr [rsi], 0 \n\t"
            "jz    6f \n\t"
            "mov   rcx, 13 \n\t"
            "mov   r8,  rsi \n\t"
            "sub   r8,  1   \n\t"
            "mov   r9,  rdx \n\t"

            "5: \n\t"
            "shr   r9, 4   \n\t"
            "mov   r10, r9 \n\t"
            "and   r10, 15 \n\t"
            "add   r10, 0x30 \n\t"
            "mov   [r8 + rcx], r10b \n\t"
            "loop  5b\n\t"

            "6: \n\t"
            :"=&a"(tileMap),"=&b"(nextNumber),"=&D"(invalid):"d"(number),"S"(patternCharSet):"rcx","r8","r9","r10","r11","r12","r13","r14","r15","memory");

        patternStr = const_cast<char*>(patternCharSet);
        return invalid;
    }
}

namespace TileSetSolver {
    void EnumerateAll(SizeType indexOffset, SizeType stepSize, StrArray& result) {
        decltype(indexOffset) patternIndex = 0;
        TileMap number = 0x11112222333340;  // 辞書順で一番小さいパターン
        TileMap tileMap = 0;
        TileMap nextNumber = 0;
        TileMap invalid = 0;

        do {
            bool enablePattern = (patternIndex == indexOffset);
            std::string patternStr;
            invalid = enumerateOne(tileMap, nextNumber, number, enablePattern, patternStr);

            if (enablePattern) {
                result.push_back(solvePattern(patternStr, tileMap));
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
