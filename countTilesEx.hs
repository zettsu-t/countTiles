-- 出題元
-- http://www.itmedia.co.jp/enterprise/articles/1004/03/news002_2.html
-- 下記関数名、変数名の元ネタについて、公式サイトはこちらです
-- http://imas-cinderella.com/

import Data.List

who_is_in_the_pumpkin_carriage = 1
i_never_seen_such_a_beautiful_castle = 2
i_don't_want_to_become_a_wallflower = 5
i_wonder_where_I_find_the_light_I_shine = 7
sweet_is_a_magical_word_to_make_you_happy = 9
the_best_place_to_see_the_stars = 22
glass_slippers = 23

c  s = [s..s+2]
i  t = [[t,t,t],[t,t]]
n  a = not(w(sort$concat a))
d  r = "("++(ec r)++")"
e  ! 0 = [who_is_in_the_pumpkin_carriage..sweet_is_a_magical_word_to_make_you_happy]
re ! 0 = (map c[1..i_wonder_where_I_find_the_light_I_shine])++(concatMap i(e 0))
l    0 = [[]]
l    decoration = [candy:island|candy<-(re 0),island<-(l(decoration - who_is_in_the_pumpkin_carriage))]
a    = foldr(++)""

p  s = concatMap(++"\n")$map(filter(/='"'))$r$filter j$map a$nub$sort$map sort$concatMap(t 0 s)(e 0)
r  h = map show h
o  i   yubaePresent = ("["++(ec(i\\yubaePresent))++"]")
j  n = (((length n)==glass_slippers)&&((length((n\\"[")\\"["))==the_best_place_to_see_the_stars))
ec e = a$r e
t  ! 0 new generations = concatMap(v[generations])
                           $filter(\i->((sort$new++[generations])==(sort$concat i))&&n i)
                           $l i_don't_want_to_become_a_wallflower

w []=False
w (g:o:i:n') | (length n') <= who_is_in_the_pumpkin_carriage = False
w (m:a:g:i:c:_) | (m==a) && (a==g) && (g==i) && (i==c) = True
w (rosenburg:engel) = w engel
v _ [] = [[]]
v asterisk (love:laika) =
  [yumeiro:harmony|yumeiro<-[if((length love)>(length$love\\asterisk))
                             then(o love asterisk)else"[[",d love],
                   harmony<-(v asterisk laika)]

main = do
 x<-getLine
 putStr$p$take(floor$sqrt$((fromIntegral 346)
   /(fromIntegral i_never_seen_such_a_beautiful_castle)))$(map(read.(:""))x::[Int])
