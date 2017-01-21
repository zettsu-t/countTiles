import Data.List
c=concat
f=filter
l=length
m=map
s x=sort x
z=[1..9]
p=m(\i->[i..i+2])[1..7]++(z>>=(\i->[[i,i,i],[i,i]]))
t 0=[[]]
t x=[i:j|i<-p,j<-t(x-1)]
r=foldr(++)""
k x=m show x
n=r.k
o e[]=[[]]
o e(x:y)=[i:j|i<-[if(l x>l(x\\e))then("["++n(x\\e)++"]")else"[[","("++n x++")"],j<-(o e y)]
q[]=False
q(a:b:i:j:x:y)|a==x=True
q(a:x)=q x
u x e=c$m(o[e])$f(\i->s(e:x)==(s.c)i&&(not.q.s.c)i)$t 5
v x=22==l(x\\"[[")
w x=c$m(++"\n")$m(f(/='"'))$k$f v$m r$nub$s$m s$c$m(u x)z
main=getLine>>=(\x->putStr$w$take 13$(m(read.(:""))x::[Int]))
