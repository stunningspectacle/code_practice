#!/usr/bin/python

from math import sqrt

def getNext(a,b,c):  #61, 7, 1
 i = abs(a-b*b)/c
 n = b/i
 if i!=1:
     n = n+1
     b1 = n*i-b
     while True: 
	 if a-(b1+i)**2<0:
	     break
	 b1 = b1+i
	 n = n + 1
     return [n,a,b1,i]
 else:
     return [n+int(sqrt(a)),a,int(sqrt(a)),1]
	 
def f(n): 
 global aList
 aList=[0]
 k = sqrt(n)
 if k ==int(k):
     return False
 a = n
 b = int(k)
 c = 1  #61, 7, 1
 t=getNext(a,b,c)
 aList.append(t[0])
 aList[0] += 1
 while True:
     if t[2]==b and t[3]==c:
	 break
     t=getNext(t[1],t[2],t[3])
     aList.append(t[0])
     aList[0] += 1
 if aList[0]%2==0:
     return True
 else:
     aList = aList+aList[1:]
     aList[0] = aList[0]*2
     return True

aList=[]
maxx = 0
result = 0
TOP = 62

for D in range(61, TOP):
 if f(D):
     p=[]
     q=[]
#          print aList[0]
     for i in range(aList[0]+1):
	 p.append(0)
	 q.append(0)
     p[0] = aList[-1]/2
     p[1] = aList[1]*p[0]+1
     q[0] = 1
     q[1] = aList[1]
     for i in range(2,aList[0]):
	 p[i]=aList[i]*p[i-1]+p[i-2]
	 q[i]=aList[i]*q[i-1]+q[i-2]
     x = p[aList[0]-1]
     y = q[aList[0]-1]
     print aList
     print p
     print q
     print '%d**2-%d*%d**2=%d'%(x,D,y,x**2-D*y**2)
     if x > maxx:
	maxx = x
	result = D

print result,maxx
