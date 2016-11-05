#!/usr/bin/ruby
# coding: utf-8
#
# ログを比較する
# 手牌に対する待ちの複数行、行内の並びについては順不同とする

# 検査または入力形式の異常
class InternalCheckerError < StandardError
end

# 手牌ごとの結果
class TileSet
  def initialize(line)
    @header = line + "\n"
    @lineSet = []
  end

  # 行内の並びを統一する
  # (none)も検出する
  def add(line)
    sordedLine = line.scan(/\W\w+\W/).sort.join
    raise InternalCheckerError unless sordedLine.size == line.size
    @lineSet << sordedLine
  end

  # 比較可能な文字列を返す
  def toString
    raise InternalCheckerError if @lineSet.empty?
    @header + @lineSet.sort.join("\n")
  end
end

# ログファイル
class LogFile
  def initialize(filename)
    @tileSet = TileSet.new("")
    @fullSet = []

    File.open(filename, "r") { |file|
      while line = file.gets
        parseLine(line.chomp)
      end
    }
  end

  # 各行を読んでブロックに分ける
  def parseLine(line)
    if (!line.empty?) && (line[-1] == ":")
      @tileSet = TileSet.new(line)
      @fullSet << @tileSet
    else
      @tileSet.add(line)
    end
  end

  # 手牌ごとの結果を返す
  def toStringSet
    @fullSet.map(&:toString)
  end
end

# ログファイル名一覧
LOG_FILENAME_SET = ["logCpp.txt", "logRuby.txt", "logHs.txt"].map(&:freeze).freeze
# 手牌の種類
LOG_BLOCK_SIZE = 93600.freeze

# ログファイル群を比較する
# ファイルサイズと手牌の種類が一致することは、Makefileで確認済
class LogFileSet
  # ログファイル群を読み込んで解析する
  def initialize
    @logFileSet = LOG_FILENAME_SET.map { |filename| LogFile.new(filename) }
  end

  # ログファイル群を比較する
  def compare
    logSet = @logFileSet.map(&:toStringSet)
    baseLog = logSet.shift
    raise InternalCheckerError if logSet.empty?

    logSet.all? do |otherLog|
      logPair = [baseLog, otherLog].transpose
      (logPair.size != LOG_BLOCK_SIZE) ? false : compareLogPair(logPair)
    end
  end

  # ログファイル2つを比較する
  def compareLogPair(logPair)
    logPair.all? do |base, other|
      result = (base == other)
      warn "Different\n#{base}\n  and\n#{other}\n\n" unless result
      result
    end
  end
end

result = LogFileSet.new.compare
puts (result ? "Passed" : "Failed")
exit(result)
