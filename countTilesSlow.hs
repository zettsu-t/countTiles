-- 出題元
-- http://www.itmedia.co.jp/enterprise/articles/1004/03/news002_2.html
-- 手牌 + 待ち牌を、すべての刻子、順子、対子の組み合わせに対して
-- 適用するので、プログラムは短いが、実行に数分掛かる。
-- 可読性を無視して、コードを1Kbyte未満に短くしたものが countTilesShort.hs

import Data.List
import System.Environment (getArgs)

-- 刻子、順子、対子が何組あるか
sizeOfParts = 5
-- 手牌の数
sizeOfInput = sizeOfParts * 3 - 2
-- 結果の文字列長
sizeOfStr = sizeOfParts * 5 - 2

-- すべての刻子、順子、対子
parts = (map (\i->[i..(i+2)]) [1..7]) ++ (concat $ map (\i->[[i, i ,i], [i, i]]) [1..9])

-- 刻子、順子、対子の全組み合わせ
tiles 0 = [[]]
tiles n = [i:rest | i <- parts, rest <- (tiles (n-1))]

-- 牌の並びを文字列にする
foldToStr = foldr (++) ""
toStr ts = foldToStr $ map show ts
-- 待ち形の文字列。[[は待ち牌でない並びを後で除去するためのダミー。
openStr ts e | ((length ts) > (length $ ts \\ [e])) = "[" ++ (toStr (ts \\ [e])) ++ "]"
             | otherwise = "[["
closeStr ts = "(" ++ (toStr ts) ++ ")"

-- 待ちがある/ない形の全組み合わせの文字列
toStrSet _ [] = [[]]
toStrSet e (t:ts) = [i:rest | i <- [openStr t e, closeStr t], rest <- (toStrSet e ts)]

-- 同種の牌は4個以下(5個以上あればTrue, そうでなければFalse)
hasQuint [] = False
hasQuint (a:b:c:d:e:xs) | (a == b) && (b == c) && (c == d) && (d == e) = True
hasQuint (a:xs) = hasQuint xs

-- 全文字列のうち、手配 + 待ち牌となりうるもの
allTileSet ts e = concat $ map (toStrSet e) $ filter f $ tiles sizeOfParts
  where f l = ((sort $ ts ++ [e]) == (sort $ concat l)) && (hasQuint (sort (concat l)) == False)

-- 全文字列のうち、手配 + 待ち牌となりうるもので、待ち形が成立するもの
solveAll ts = formatLines $ filter f $ map foldToStr $ nub $ sort $ map sort $ concat $ map (allTileSet ts) [1..9]
  where f s = (((length s) == sizeOfStr) && ((length ((s \\ "[") \\ "[")) == (sizeOfStr - 1)))
        formatLines lines = intercalate "\n" $ map (filter (/= '"')) $ map show lines

-- 引数として連続13文字の数字を受け付ける
-- countTilesShort.hsは、起動後に連続13文字の数字を受け付ける
paramSet (n:xs) = n
main = do
  args <- getArgs
  let (l) = paramSet args
  putStrLn $ show $ solveAll $ take sizeOfInput $ (map (read . (:"")) l :: [Int])
