-- 出題元
-- http://www.itmedia.co.jp/enterprise/articles/1004/03/news002_2.html
-- 引数を何かつけると、清一色の全組み合わせについてテストする
-- 引数がないときはテスト後にプロンプトを表示し、連続13文字の数字を受け付ける

import Data.List
import Control.Applicative
import Control.Exception
import System.IO
import System.Environment (getArgs)

-- 刻子、順子、対子が何組あるか
sizeOfParts = 5
-- 手牌の数
sizeOfInput = sizeOfParts * 3 - 2
-- 結果の文字列長
sizeOfStr = sizeOfParts * 5 - 2

-- 刻子ならその3牌を返す
tileTriple (a:b:c:xs) | (a == b) && (b == c) && (a >= 1) && (c <= 9) = Just [a,b,c]
tileTriple otherwise = Nothing

-- 順子ならその3牌を返す
tileSequence (a:b:c:xs) | ((a + 1) == b) && ((b + 1) == c) && (a >= 1) && (c <= 9) = Just [a,b,c]
tileSequence otherwise = Nothing

-- 刻子または順子があればそれぞれ3牌を返す
-- [10, 11]は、異種の牌が足りないときに、マッチングしないようにするダミー
threeTiles tiles = map ($ tiles) [tileTriple, f]
  where f ts = tileSequence $ sort $ (nub ts) ++ [10, 11]

-- leftからtsの3牌を取り除いてrightに移す
moveThreeTiles _ _ Nothing = [[]]
moveThreeTiles left right (Just ts) = splitTo4x3tiles (left \\ ts) (right ++ ts)

-- leftから3牌ずつ取り除いてrightに移す
-- これ以上取り除くものがなければバックトラッキングを終了して結果を返す
splitTo4x3tiles [] right = [right]
splitTo4x3tiles left right = concatMap (moveThreeTiles left right) (threeTiles left)

-- 14牌のtsの対子を[p,p]に固定して、対子以外の12牌について、
-- 左から右に移しながらバックトラッキングを行う
setPairAndSearch ts p | (length (ts \\ [p,p])) == ((length ts) - 2) =
                          map (\x -> ([p,p] ++ x)) (splitTo4x3tiles (ts \\ [p,p]) [])
setPairAndSearch _ _ = [[]]

-- 同種の牌は4個以下(5個以上あればTrue, そうでなければFalse)
hasQuint [] = False
hasQuint (a:b:c:d:e:xs) | (a == b) && (b == c) && (c == d) && (d == e) = True
hasQuint (a:xs) = hasQuint xs

-- 手牌13牌(ts)に1牌(e)を追加し、対子を決め打ちして
-- バックトラッキングを行い、候補一覧を取得する
addOneAndSearch ts e | hasQuint (sort (ts ++ [e])) == False = concatMap (setPairAndSearch $ sort (ts ++ [e])) [1..9]
                     | otherwise = [[]]

-- 手牌13牌(ts)に1牌(e)を追加してバックトラッキングを行い、候補をすべて取得する
-- バックトラッキング途中で打ち切った場合は14牌ないので捨てる
searchFullSet ts e = filter (\ls -> (length ls) == (sizeOfInput + 1)) $ addOneAndSearch ts e

-- 牌の並びtsを文字列にする
tilesToString ts = foldr (++) "" $ map show ts
-- 待ち形の並びtsを文字列にする
openTilesToString ts = "[" ++ (tilesToString ts) ++ "]"
-- 完成形の並びtsを文字列にする
closeTilesToString ts = "(" ++ (tilesToString ts) ++ ")"

-- 待ちeの時の牌の並び(a:...)を文字列にする
-- eがを含むときは待ち型、含まないときは完成形として扱う
keyTilesToString e (a:b:c:xs) | (e == a) && (e == b) && (e == c) = openTilesToString [b,c]
keyTilesToString e (a:b:c:xs) | e == a = openTilesToString [b,c]
keyTilesToString e (a:b:c:xs) | e == b = openTilesToString [a,c]
keyTilesToString e (a:b:c:xs) | e == c = openTilesToString [a,b]
keyTilesToString e (a:b:c:xs) | otherwise = closeTilesToString [a,b,c]
keyTilesToString e (a:b:xs) | e == a = openTilesToString [a]
keyTilesToString e (a:b:xs) | e /= a = closeTilesToString [a,b]

