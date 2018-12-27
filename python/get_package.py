#!/usr/bin/python

import string, sys, time

try:
	from urllib.request import urlopen
except ImportError:
	from urllib2 import urlopen

simics_url = "http://simics-download.intel.com/simics-5/win64/"
CHUNK_SIZE = 1024 * 1024
SLEEP_SECS = 1800
FAILURE = "FAILURE"

packages = (
"simics-pkg-1000",						# Simics base
"simics-pkg-7360", "simics-pkg-7363",				# LKF
"simics-pkg-7340", "simics-pkg-7343",				# TGL
"simics-pkg-7390", "simics-pkg-7393", "simics-pkg-7288",	# OSE
)

#packages = ("simics-pkg-1012", "simics-pkg-2101")
fd_log = 0
count = 0

def log(s):
	fd_log.write(time.asctime() + " :" + s + "\n")
	fd_log.flush()

def get_url():
	err = 0
	#for line in open("url"):
	while True:
		try:
			for line in urlopen(simics_url):
				for pkg in packages:
					start = line.find(pkg)
					if start != -1:
						name = line[start:].split("\"")[0]
						pkg_url = simics_url + name
						yield pkg_url
		except Exception as e:
			log(str(e) + " " + str(err))
			err += 1
			time.sleep(SLEEP_SECS)
			continue
		break


def download_file(url):
	err = 0
	local_filename = url.split('/')[-1]
	try:
		req = urlopen(url)
	except Exception as e:
		log("download_file: " + str(e))
		return FAILURE
	print "Downloading", local_filename,
	with open(local_filename, 'wb') as f:
		while True:
			chunk = req.read(CHUNK_SIZE)
			if not chunk:
				break
			f.write(chunk)
			#f.flush()
			print ".",
			sys.stdout.flush()
	print "finished"
	log("Download " + local_filename + " finished")
	return local_filename

def get_pkg():
	fd_saved = open("pkg_downloaded", "a+")
	pkgs_saved = [pkg for pkg in fd_saved.read().split()]
	#print pkgs_saved
	for pkg_url in get_url():
		filename = pkg_url.split('/')[-1]
		if not filename in pkgs_saved:
			#print filename, "is already downloaded"
			filename = download_file(pkg_url)
			if not filename == FAILURE:
				fd_saved.write(filename + "\n")
	fd_saved.close()

fd_log = open("log", "a+")
while True:
	log("get pkg " + str(count))
	get_pkg()
	count += 1
	time.sleep(SLEEP_SECS)

