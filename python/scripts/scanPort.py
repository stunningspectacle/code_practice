#!/usr/bin/python
# -*- coding: gb18030 -*-
from socket import *
import os
import sys

type = sys.getfilesystemencoding()
def getPort(fileroute = "PORTS.txt"):
    list={}
    fileobject = open(fileroute,'r')
    for eachLine in fileobject:
	strs = str(eachLine).split('=')
    if len(strs) > 1 :
	list[int(strs[0])] = strs[1]
    return list

#list = getPort()
#keys = list.keys()
#keys.sort()
#
HOST = sys.argv[1]
#fileobject = open("D:\\result1.txt",'w')
#tcpCliSock = socket(AF_INET,SOCK_STREAM)
#result = ''
#
#list_sort = sorted(list, key = lambda d:d[0], reverse = False)

print "HOST is ", HOST

for p in xrange(911, 10000):
    #if p % 100 == 0:
    print "p =", p
    try:
	tcpCliSock = socket(AF_INET,SOCK_STREAM)
	tcpCliSock.connect((HOST,p))
	tcpCliSock.close()
	del tcpCliSock
	result = str(p)+" -> " + "open"
	print result
	#fileobject.writelines([result])
    except error:
	pass
    continue
