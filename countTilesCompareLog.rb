#!/usr/bin/ruby
# coding: utf-8
#
# ログを比較する
# 手牌に対する待ちの複数行、行内の並びについては順不同とする

module Mode
  attr_accessor :parallelGemLoaded
  # mapを並列実行可能
  @@parallelGemLoaded = false
end

include Mode

# parallelがないなら並列実行しない
begin
  require 'parallel'
  Mode.parallelGemLoaded = true
rescue LoadError
  Mode.parallelGemLoaded = false
end

# 検査または入力形式の異常
class InternalCheckerError < StandardError
end

# 手牌ごとの結果
class TileSet
  def initialize(line)
    @input = getDigits(line)
    @header = line
    @lineSet = []
  end

  # 数字だけ取り出す
  def getDigits(line)
    line.scan(/\d/).sort.join
  end

  # 答えが異常
  def reportError(str)
    raise InternalCheckerError.new(@header + str)
  end

  # 行内の並びを統一する
  # (none)も検出する
  def add(line)
    digits = getDigits(line)
    reportError(line) unless digits.empty? || (@input == digits)

    sordedLine = line.scan(/\W\w+\W/).sort.join
    reportError(line) unless sordedLine.size == line.size

    unless digits.empty?
      openSet = line.scan(/\[\d+\]/)
      reportError(line) unless openSet.size == 1
      numStrSet = openSet[0].scan(/\d/)
      numSet = numStrSet.map(&:to_i).sort

      if numSet.size == 1
        # 5牌目は待てない
        reportError(line) if @input.include?(numStrSet[0] * 4)
      elsif numSet.size == 2
        reportError(line) unless (numSet[0] == numSet[1]) || ((numSet[0] + 1) == numSet[1]) || ((numSet[0] + 2) == numSet[1])

        key = numSet[0]
        extraTiles = [key] if (key == numSet[1])
        extraTiles = [key - 1, key + 2].select { |tile| (tile >= 1) && (tile <= 9) } if ((key + 1) == numSet[1])
        extraTiles = [key + 1] if ((key + 2) == numSet[1])

        # 5牌目は待てない
        reportError(line) if extraTiles.all? { |tile| @input.include?(tile.to_s * 4) }
      else
        reportError(line)
      end
    end

    @lineSet << sordedLine
  end

  # 比較可能な文字列を返す
  def toString
    reportError(@lineSet.join("\n")) if @lineSet.empty?
    lineSet = @lineSet.sort
    uniqLineSet = lineSet.uniq
    reportError(@lineSet.join("\n")) unless @lineSet.size == uniqLineSet.size
    @header + "\n" + lineSet.join("\n")
  end
end

# ログファイル
class LogFile
  attr_reader :filename
  def initialize(filename)
    @filename = filename
    @fullSet = []
    # 後で捨てるためのダミー
    @tileSet = TileSet.new("")

    File.open(filename, "r") { |file|
      puts "Reading #{filename}\n"
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
    if Mode.parallelGemLoaded
      @logFileSet = Parallel.map(LOG_FILENAME_SET) { |filename| LogFile.new(filename) }
    else
      @logFileSet = LOG_FILENAME_SET.map { |filename| LogFile.new(filename) }
    end
  end

  # 答えが異常
  def reportError(str)
    raise InternalCheckerError.new(str)
  end

  # ログファイル群を比較する
  def compare
    logData = Struct.new(:filename, :strSet)
    logSet = @logFileSet.map { |log| logData.new(log.filename, log.toStringSet) }
    baseLog = logSet.shift
    reportError(baseLog.filename) unless baseLog.strSet.size == LOG_BLOCK_SIZE

    logSet.all? do |otherLog|
      puts "Comparing #{baseLog.filename} and #{otherLog.filename}\n"
      reportError(otherLog.filename) unless otherLog.strSet.size == LOG_BLOCK_SIZE
      logPair = [baseLog.strSet, otherLog.strSet].transpose
      compareLogPair(logPair)
    end
  end

  # ログファイル2つを比較する
  def compareLogPair(logPair)
    logPair.all? do |base, other|
      result = (base == other)
      reportError("Different\n#{base}\n  and\n#{other}\n\n") unless result
      result
    end
  end
end

puts "Parallel map enabled = #{Mode.parallelGemLoaded}\n"
result = LogFileSet.new.compare
puts (result ? "Passed" : "Failed")
exit(result)
