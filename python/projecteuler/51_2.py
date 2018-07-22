#!/usr/bin/python
import euler

def replace(a, b, c, d, e, f, ua, ub, uc, ud, ue, r):
   result = 0
   if ua: result += a * 100000
   else:  result += r * 100000
   if ub: result += b * 10000
   else:  result += r * 10000
   if uc: result += c * 1000
   else:  result += r * 1000
   if ud: result += d * 100
   else:  result += r * 100
   if ue: result += e * 10
   else:  result += r * 10
   return result + f

def problem():
   for mask in range(1, 32):
       ua = (mask & 16) / 16
       ub = (mask & 8) / 8
       uc = (mask & 4) / 4
       ud = (mask & 2) / 2
       ue = (mask & 1)
       if ua + ub + uc + ud + ue > 2:
	   continue
       for a in range(ua * 9 + 1):
	   for b in range(ub * 9 + 1):
	       for c in range(uc * 9 + 1):
		   for d in range(ud * 9 + 1):
		       for e in range(ue * 9 + 1):
			   for f in [1, 3, 7, 9]: 
			       num_primes = 0
			       for r in range(1, 10):
				   if euler.is_prime(replace(a, b, c, d, e, f, ua, ub, uc, ud, ue, r)):
				       num_primes += 1
			       if num_primes > 7:
				   print ua, ub, uc, ud, ue
				   for r in range(1, 10):
				       if euler.is_prime(replace(a, b, c, d, e, f, ua, ub, uc, ud, ue, r)):
					   print replace(a, b, c, d, e, f, ua, ub, uc, ud, ue, r)

import time
t = time.clock()
problem()
print time.clock() - t
