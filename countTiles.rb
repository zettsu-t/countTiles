#!/usr/bin/ruby
# coding: utf-8
#
# 出題元
# http://www.itmedia.co.jp/enterprise/articles/1004/03/news002_2.html
#
# 起動方法
#   ruby countTiles.rb (このスクリプト名)
# 起動すると、標準入力から手牌(1-9の数字13桁)が入力されるのを待つので、
# 入力してenterを押すと、手牌の待ちを出力する
#
# 引数を指定すると、テストを実行し、入力を待たずに終了する。
# 引数に1を指定すると、清一色の全組み合わせについてテストする
# 引数に2以上を指定すると、引数で指定した回数のテストを実行する

SIZE_OF_COMPLETE_TILES = 14.freeze  # あがるときの牌の数
MIN_TILE_NUMBER = 1.freeze  # 最も数字が小さい牌
MAX_TILE_NUMBER = 9.freeze  # 最も数字が大きい牌
SIZE_OF_A_TILE =  4.freeze  # 同種の牌の数

# テスト回数が0回なら入力を待つ
# テスト回数が1回なら、清一色の全組み合わせについてテストする
ARG_COUNT_DEFAULT = 0.freeze
ARG_COUNT_ENUMERATE_TILES = 1.freeze

# 不正な入力値
class InvalidInputError < StandardError
end

