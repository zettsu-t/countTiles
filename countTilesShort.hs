import Data.List
c x=concat x
f x=filter x
l x=length x
m x y=map x y
s x=sort x
p=(m(\i->[i..i+2])[1..7])++(c$m(\i->[[i,i,i],[i,i]])[1..9])
t 0=[[]]
t x=[i:j|i<-p,j<-(t(x-1))]
r=foldr(++)""
k x=m show x
n x=r$k x
o e []=[[]]
o e (x:y)=[i:j|i<-[if((l x)>(l$x\\e))then("["++(n(x\\e))++"]")else"[[","("++(n x)++")"],j<-(o e y)]
q []=False
q (a:b:i:j:x:y)|(a==b)&&(b==i)&&(i==j)&&(j==x)=True
q (a:x)=q x
u x e=c$m(o[e])$f(\i->((s$x++[e])==(s$c i))&&not(q(s$c i)))$t 5
v x=(((l x)==23)&&((l((x\\"[")\\"["))==22))
w x=intercalate"\n"$m(f(/='"'))$k$f v$m r$nub$s$m s$c$m(u x)[1..9]
main=do
 x<-getLine
 putStrLn$show$w$take 13$(m(read.(:""))x::[Int])