-- 14牌を2 + 4*3牌に区切ったものをそれぞれ文字列にする(iはあと何区間あるか)
-- 各区間に、待ち牌があるかないかを適用した結果の、文字列群を返す
fullTilesToString fs ts 0 = []
fullTilesToString fs ts i = [g fs] ++ (fullTilesToString (rotate fs) ts (i - 1))
  where g = \hs -> foldr (++) "" (sort (zipWith ($) hs ts))
        rotate ls = take (length ls) (drop ((length ls) - 1) (cycle ls))

-- n*3牌に区切る
chunkOf3tiles ts | (length ts) <= 3 = [ts]
                 | otherwise = [take 3 ts] ++ (chunkOf3tiles (drop 3 ts))

-- 1牌(e)を追加して14牌(ts)にしたときの結果を文字列群にする
-- 待ちと完成形が混じっているので後で取り除く
fullTilesToStringSet e ts = fullTilesToString fs tss (length fs)
  where tss = [take 2 ts] ++ (chunkOf3tiles $ drop 2 ts)
        fc = closeTilesToString
        fs = [keyTilesToString e, fc, fc, fc, fc]

-- 手牌13牌(ts)に1牌(e)を追加してバックトラッキングを行い、結果を文字列群として取得する
-- 待ちと完成形が混じっているので後で取り除く
searchAll ts e = concatMap (fullTilesToStringSet e) $ searchFullSet ts e

-- 手牌13牌(ts)の待ち形だけ取り出す
findAll ts = filter (\str -> (length str) == sizeOfStr) $ nub $ concatMap (searchAll ts) [1..9]

-- 待ちがなければ(none)にする
formatResult s | ((length s) < 1) = "(none)\n"
               | otherwise = s

-- 手牌13牌(ts)のリストから待ち形を求めてそれぞれ一行にする
solveTiles ts = formatLines $ nub $ findAll $ sort $ ts
  where formatLines lines = formatResult $ concatMap (++ "\n") $ map (filter (/= '"')) $ map show lines

-- 手牌13牌(ts)の文字列から待ち形を求めてそれぞれ一行にする
-- 数字以外の入力はエラーになる。0があると待ち無しになる。
solveTileStr ts = solveTiles $ take sizeOfInput $ (map (read . (:"")) $ take sizeOfInput (ts ++ dummy) :: [Int])
  where dummy = concat $ replicate sizeOfInput "a"

-- テストケースを解く
solveTest =
  (f "1112224588899" "(111)(222)(888)(99)[45]\n") &&
  (f "1122335556799" "(123)(123)(567)(99)[55]\n(123)(123)(555)(99)[67]\n(123)(123)(55)(567)[99]\n") &&
  (f "1112223335559" "(111)(222)(333)(555)[9]\n(123)(123)(123)(555)[9]\n") &&
  (f "1223344888999" "(234)(234)(888)(999)[1]\n(123)(44)(888)(999)[23]\n(123)(234)(888)(999)[4]\n") &&
  (f "1112345678999" "(123)(456)(789)(99)[11]\n(111)(456)(789)(99)[23]\n(111)(345)(678)(999)[2]\n(11)(345)(678)(999)[12]\n(11)(123)(678)(999)[45]\n(111)(234)(789)(99)[56]\n(111)(234)(678)(999)[5]\n(11)(123)(456)(999)[78]\n(111)(234)(567)(99)[89]\n(111)(234)(567)(999)[8]\n(11)(123)(456)(789)[99]\n") &&
  (f "1113333555666" "(11)(333)(555)(666)[13]\n(111)(333)(55)(666)[35]\n") &&
  (f "0001112223334" "(none)\n")
  where f ts expected = assert (expected == (solveTileStr ts)) (expected == (solveTileStr ts))

test | (solveTest == True) = "All test passed"

-- 清一色の全組み合わせについてテストする
enumTiles p _ 0 = [p]
enumTiles p _ r | r < 0 = []
enumTiles p 10 r = []
enumTiles p i r = concatMap f $ (reverse [0..4])
  where f n = enumTiles (p ++ (replicate n i)) (i + 1) (r - n)

testAll :: IO()
testAll = putStr $ concatMap f $ enumTiles [] 1 13
  where f ts = (g ts) ++ (solveTiles ts)
        g ts = (foldr (++) "" $ map show ts) ++ ":\n"

-- 入力した文字列について解く
solveAll :: IO()
solveAll = do
  putStr (message ++ " >")
  hFlush stdout
  l <- getLine
  putStr $ solveTileStr l
  where message = test

-- テスト後にプロンプトを表示し、連続13文字の数字を受け付ける
paramSet [] = False
paramSet (x:xs) = True

main = do
  args <- getArgs
  let arg = paramSet args
  if arg then testAll else solveAll
