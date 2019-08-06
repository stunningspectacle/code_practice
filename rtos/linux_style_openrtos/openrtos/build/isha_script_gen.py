# Generates isha script for GLV
#
# Author: Yonadav Noiman
# With the generous help of: Alex Brill

import argparse
import os
import struct

#change this to control code vs. data banks in SRAM
#iccm_limit is in 4K units e.g. 0x30 means 192K for code and 192K for data
#more possible values: 0x20, 0x40
ICCM_LIMIT = 0x20 
ISH_ENTRY_POINT = 0xff000000

class ScriptCommand(object):
	def __init__(self):
		pass
	
	def bin(self):
		return self.bytes

class DDCommand(ScriptCommand):		
	def __init__(self, db, dwords, poll):	
		self.bytes = bytearray()
		len_dw = len(dwords)
		self.bytes.extend(struct.pack('BBBB',len_dw, poll, 0x0, 0x0))
		self.bytes.extend(struct.pack('I',db))
		for dw in dwords:
			self.bytes.extend(struct.pack('I',dw))
		self.bytes.extend([0]*(32-len_dw)*4)		

class InitMemCommand(DDCommand):		
	def __init__(self, bank_bitmap):
		super(InitMemCommand, self).__init__(0xc0013404, [bank_bitmap], 1)	

class ReadCommand(DDCommand):		
	def __init__(self, addr, len):
		super(ReadCommand, self).__init__(0xc0023408, [addr, len], 1)	

class WriteCommand(DDCommand):		
	def __init__(self, addr, dwords_list):
		len_dw = len(dwords_list)
		super(WriteCommand, self).__init__(0xc0033400|(len_dw*4+8), [addr,len_dw] + dwords_list, 1)		

class ExecuteCommand(DDCommand):		
	def __init__(self, jump_addr):
		super(ExecuteCommand, self).__init__(0xa0043404, [jump_addr], 0)	
		
		
class LoadCommand(ScriptCommand):		
	def __init__(self, code_start, code_end, data_start, data_end):
		self.bytes = bytearray(struct.pack('BBBBI',0x4, 0x0, 0x0, 0x20, 0x0))
		self.bytes.extend(struct.pack('IIII', code_start, code_end, data_start, data_end))
		self.bytes.extend([0]*28*4)	
		
		
class EomCommand(ScriptCommand)	:
	def __init__(self):
		self.bytes = bytearray(struct.pack('BBBB',0x0, 0x0, 0x0, 0x40))
		self.bytes.extend([0]*33*4)	
		
class LoadAddresses(ScriptCommand) :
	def __init__(self, code_start, code_end, data_start, data_end):
		self.bytes = bytearray(struct.pack('IIII', code_start, code_end, data_start, data_end))
		
class Script(object):
	def __init__(self, path):
		self.path = path
		self.cmd_list = []
		
	def add(self, cmd):
		self.cmd_list.append(cmd)
		
	def flush(self):
		f = open(self.path, 'wb')
		for cmd in self.cmd_list:
			f.write(cmd.bin())
		f.close()
		


symbols_in_elf_demand_paging = ['__bootcode_start', '__bootcode_end', '__bootdata_start', '__bootdata_end']
symbols_in_elf = ['__bootcode_start', '__code_end', '__bootdata_start', '__data_end']
symbol_addresses = {}
if __name__ == '__main__':
	print "generating ISHA script..."

	parser = argparse.ArgumentParser(description='Generates ISHA script')
	parser.add_argument('--elf', dest='elf',  default="", help='Relative path to input ELF.', required=True)
	parser.add_argument('--toolchain', dest='toolchain',  default="", help='Relative path to toolchain', required=True)
	parser.add_argument('--proj_config', dest='proj_config', default="", help='Relative path to project_config.h ', required=True)
	parser.add_argument('--bsp_config', dest='bsp_config', default="", help='Relative path to bsp_config.h ', required=True)
	parser.add_argument('--script', dest='script',  default="", help='create script.bin?', required=True)
	
	args = parser.parse_args()

	#extract ISH_BOOT_START from bsp_config.h for paging disabled case
	pc_path = os.path.join(args.bsp_config, 'bsp_config.h')
	if not os.path.exists(pc_path):
		raise Exception('Cannot find bsp_config.h at %s' %(pc_path,))
		
	f = open(pc_path, 'r')
	for l in f.readlines():
		ll = l.split()
		if (ll and len(ll)>1):
			if (ll[1]=='ISH_CONFIG_SRAM_BASE'):
				ISH_BOOT_START = int(ll[2],16)
	f.close()	
	
	#print ISH_BOOT_START
	
	#extract paging state from project_config.h
	pc_path = os.path.join(args.proj_config, 'project_config.h')
	if not os.path.exists(pc_path):
		raise Exception('Cannot find project_config.h at %s' %(pc_path,))
		
	f = open(pc_path, 'r')
	#paging values: 0 - disabled, 1 - enabled_no_demand, 2 - enabled
	paging = 0
	for l in f.readlines():
		ll = l.split()
		if (ll and len(ll)>1):
			if (ll[1]=='ISH_CONFIG_SUPPORT_PAGING'):
				paging += int(ll[2])
			if (ll[1]=='ISH_CONFIG_SUPPORT_DEMAND_PAGING'):
				paging += int(ll[2])
	f.close()
	
	#print paging	
	
	if (paging != 0):
		ISH_BOOT_START = 0x1000
		
	if (paging == 2):
		#demand paging enabled
		symbols = symbols_in_elf_demand_paging
	else:
		symbols = symbols_in_elf
		
	objdump_exe_path = os.path.join(args.toolchain, 'bin\i486-elf-objdump.exe')
	if not os.path.exists(objdump_exe_path):
		raise Exception('Cannot find objdump.exe at %s' %(objdump_exe_path,))
		
	if not os.path.exists(args.elf):
		raise Exception('Cannot find elf file at %s' %(args.elf,))
	
	objdump_result = os.path.join(os.path.dirname(args.elf), 'symbols.out')
	
	# get symbols table out of the elf file, write it to a file
	os.system('%s -t %s > %s' % (objdump_exe_path, args.elf, objdump_result))
	
	f = open(objdump_result, 'r')
	
	# walk thorough the symbols
	for l in f.readlines():
		#print l
		for symbol in symbols:
			ll = l.split()
			if ll and (ll[-1] == symbol):
				# found symbol, put address to hash
				symbol_addresses[symbol] = int('0x%s' % (ll[0],),16)-ISH_BOOT_START
	
	f.close()
	os.remove(objdump_result);
	
	#print symbol_addresses
	
	if (args.script == 'true'):
		# now generate the script
		script_path = os.path.join(os.path.dirname(args.elf), 'script.bin')
		
		isha_script = Script(script_path)
		
		
		# actual script building
		isha_script.add(InitMemCommand(0x3f))	
		isha_script.add(LoadCommand(*[symbol_addresses[s] for s in symbols]))
		isha_script.add(WriteCommand(0x500040,[ICCM_LIMIT]))
		code_banks = ICCM_LIMIT/0x10
		for i in range (0, code_banks):
			isha_script.add(WriteCommand(0x500050+i*4,[i]))
		isha_script.add(ExecuteCommand(ISH_ENTRY_POINT))
		isha_script.add(EomCommand());
		
		isha_script.flush()
	
	# generate metadata
	meta_path = os.path.join(os.path.dirname(args.elf), 'metadata.bin')
	
	meta = Script(meta_path)
	
	meta.add(LoadAddresses(*[symbol_addresses[s] for s in symbols]))
	
	meta.flush()
	
	
	