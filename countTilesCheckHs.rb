#!/usr/bin/ruby
# coding: utf-8
#
# countTilesSlow.hs, countTilesShort.hs をテストする
# 実行結果がすべて期待値と一致すれば0、相違があれば1を返す

# 引数と期待値
TEST_CASE_SET = [["1112224588899", "(111)(222)(888)(99)[45]\n"].map(&:freeze),
                 ["1122335556799", "(123)(123)(55)(567)[99]\n(123)(123)(555)(99)[67]\n(123)(123)(567)(99)[55]\n"].map(&:freeze),
                 ["1112223335559", "(111)(222)(333)(555)[9]\n(123)(123)(123)(555)[9]\n"].map(&:freeze),
                 ["1223344888999", "(123)(234)(888)(999)[4]\n(123)(44)(888)(999)[23]\n(234)(234)(888)(999)[1]\n"].map(&:freeze),
                 ["1112345678999", "(11)(123)(456)(789)[99]\n(11)(123)(456)(999)[78]\n(11)(123)(678)(999)[45]\n(11)(345)(678)(999)[12]\n(111)(234)(567)(99)[89]\n(111)(234)(567)(999)[8]\n(111)(234)(678)(999)[5]\n(111)(234)(789)(99)[56]\n(111)(345)(678)(999)[2]\n(111)(456)(789)(99)[23]\n(123)(456)(789)(99)[11]\n"].map(&:freeze),
                 ["1113333555666", "(11)(333)(555)(666)[13]\n(111)(333)(55)(666)[35]\n"].map(&:freeze)
                ].map(&:freeze).freeze

# 実行ファイル名とログ出力先ファイル名
EXECUTABLE_SET = [["./countTilesSlow",  "logHsSlow.txt"].map(&:freeze),
                  ["./countTilesShort", "logHsShort.txt"].map(&:freeze),
                  ["./countTilesEx",    "logHsEx.txt"].map(&:freeze)
                 ].map(&:freeze).freeze

class TestCaseSet
  def initialize
  end

  # テストが期待値と一致すればtrue、相違があればfalseを返す
  def testAll
    exeSet = EXECUTABLE_SET
    total = true

    TEST_CASE_SET.each do |arg, expected|
      exec(exeSet, arg)

      exeSet.each do |exename, logname|
        actual = `cat #{logname}`
        str = "#{exename} actual:\n#{actual}"
        str += "expected:\n#{expected}"

        removeCr = -> str { str.gsub(/\r+/,"") }
        result = (removeCr.call(expected) == removeCr.call(actual))
        str += (result ? "Passed" : "Failed")
        str += "\n\n"

        total &= result
        puts str
      end
    end

    total
  end

  def exec(exeSet, arg)
    # 並列実行
    exeSet.map do |exename, logname|
      command = "bash -c " + '"' + "echo #{arg} | #{exename} #{arg} > #{logname}" + '"'
      Process.spawn(command)
    end.each { |pid| Process.waitpid(pid) }
  end
end

result = TestCaseSet.new.testAll
puts (result ? "All tests passed" : "Failed")
exit(result)
