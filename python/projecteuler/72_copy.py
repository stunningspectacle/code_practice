#/usr/bin/python

from euler import is_prime
from math import sqrt

def primes(m, n):
    '''Find all primes in [m, n).'''
    a = []
    if 2 < n and m < n :
        b = primes(2, int((n+0.2)**0.5)+1)
        for s in xrange(max(2, m), n, 20000):
            k = min(20000, n - s)
            c = k * [True]
            for p in b:
                i = max(p, (s+p-1)//p) * p - s
                if i < k:
                    c[i::p] = ((k-i+p-1) // p) * (False,)
            a.extend(s+i for i in xrange(k) if c[i])
    return a
def divisorDistribution(n):#optimized
    divisorsDict={}
    divsor = 1
    while n> 1:
        isPrime = True
        if is_prime(n):
            if n in divisorsDict:
                divisorsDict[n] += 1
            else:
                divisorsDict[n] = 1
            break
        for i in primeList:
            if i < divsor:
                continue
            if n%i==0:
                n = n/i
                divsor = i
                isPrime = False
                if i in divisorsDict:
                    divisorsDict[i] += 1
                else:
                    divisorsDict[i] = 1
                break
        if isPrime:
            if n in divisorsDict:
                divisorsDict[n] += 1
            else:
                divisorsDict[n] = 1
            break
    return divisorsDict
def getPhi(n):
    keysList=divisorDistribution(n).keys()
    phi=n
    for key in keysList:
        phi=phi*(key-1)/key
    return phi
count=0   
primeList =primes(1,1000000)
for d in range(2,1000001):
    if d%1000==0:
        print d
    count+=getPhi(d)
print count
