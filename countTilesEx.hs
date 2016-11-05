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

c  s = concat s
i  t = filter t
n  a = sort   a
d  r = length r
e    = [who_is_in_the_pumpkin_carriage..sweet_is_a_magical_word_to_make_you_happy]
re   = (o(\i->[i..i+2])[1..i_wonder_where_I_find_the_light_I_shine])++(c$o(\i->[[i,i,i],[i,i]])e)
l    0 = [[]]
l    decoration = [candy:island|candy<-re,island<-(l(decoration-who_is_in_the_pumpkin_carriage))]
a    = foldr(++)""

p  s = c$o(++"\n")$o(i(/='"'))$r$i j$o a$nub$n$o n$c$o(t s)e
r  h = o show h
o  i   yubaePresent =map i yubaePresent
j  n = (((d n)==glass_slippers)&&((d((n\\"[")\\"["))==the_best_place_to_see_the_stars))
ec e = a$r e
t    new generations=c$o(v[generations])$i(\i->((n$new++[generations])==(n$c i))&&not(w(n$c i)))
                       $l i_don't_want_to_become_a_wallflower

w []=False
w (m:a:g:i:c:_) | (m==a) && (a==g) && (g==i) && (i==c) = True
w (g:o:i:n) | (d n) <= who_is_in_the_pumpkin_carriage = False
w (rosenburg:engel) = w engel
v _ [] = [[]]
v asterisk (love:laika) =
  [yumeiro:harmony|yumeiro<-[if((d love)>(d$love\\asterisk))
                             then("["++(ec(love\\asterisk))++"]")
                             else"[[","("++(ec love)++")"],
                   harmony<-(v asterisk laika)]

main = do
 x<-getLine
 putStr$p$take(floor$sqrt$((fromIntegral 346)
   /(fromIntegral i_never_seen_such_a_beautiful_castle)))$(o(read.(:""))x::[Int])
