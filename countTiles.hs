-- 出題元
-- http://www.itmedia.co.jp/enterprise/articles/1004/03/news002_2.html

import Data.List
import Control.Applicative
import Control.Exception

-- 刻子ならその3牌を返す
tileTriple (a:b:c:xs) | (a == b) && (b == c) = Just [a,b,c]
                      | otherwise = Nothing

-- 順子ならその3牌を返す
tileSequence (a:b:c:xs) | ((a + 1) == b) && ((b + 1) == c) && (a >= 1) && (c <= 9) = Just [a,b,c]
                        | otherwise = Nothing

-- 刻子または順子があればそれぞれ3牌を返す
-- [10, 11]は、異種の牌が足りないときに、マッチングしないようにするダミー
threeTiles tiles = [tileTriple tiles, tileSequence $ sort $ (nub tiles) ++ [10, 11]]

-- leftからtsの3牌を取り除いてrightに移す
moveThreeTiles left right Nothing = [[]]
moveThreeTiles left right (Just ts) = concat $ splitTo4x3tiles (left \\ ts) (right ++ ts)

-- leftから3牌ずつ取り除いてrightに移す
-- これ以上取り除くものがなければバックトラッキングを終了して結果を返す
splitTo4x3tiles left right | ((length left) > 0) = map (moveThreeTiles left right) (threeTiles left)
                           | ((length left) == 0) = [[right]]

-- 14牌のtsの対子を[p,p]に固定して、対子以外の12牌について、
-- 左から右に移しながらバックトラッキングを行う
setPairAndSearch ts p | (length (ts \\ [p,p])) == ((length ts) - 2) =
                          map (\x -> ([p,p] ++ x)) (concat $ splitTo4x3tiles (ts \\ [p,p]) [])
                      | otherwise = [[]]

-- 同種の牌は4個以下(5個以上あればTrue, そうでなければFalse)
hasQuint [] = False
hasQuint (a:b:c:d:e:xs) | (a == b) && (b == c) && (c == d) && (d == e) = True
hasQuint (a:xs) = hasQuint xs

-- 手牌13牌(ts)に1牌(e)を追加し、対子を決め打ちして
-- バックトラッキングを行い、候補一覧を取得する
addOneAndSearch ts e | hasQuint (sort (ts ++ [e])) == False =
                         map (setPairAndSearch $ sort (ts ++ [e])) [1..9]
                     | otherwise = [[]]

-- 手牌13牌(ts)に1牌(e)を追加してバックトラッキングを行い、候補をすべて取得する
-- バックトラッキング途中で打ち切った場合は14牌ないので捨てる
searchFullSet ts e = filter (\ls -> (length ls) == 14) $ concat $ addOneAndSearch ts e

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
searchAll ts e = concat $ map (fullTilesToStringSet e) $ searchFullSet ts e

-- 手牌13牌(ts)の待ち形だけ取り出す
findAll ts = filter (\str -> (length str) == 23) $ nub $ concat $ map (searchAll ts) [1..9]

-- 手牌13牌(ts)の待ち形を求めてそれぞれ一行にする
solveAll ts = formatLines $ nub $ findAll $ sort $ take 13 $ (map (read . (:"")) ts :: [Int])
  where formatLines lines = intercalate "\n" $ map (filter (/= '"')) $ map show lines

-- テスト
solveTest =
  (solve "1112224588899" "(111)(222)(888)(99)[45]") &&
  (solve "1122335556799" "(123)(123)(567)(99)[55]\n(123)(123)(555)(99)[67]\n(123)(123)(55)(567)[99]") &&
  (solve "1112223335559" "(111)(222)(333)(555)[9]\n(123)(123)(123)(555)[9]") &&
  (solve "1223344888999" "(234)(234)(888)(999)[1]\n(123)(44)(888)(999)[23]\n(123)(234)(888)(999)[4]") &&
  (solve "1112345678999" "(123)(456)(789)(99)[11]\n(111)(456)(789)(99)[23]\n(111)(345)(678)(999)[2]\n(11)(345)(678)(999)[12]\n(11)(123)(678)(999)[45]\n(111)(234)(789)(99)[56]\n(111)(234)(678)(999)[5]\n(11)(123)(456)(999)[78]\n(111)(234)(567)(99)[89]\n(111)(234)(567)(999)[8]\n(11)(123)(456)(789)[99]") &&
  (solve "1113333555666" "(11)(333)(555)(666)[13]\n(111)(333)(55)(666)[35]")
  where solve ts expected = assert (expected == (solveAll ts)) True

test | (solveTest == True) = "All test passed"

-- テスト後にプロンプトを表示し、連続13文字の数字を受け付ける
main = do
  putStrLn (message ++ " >")
  l <- getLine
  putStrLn $ solveAll l
  where message = test
