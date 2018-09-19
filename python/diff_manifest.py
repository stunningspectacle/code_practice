#!/usr/bin/python

import os,sys
from xml.dom import minidom
import re
#os.system("repo init -u ssh://ctegerrit.sh.intel.com/manifest -b s/sofia3g/mrd -m default-pv.xml")
#os.system("repo sync -c -j32")

PWD =os.getcwd()
oldmanifest=PWD+"/0608.xml"
newmanifest=PWD+"/0609.xml"
print 1
def getAllProjects(manifestObj):
	allProjects = {}
	itemlist = manifestObj.getElementsByTagName('project')
	for item in itemlist:
		allProjects[item.attributes['name'].value]=item.attributes['revision'].value
	return allProjects

oldProjs = getAllProjects(minidom.parse(oldmanifest))
newProjs = getAllProjects(minidom.parse(newmanifest))

##
newAddedProject =[]
PWD = os.getcwd()
for k in newProjs.keys():
	if k in oldProjs.keys():
		if newProjs[k] != oldProjs[k]:
			os.popen("repo forall %s -c 'mkdir -p %s/patches/$REPO_PATH'" % (k,PWD))
			os.popen("repo forall %s -c 'git format-patch %s..%s -o %s/patches/$REPO_PATH'" %(k,oldProjs[k],newProjs[k],PWD))
	else:
		newAddedProject.append(k)

os.system("tree -fi patches|grep -E 'patch$' >1")



