/*
 * 出題元
 * http://www.itmedia.co.jp/enterprise/articles/1004/03/news002_2.html
 *
 * 清一色の全組み合わせについて、すべての待ちを1秒台で結果を出力する
 * 起動時の引数に-N2をつけると2スレッドで、-NをつけるとCPUの論理スレッド数の
 * スレッドを使って解く。
 */

#include <cstdint>
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <vector>
#include "countTilesBits.hpp"

#ifdef USE_BOOST_THREAD
// MinGWではstd::threadが使えないので、boost::threadを使う
#include <boost/thread/future.hpp>
#define THREAD_FUTURE boost::unique_future
#define THREAD_ASYNC  boost::async
#define THREAD_LAUNCH_ASYNC  boost::launch::async
#define THREAD_HARDWARE_CONCURRENCY  boost::thread::hardware_concurrency
#else
#include <future>
#define THREAD_FUTURE std::future
#define THREAD_ASYNC  std::async
#define THREAD_LAUNCH_ASYNC std::launch::async
#define THREAD_HARDWARE_CONCURRENCY  std::thread::hardware_concurrency
#endif

using namespace TileSetSolver;

namespace {
    void solveAllInSingleThread(std::ostream& os) {
        StrArray result;
        EnumerateAll(0, 1, result);
        for(auto str : result) {
            os << str;
        }
        return;
    }

    void solveAllWithThreads(SizeType sizeOfThreads, std::ostream& os) {
        std::vector<StrArray> resultSet;
        resultSet.resize(sizeOfThreads);

        // 並行して評価する関数群を準備する
        std::vector<THREAD_FUTURE<void>> futureSet;
        for(decltype(sizeOfThreads) index = 0; index < sizeOfThreads; ++index) {
            futureSet.push_back(
                THREAD_ASYNC(THREAD_LAUNCH_ASYNC,
                             [&resultSet, index, sizeOfThreads](void) -> void
                             { EnumerateAll(index, sizeOfThreads, resultSet.at(index)); }));
        }

        // 並行して評価して、結果がそろうのを待つ
        for(auto& f : futureSet) {
            f.get();
        }

        // 並行実行結果から、順番に結果を取得する
        bool cont = true;
        for(decltype(resultSet)::value_type::size_type i = 0; cont; ++i) {
            for(auto& result : resultSet) {
                if (result.size() <= i) {
                    cont = false;
                    break;
                }
                os << result.at(i);
            }
        }

        return;
    }

    void SolveAll(SizeType sizeOfThreads, std::ostream& os) {
        if (sizeOfThreads <= 1) {
            solveAllInSingleThread(os);
        } else {
            solveAllWithThreads(sizeOfThreads, os);
        }
        return;
    }

    // 実行するスレッド数を返す
    using SizeOfThreads = unsigned int;
    SizeOfThreads GetSizeOfThreads(const char* pArg) {
#ifdef __CYGWIN__
        // Cygwinでマルチスレッド実行すると却って遅くなる
        return 1;
#else
        std::string arg = pArg;
        std::string opt = "-N";

        if (arg.find(opt) != 0) {
            return 1;
        }

        if (arg.size() > opt.size()) {
            auto n = atoi(pArg + opt.size());
            if (n > 0) {
                return n;
            }
        }

        return THREAD_HARDWARE_CONCURRENCY();
#endif
    }
}

int main(int argc, char* argv[]) {
    SizeOfThreads sizeOfThreads = (argc > 1) ? GetSizeOfThreads(argv[1]) : 1;
    SolveAll(sizeOfThreads, std::cout);
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