# 同種の牌が多すぎる
class TooManyTilesForKind
  attr_reader :value
  def initialize(tiles)
    @value = tiles.map(&:to_s).join.match(/(\d)\1{#{SIZE_OF_A_TILE},}/)
  end
end

class TileSet
  def initialize(line)
    # lineとして与えられる一行から手牌13種を入力する
    # それぞれの牌は1-9で、これらからなる連続13文字とする。
    # 行頭の空白と、13文字の後の空白および空白以後の余分な文字は無視する。
    # 処理しやすいよう、1から9(昇順)にソートする
    tiles = line.chomp.strip.match(/^(\S+)/)[1].split("").map{ |c| c.to_i }
    @tiles = tiles.sort.reject { |i| (i < MIN_TILE_NUMBER) || (i > MAX_TILE_NUMBER) }
    raise InvalidInputError if @tiles.size != (SIZE_OF_COMPLETE_TILES - 1)
  end

  # すべての組み合わせを探して表示する
  def searchAll
    solutions = []

    # 待ち牌を決め打ちして、14牌があがり型を成しているかどうか調べる
    MIN_TILE_NUMBER.upto(MAX_TILE_NUMBER) do |extraTile|
      # 14牌にする
      tiles = @tiles.dup.concat([extraTile]).sort

      # 同種の牌が多すぎる
      next if TooManyTilesForKind.new(tiles).value

      # 解を探して待ち牌に印をつける
      solutions.concat(markExtraTile(search(tiles, false), extraTile.to_s))
    end

    # 解を表示する。同じものは一回だけ表示する。
    # 解がなければ、C++, Haskell版にあわせて(none)にする
    str = solutions.sort.uniq.join("\n")
    (str.empty? ? "(none)" : str) + "\n"
  end

  # 待ち牌がある対子,刻子,順子を特定して、()[]表記を作る
  # cadidatesは解の組み合わせ(配列)
  # extraTileは決め打ちした待ち牌(文字列)
  def markExtraTile(cadidates, extraTile)
    solutions = []

    # map-compactでも書けるが、候補でないものを早めに除外したいのでeachにする
    cadidates.each do |cadidate|
      # 14牌そろっていない = 手でないものは除く
      next if cadidate.gsub(/\D+/,"").size < SIZE_OF_COMPLETE_TILES

      # 解は対子,刻子,順子を(..)(...)形式で表現した文字列なので、()で分割する
      parts = cadidate.scan(/(\(\d+\))/).map(&:first)
      parts.each_with_index do |part, partIndex|
        # この対子,刻子,順子には、待ち牌が含まれていない
        next unless part.index(extraTile)

        # 要素は参照なので、他の候補と参照を共有しているが、要素を変更しないから問題ない
        # 決め打ちした待ち牌がこの要素で重複しても、または他の要素で使われていても問題ない
        newParts = parts.dup
        # 待ち牌と同種の牌を一つだけ除くので、gsubではなくsub
        newParts[partIndex] = parts[partIndex].sub(/#{extraTile}/,"").tr("()","[]")

        # 重複を取り除けるようソートしておく
        solutions << newParts.sort.join("")
      end
    end

    solutions
  end

  # argTilesの牌列 = 14牌の全部または一部から、対子、刻子、順子を探す
  # 対子がすでに見つかっているのでこれ以上探さないときは、foundPair = true
  def search(argTiles, foundPair)
    solutions = []

    # 対子、刻子、順子を再帰的に探す。七対子は除外する。
    # partTypeは、関数名の"search"以降
    searchPart = -> (partType, conditionToThis, subFoundPair) {
      # 探すかどうかは引数conditionToThisで与える
      if conditionToThis
        solution, remainingTiles = send(("search" + partType).to_sym, argTiles)
        if (solution)
          # 見つかった対子,刻子,順子を取り除いた残りで組み合わせを探す
          subSolutions = search(remainingTiles, subFoundPair)
          newSolutions = subSolutions.empty? ? [solution] : subSolutions.map { |sub| solution + sub }
          solutions.concat(newSolutions)
        end
      end
    }

    searchPart.call("Pair", !foundPair, true)
    searchPart.call("Triple", true, foundPair)
    searchPart.call("SequenceOfThree", true, foundPair)

    # 再帰の最上位であがり型を成しているなら14牌ある
    # 再帰の最上位で13牌以下なら、あがり型を成さないので探索を打ち切った
    solutions
  end

  # 対子を探す
  def searchPair(tiles)
    return searchFlush(tiles, 2)
  end

  # 刻子を探す
  def searchTriple(tiles)
    return searchFlush(tiles, 3)
  end

  # 同種の牌の組み合わせを探す
  def searchFlush(tiles, size)
    solution = nil
    remainingTiles = tiles.dup

    if (tiles.size >= size) && (1..(size - 1)).all? { |i| tiles[0] == tiles[i] }
      solution = formatPart(Array.new(size, tiles[0]))
      remainingTiles.shift(size)
    end

    return solution, remainingTiles
  end

  # 順子を探す
  def searchSequenceOfThree(tiles)
    searchTileSequence(tiles, 3)
  end

  # 連番を探す
  def searchTileSequence(tiles, size)
    solution = nil
    remainingTiles = tiles.dup

    if (tiles.size >= size)
      # 同種の牌が複数ありうる : 123ではなく111223から23と位置を探す
      base = tiles[0]
      positions = (0..(size - 1)).map{ |i| tiles.index(base + i) }.compact

      # N ... N+size-1 番の牌が見つかった
      if (positions.size == size)
        solution = formatPart((base..(base + size - 1)).to_a)
        # ソート済なので、先頭から順に削除することが保証されている
        positions.each_with_index { |position, index| remainingTiles.delete_at(position - index) }
      end
    end

    return solution, remainingTiles
  end

  # 対子、刻子、順子を文字列表記にする
  def formatPart(tiles)
    "(" + tiles.map{ |tile| tile.to_s }.join + ")"
  end
end

# テスト失敗
class TestingFailed < StandardError
end

# テスト
class TileSetTestCases
  def initialize(testSize)
    @testSize = testSize
    @random = Random.new
  end

  # 例題を解く
  def check
    # 同じ牌が5つ以上あったら検出する
    1.upto(SIZE_OF_A_TILE) do |n|
      middleStr = "5" * n
      str = "1234" + middleStr + "6789"
      raise TestingFailed if str.match(/(\d)\1{#{SIZE_OF_A_TILE},}/)
    end

    (SIZE_OF_A_TILE+1).upto(SIZE_OF_A_TILE+3) do |n|
      middleStr = "5" * n
      str = "1234" + middleStr + "6789"
      raise TestingFailed unless str.match(/(\d)\1{#{SIZE_OF_A_TILE},}/)
    end

    [["1112224588899", ["(111)(222)(888)(99)[45]"]],
     ["1122335556799", ["(123)(123)(55)(567)[99]", "(123)(123)(555)(99)[67]", "(123)(123)(567)(99)[55]"]],
     ["1112223335559", ["(111)(222)(333)(555)[9]", "(123)(123)(123)(555)[9]"]],
     ["1223344888999", ["(123)(234)(888)(999)[4]", "(123)(44)(888)(999)[23]", "(234)(234)(888)(999)[1]"]],
     ["1112345678999", ["(11)(123)(456)(789)[99]", "(11)(123)(456)(999)[78]", "(11)(123)(678)(999)[45]",
                        "(11)(345)(678)(999)[12]", "(111)(234)(567)(99)[89]", "(111)(234)(567)(999)[8]",
                        "(111)(234)(678)(999)[5]", "(111)(234)(789)(99)[56]", "(111)(345)(678)(999)[2]",
                        "(111)(456)(789)(99)[23]", "(123)(456)(789)(99)[11]"]],
     ["1113333555666", ["(11)(333)(555)(666)[13]", "(111)(333)(55)(666)[35]"]]].each do |line, solutions|
      actual = TileSet.new(line).searchAll
      expected = solutions.sort.uniq.join("\n") + "\n"
      raise TestingFailed if actual != expected
    end

    # 不正な入力には例外を投げる
    ["111222458889", "111222458889x", "0001112223334"].each do |line|
      caught = false
      begin
        TileSet.new(line).searchAll
      rescue InvalidInputError
        caught = true
      end
      raise TestingFailed unless caught
    end
  end

  # あがり型かどうか無視して13牌集めたものを解く
  def checkRandomPattern
    1.upto(@testSize) do |i|
      pattern = getRandomPattern
      actual = TileSet.new(pattern.join("")).searchAll
      checkTiles(pattern, actual, nil)
    end
  end

  # あがり型の14牌を解く
  def checkCompletePattern
    1.upto(@testSize) do |i|
      basePattern = getCompletePattern
      # 同種の牌が多すぎる
      next if TooManyTilesForKind.new(basePattern).value

      # 1牌抜いて、それが待ち牌であることを確認する
      0.upto(basePattern.size - 1) do |position|
        pattern = basePattern.dup
        extraTile = pattern[position]
        pattern.delete_at(position)

        actual = TileSet.new(pattern.join("")).searchAll
        checkTiles(pattern, actual, extraTile)
      end
    end
  end

  # 待ち方が成立しているかどうか調べる
  # actual : TileSetの出力結果
  # extraTile : 待ち牌 = あがりから抜いた牌(なければnil)
  def checkTiles(pattern, actual, extraTile)
    # 待ち牌を見つけた
    foundTile = false

    # [..]が待ち
    actual.each_line do |line|
      line.scan(/\[(\d+)\]/).each do |strArray|
        raise TestingFailed unless strArray.size == 1
        tiles = strArray[0].split("").map(&:to_i)
        raise TestingFailed if (tiles.size < 1) || (tiles.size > 2)

        # 単騎待ちに見えても延べ単のことがあるので、[]の牌 = 抜いた牌とは限らない
        # つまり (123)[4] or (234)[1]
        foundTile = true if extraTile && (tiles.size == 1) && (tiles[0] == extraTile)

        # 刻子または順子になることを確認する
        # 待ちが[]の牌以外 = 抜いた牌とは限らない
        # 例えば、(111)(333)(444)(66)[68] or (111)(333)(444)(666)[8] は
        # 8を抜いたのに7でもあがれる
        next unless tiles.size == 2
        base = tiles[0]
        raise TestingFailed unless [base, base + 1, base + 2].any? { |tile| tile == tiles[1] }

        next unless extraTile
        if (tiles[0] == tiles[1])
          # 刻子になる。シャボ待ちの場合は別の[]で見つかることがある。
          foundTile = true if (tiles[0] == extraTile)
        else
          tileSorted = [tiles[0], tiles[1], extraTile].sort.uniq
          if tileSorted.size == 3
            # 順子になる
            foundTile = true if ((tileSorted[0] + 1) == tileSorted[1]) && ((tileSorted[1] + 1) == tileSorted[2])
          end
          # 前記の嵌張待ちのように tileSorted.size == 2 {668} になることもあるが無視する
        end
      end
    end

    # 抜いた牌はいずれかの形で待ち牌になっている
    # 同種の牌が多すぎると待てないが、そのようなテストは入力しない
    if extraTile && !foundTile
      str = pattern.join + "\nactual:\n#{actual}\nextraTile:#{extraTile}\n"
      raise TestingFailed.new(str)
    end
    0
  end

  # あがり型かどうか無視して13牌集める
  def getRandomPattern
    candidates = (MIN_TILE_NUMBER..MAX_TILE_NUMBER).map{ |i| Array.new(SIZE_OF_A_TILE, i.to_s) }.flatten

    tiles = []
    1.upto(SIZE_OF_COMPLETE_TILES - 1) do |i|
      position = @random.rand(candidates.size)
      tiles << candidates[position]
      # 同じものは二度選ばない
      candidates.delete_at(position)
    end

    tiles.sort
  end

  # あがり型の14牌集める
  def getCompletePattern
    candidates = {}
    (MIN_TILE_NUMBER..MAX_TILE_NUMBER).each { |i| candidates[i] = SIZE_OF_A_TILE }
    tiles = []

    # 刻子ができすぎないように、刻子の出現確率をランダムに変える
    thresholdTable = [0, 0, 0, 1, 1, 2, 3]
    levelOfTriple = thresholdTable.max * 2 + 2
    thresholdOfTriple = thresholdTable.at(@random.rand(thresholdTable.size))

    while(tiles.size < (SIZE_OF_COMPLETE_TILES - 2))
      size = 3
      # randは0が先頭
      n = MIN_TILE_NUMBER + @random.rand(candidates.size)
      if @random.rand(levelOfTriple) <= thresholdOfTriple
        # 刻子を作る
        if (candidates[n] >= size)
          size.times { tiles << n }
        end
        candidates[n] -= size
      else
        # 順子を作る
        if (n <= (MAX_TILE_NUMBER - size + 1)) && (0..(size - 1)).all? { |i| candidates[n + i] > 0 }
          0.upto(size - 1) do |i|
            tiles << n + i
            candidates[n + 1] -= 1
          end
        end
      end
    end

    # 対子を作る
    while(tiles.size < SIZE_OF_COMPLETE_TILES)
      # randは0が先頭
      n = MIN_TILE_NUMBER + @random.rand(candidates.size)
      size = 2
      if (candidates[n] >= size)
        size.times { tiles << n }
        candidates[n] -= size
      end
    end

    tiles.sort
  end

  # 清一色の全組み合わせについてテストする
  def checkAllPattern
    enumerateTiles([], 1, SIZE_OF_COMPLETE_TILES - 1)
  end

  def checkPattern(pattern)
    actual = TileSet.new(pattern.join).searchAll
    checkTiles(pattern, actual, nil)
    puts pattern.join + ":\n" + actual
  end

  def enumerateTiles(pattern, head, remaining)
    if (remaining == 0)
      checkPattern(pattern)
      return
    end

    return if head > MAX_TILE_NUMBER

    # 小さい番号の牌からたくさん集める(0個を含む)
    [remaining, SIZE_OF_A_TILE].min.downto(0) do |i|
      newPattern = pattern.dup
      newPattern.concat([head] * i)
      enumerateTiles(newPattern, head + 1, remaining - i)
    end
  end
end

# テストのサイズを引数で指定する
testSize = ARG_COUNT_DEFAULT
if ARGV.size > 0
  testSize = ARGV[0].to_i
end

testcase = TileSetTestCases.new(testSize)
testcase.check

if testSize == ARG_COUNT_ENUMERATE_TILES
  testcase.checkAllPattern
elsif testSize > ARG_COUNT_ENUMERATE_TILES
  testcase.checkRandomPattern
  testcase.checkCompletePattern
end

puts "All tests passed"
if testSize <= ARG_COUNT_DEFAULT
  puts TileSet.new(STDIN.gets).searchAll
end
0
