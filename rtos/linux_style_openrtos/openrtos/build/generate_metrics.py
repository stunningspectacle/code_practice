import subprocess
from glob import glob
import os
import sys

from _ctypes import sizeof
if __name__ == '__main__':
	

	files = []
	start_dir = sys.argv[1] 
	image_name = sys.argv[2]
	pattern   = "*.o"

	for dir,_,_ in os.walk(start_dir):
		files.extend(glob(os.path.join(dir,pattern)))
	
	
	print 'Generating metrics summary... '
	fout = open(os.path.join(start_dir, 'FreeRTOS_metrics.out'), 'w')
	
	elf_fname = os.path.join(start_dir, image_name + '.elf')
	fout.write(subprocess.Popen("%%GCC_DIR%%/bin/i486-elf-size.exe %s" % (elf_fname,), shell=True, stdout=subprocess.PIPE).stdout.read())

	for obj in files:
		obj_data = subprocess.Popen("%%GCC_DIR%%/bin/i486-elf-size.exe %s" % (obj,), shell=True, stdout=subprocess.PIPE).stdout.read()
		fout.write(obj_data.split('\n')[1])

	fout.close()
	print 'Generating Done.'
	

