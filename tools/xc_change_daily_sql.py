'''
state.sqlite:
change_properties TABLE:
changeid
property_name
property_value
70|event.patchSet.number|["1", "Change"]
70|event.change.owner.email|["jack.ren@intel.com", "Change"]
70|event.uploader.name|["Ren, Jack", "Change"]
70|event.change.branch|["froyo-prod-bb", "Change"]
70|event.change.id|["Ie2ee3d873ad9b0fd583057e13999fa8f45255aee", "Change"]
70|event.type|["patchset-created", "Change"]
70|event.patchSet.uploader.email|["jack.ren@intel.com", "Change"]
70|event.patchSet.uploader.name|["Ren, Jack", "Change"]
70|event.change.owner.name|["Ren, Jack", "Change"]
70|event.change.number|["1355", "Change"]
70|event.uploader.email|["jack.ren@intel.com", "Change"]
70|event.change.url|["http://android.intel.com:8080/1355", "Change"]
70|event.change.subject|["tiwl1283: update prebuilt", "Change"]
70|event.patchSet.ref|["refs/changes/55/1355/1", "Change"]
70|event.change.project|["a/bsp/hardware/intel/PREBUILT/tiwl1283", "Change"]
70|event.patchSet.revision|["a8be1df9e4f37d595b68ad752c5d60a5370b24c3", "Change"]
'''
import sqlite3
import os
import time
import subprocess
import xml.dom.minidom
import commands
import json

class patch_info:
	""" class that stores patch's info """
        def __init__(self):
                self.branch=''
                self.change_type=''
                self.project=''
                self.change_number=''
                self.patchset_number=''
                self.owner_email=''
                self.commitid=''
		self.subject=''
        def foutput(self):
                print ("branch=", self.branch)
                print ("change_type=", self.change_type)
                print ("project=", self.project)
                print ("change_number=", self.change_number)
                print ("patchset_number=", self.patchset_number)
                print ("owner_email=", self.owner_email)
                print ("commitid=", self.commitid)
		print ("subject=",self.subject)
	def property_value(self, element):
                str = element[2].split(',')[0][2:-1]
                return str
	def parse_patch_info_from_db(self, raw_data_from_db):
		"""Parse the patch info from the raw date got from database"""
                for i in range(len(raw_data_from_db)):
                        element=raw_data_from_db[i]
			str = self.property_value(element)
#			str = element[2]
                        if cmp(element[1], u"event.change.branch") == 0:
				self.branch=str
                        if cmp(element[1], u"event.type") == 0:
				self.change_type=str
                        if cmp(element[1], u"event.change.project") == 0:
				self.project=str
                        if cmp(element[1], u"event.change.number") == 0:
				self.change_number=str
                        if cmp(element[1], u"event.change.owner.email") == 0:
				self.owner_email=str
                        if cmp(element[1], u"event.patchSet.number") == 0:
				self.patchset_number=str
                        if cmp(element[1], u"event.patchSet.revision") == 0:
				self.commitid=str
                        if cmp(element[1], u"event.change.subject") == 0:
				self.subject="'" + str + "'"
	def parse_patch_info_from_str(self, data_str):
		"""Parse the patch info from the data string"""
		str=data_str.split(' ')
		self.project=str[0]
		self.change_number=str[1].split('/')[0]
		self.patchset_number=str[1].split('/')[1]
		self.owner_email=str[2]
		self.commitid=str[3]
		self.subject=str[4]
		
	def produce_info_dict(self):
		info_dict = {'project':self.project, 'change_number':self.change_number, 'patchset_number':self.patchset_number, 'owner_email':self.owner_email, 'commitid':self.commitid,'subject':self.subject}
		return info_dict
	def get_patch_path(self):
		doc=xml.dom.minidom.parse(code_dir+'/.repo/manifest.xml')
		projects=doc.getElementsByTagName('project')
		for i in range(len(projects)):
			if self.project == projects[i].getAttribute('name'):
				patch_path=projects[i].getAttribute('path')
				break
			else:
				patch_path=' '
		return patch_path


def CheckMergedorAbandoned(change_number):
	""" Check patch's status if it is merged or abandoned """
        cmd = 'ssh android.intel.com gerrit query --format TEXT --current-patch-set status:merged branch:froyo-prod-bb ' + change_number
        p = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
        s = p.stdout.read()
        if 'rowCount: 1' in s:
                return True
        cmd = 'ssh android.intel.com gerrit query --format TEXT --current-patch-set status:abandoned branch:froyo-prod-bb ' + change_number
        p = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
        s = p.stdout.read()
        if 'rowCount: 1' in s:
                return True
        return False


def PatchCreate(origin_path):
	""" Get the patches' info from sqlite table and put into x_sub """
	conn = sqlite3.connect(origin_path)
	c = conn.cursor()
	os.chdir(date_dir)
	patch_list_file = date+"_patch_list"
	while True: 
		p=patch_info()
		cmd = "select distinct changeid from change_properties"
		rec = c.execute(cmd)
		a = c.fetchone()
		if a == None:
			break
		i = a
		cmd = "select * from change_properties where changeid=%d" %i
		rec = c.execute(cmd)
		a = c.fetchall()
		p.parse_patch_info_from_db(a)

		cmd = "delete from change_properties where changeid=%d" %i
                rec = c.execute(cmd)
                conn.commit()
		
		info_dict = p.produce_info_dict() 
		'''	
		if (cmp(p.change_type, u'change-merged')==0 or cmp(p.change_type, u'change-abandoned')==0):
			if (p.change_number in x_sub.keys()):
				del x_sub[p.change_number]
		else:
			if (cmp(p.change_type, u'patchset-created')==0 and cmp(p.branch, u'froyo-prod-bb')==0):
				if (p.change_number in x_sub.keys()):
					num=x_sub[p.change_number]['patchset_number']
					if (p.patchset_number > num):
						x_sub[p.change_number]=info_dict
				else:
					x_sub[p.change_number]=info_dict
		'''
		if (cmp(p.branch, u'froyo-prod-bb')==0):
                        ret = CheckMergedorAbandoned(p.change_number)
                        if(ret == True):
                                if (p.change_number in x_sub.keys()):
                                        del x_sub[p.change_number]
                        else:
                                if (p.change_number in x_sub.keys()):
                                        num=x_sub[p.change_number]['patchset_number']
                                        if (p.patchset_number > num):
                                                x_sub[p.change_number]=info_dict
                                else:
                                                x_sub[p.change_number]=info_dict

	c.close()
	if DEBUG == True:
		print x_sub.values()
	write_into_file(patch_list_file, x_sub.values())
	os.chdir(work_dir)

def write_into_file(patch_list_file, info_dicts):
	f = open(patch_list_file, 'aw')
	for info_dict in info_dicts:
		str_line=info_dict['project']+' '+info_dict['change_number']+'/'+info_dict['patchset_number']+' '+info_dict['owner_email']+' '+info_dict['commitid']+ ' ' + info_dict['subject']+ '\n'
		f.write(str_line)
	f.close()

def build_patch():
	if DEBUG == True:
		print 'build_patch----------->'
	else:
		cmd = './build.sh mrst_edv -r'
		os.system(cmd)
	
def check_build(): #xc_change
	if DEBUG == True:
		return True
	
	else:
		if os.path.exists('./out/target/product/mrst_edv/boot.tar.gz') == True and os.path.exists('./out/target/product/mrst_edv/system.tar.gz') == True:
			return True
		else:
			return False

def clean_out(): #xc_change
	if DEBUG == True:
		print 'clean out NOW'
	else:
		os.system("sudo umount /out")
		os.system("echo 'y' | sudo mkfs.reiserfs /dev/sda4")
		os.system("sudo mount /dev/sda4 /out")
		os.system("sudo chown matthew:matthew /out -R")


def DownloadPatch():	# x_sub{} has stored valid patches , write them into 'date_patch_list' 
#	path = os.getcwd()
#	date_path=date
	os.chdir(date_dir)
	f_result = open(date+'_result', 'aw')
	
	os.chdir(code_dir)
	r=[]
	for change_number in x_sub.keys():
		clean_out()   #xc_change
		info_dict = x_sub[change_number]

		ret = judge_new_patch(change_number)
		apply_cmd=''
		if (ret == True):
			r=ApplyPatch(change_number)
			if DEBUG == True:
				print 'Download patch cwd:'+os.getcwd()
			apply_cmd = r[1]
			if (r[0] == 0):
	#			f_result.write('http://android.intel.com:8080/#change,' + change_number + ' '+ info_dict['patchset_number']  + '\tConflict\t' + apply_cmd + '\n')
				x_sub[change_number]['result']='conflict'
				resulttosql(change_number)#xc_change
				print ('x_conflict=', x_conflict)
				x_conflict[change_number]=x_sub[change_number]
				continue
			else:
				if change_number in x_conflict.keys():
					del x_conflict[change_number]
				if (r[0] == 3):
					f_result.write('http://android.intel.com:8080/#change,' + change_number + ' ' + info_dict['patchset_number'] +  '\tnothing to commit\t' + apply_cmd + '\n')
					x_sub[change_number]['result']='nothing to commit'
					resulttosql(change_number)
					continue
	
				if (r[0] == 2):
					f_result.write('http://android.intel.com:8080/#change,' + change_number + ' ' + info_dict['patchset_number'] +  '\tPath not exist!!\t' + apply_cmd + '\n')
					x_sub[change_number]['result']='path not exist'
					resulttosql(change_number) #xc_change
					continue
		else:
			continue

		build_patch()

		if check_build() == False:
			clean_out() #xc_change
			build_patch()
			if check_build() == False:
				x_sub[change_number]['result']='fail'
				result_fail(f_result,info_dict['change_number'],info_dict['patchset_number'],apply_cmd)
			else:
				x_sub[change_number]['result']='pass'
				result_pass(f_result,info_dict['change_number'],info_dict['patchset_number'],apply_cmd)
		else:
			x_sub[change_number]['result']='pass'
			result_pass(f_result,info_dict['change_number'],info_dict['patchset_number'],apply_cmd)
		resulttosql(change_number) #xc_change
	f_result.close()
	os.chdir(work_dir)

def resulttosql(change_number):
	conn = sqlite3.connect('/home/matthew/jasper/buildbot/result.sqlite')
        c = conn.cursor()
	cmd='select changeid from check_result where change_number=%d'%int(change_number)
	print ('cmd=', cmd)
	c.execute(cmd)
	changeid=c.fetchone()
	if changeid != None:
		cmd='update check_result set result='+'"'+x_sub[change_number]['result']+'"'+' where changeid=%d'%changeid
		c.execute(cmd)
		conn.commit()
		cmd='update check_result set patchset_number='+'"'+x_sub[change_number]['patchset_number']+'"'+' where changeid=%d'%changeid
		c.execute(cmd)
                conn.commit()
		cmd='update check_result set check_date='+'"'+ time.strftime("%Y/%m/%d %H:%M:%S",time.localtime(time.time()))+'"'+' where changeid=%d'%changeid
		c.execute(cmd)
                conn.commit()	
	else:
		cmd='select distinct changeid from check_result order by changeid desc'
		c.execute(cmd)
		a = c.fetchone()
		if a==None:
			num=0
		else:
			a="%d"%a
			a=int(a)
			num=a+1
		s=x_sub[change_number]['subject']
		if s.count('"')>0:
			s=' '.join(s.split('"'))
		cmd='insert into check_result(changeid,change_number,patchset_number,result,check_date,project,owner_email,subject) values(%d,'%num
		#cmd='insert into check_result(changeid,change_number,patchset_number,result,check_date,project,owner_email,subject,system_tarball,boot_tarball,build_log) values(%d,'%num
#		cmd=cmd+'"'+change_number+'"'+','+'"'+x_sub[change_number]['patchset_number']+'"'+','+'"'+x_sub[change_number]['result']+'"'+ ','+'"'+ time.strftime("%Y/%m/%d %H:%M:%S",time.localtime(time.time()))+'"' +','+'"'+x_sub[change_number]['project']+'"'  +','+'"'+x_sub[change_number]['owner_email']+'"' +','+'"'+x_sub[change_number]['subject']+'"'  +','+'"'+build_result[0]+'"' +','+'"'+build_result[1]+'"' +','+'"'+build_result[2]+'"' +  ")"   #xc_change
		cmd=cmd+'"'+change_number+'"'+','+'"'+x_sub[change_number]['patchset_number']+'"'+','+'"'+x_sub[change_number]['result']+'"'+ ','+'"'+ time.strftime("%Y/%m/%d %H:%M:%S",time.localtime(time.time()))+'"' +','+'"'+x_sub[change_number]['project']+'"'  +','+'"'+x_sub[change_number]['owner_email']+'"' +','+'"'+ s +'"'  +  ")"   #xc_change
		c.execute(cmd)
		conn.commit()
	c.close()
	conn.close()

	

def result_pass(FILE, change_number, patchsetnum, apply_cmd):
	FILE.write('http://android.intel.com:8080/#change,' + change_number + ' ' + patchsetnum + '\tPass\t' + apply_cmd + '\n')
	FILE.flush()
	os.system('cp /vobs/out/target/product/mrst_edv/boot.tar.gz boot_' + change_number + '_' + patchsetnum + '.tar.gz')
	os.system('cp /vobs/out/target/product/mrst_edv/system.tar.gz system_'+ change_number + '_'+ patchsetnum + '.tar.gz')

def result_fail(FILE, change_number, patchsetnum, apply_cmd):
	FILE.write('http://android.intel.com:8080/#change,' + change_number + ' '+ patchsetnum  + '\tFail\t' + apply_cmd + '\n')
	FILE.flush()
	os.system("cp ./out/log " + work_dir + "/" + date_dir + "/" + change_number + "_" + patchsetnum + "_log")
	
def judge_new_patch(change_number):
	""" Judge the patchset if it is a new one """
	if (change_number in x):  #compare with x, not x_sub
		if (x_sub[change_number]['patchset_number'] > x[change_number]['patchset_number']):
			if (change_number not in x_conflict):
				revert_old_patch(change_number)  #first revert old patch, then apply new patchsetNum patch
			else:
				del x_conflict[change_number] 
			x[change_number]=x_sub[change_number]
			return True
		else:
			if (change_number in x_conflict):
				if (x_sub[change_number]['patchset_number'] == x_conflict[change_number]['patchset_number']):
					return True
			else:
				return False
	else:
		x[change_number]=x_sub[change_number]
		return True

def revert_old_patch(change_number):
	""" Revert the old patchset, if got one new patchset which has same change_number """
	print 'NOW into revert_old_patch'
#	s=x[change_number][0:-1]  #xc_change  from x_sub to x
	commitid=x[change_number]['commitid']
    	patch_path=get_patchpath_from_project(x[change_number]['project'])
	if os.path.exists('./'+patch_path) ==True:
		os.chdir(patch_path)
		p=subprocess.Popen('git revert ' + commitid, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
		p.stdout.read()
		os.chdir(code_dir)
		return 1
	else:
		print 'Path not exits!'
		os.chdir(code_dir)
		return 0

def ApplyPatch(change_number):
	""" Cherry-pick one patch """
	print 'into apply_patch'
	subid=change_number[-2:]
	project=x_sub[change_number]['project']
	patchpath=get_patchpath_from_project(project)
	patchset_number=x_sub[change_number]['patchset_number']
	if DEBUG == True:
		print patchpath  #should check patchpath exiting?
		print os.getcwd()
	if os.path.exists('./'+patchpath) == True:
		os.chdir(patchpath)
		apply_cmd='git fetch http://android.intel.com:8080/p/'+project+' refs/changes/'+subid+'/'+change_number+'/'+patchset_number+' && '+'git cherry-pick FETCH_HEAD'
		print apply_cmd
#		p=subprocess.Popen(apply_cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
#		s=p.stderr.read()
		s=commands.getoutput(apply_cmd)
		if (s.find('Finished one cherry-pick') == -1):
			os.system('git reset --hard HEAD')
			ret=0  #can not Apply current patch, conflict
		elif (s.find('nothing to commit') == -1):
			ret=1  #successful
		else:
			ret=3  #nothing to commit
		os.chdir('/vobs')
		return (ret,apply_cmd)
	else:
		ret =2 #Patch path not exist
		return (ret,'')

def get_patchpath_from_project(project_str):
	""" Get the path of the patch's prjoct from manifest.xml """
	doc=xml.dom.minidom.parse(code_dir+'/.repo/manifest.xml')
	projects=doc.getElementsByTagName('project')
	for i in range(len(projects)):
		if project_str == projects[i].getAttribute('name'):
			patch_path=projects[i].getAttribute('path')
			break
		else:
			patch_path=' '
	return patch_path
	
def CreateSqlite():
	""" If result.sqlite is not exist, create it """
	if os.path.exists('result.sqlite') == True:
#		os.system('rm -f result.sqlite')
		return 0
        conn = sqlite3.connect('result.sqlite')
        c = conn.cursor()
        cmd =  'CREATE TABLE check_result (`changeid` INTEGER NOT NULL,`change_number` VARCHAR(18) NOT NULL,`patchset_number` VARCHAR(18) NOT NULL,`result` VARCHAR(18) NOT NULL,`check_date` VARCHAR(18) NOT NULL,`project` VARCHAR(18) NOT NULL,`owner_email` VARCHAR(18) NOT NULL, `subject` VARCHAR(18) NOT NULL)'
        rec = c.execute(cmd)
        cmd = 'CREATE INDEX `check_result_changeid` ON `check_result` (`changeid`)'
        rec = c.execute(cmd)
        c.close()
        conn.close()

def HandleMannualPatch(manual_patch_file):
	""" Add patches from file manual_patch to x_sub """
	f=open(manual_patch_file, 'r')
	fileList=f.readlines()
        for oneline in fileList:
		change_number=oneline.split('/')[0]
		patchset_number=oneline.split('/')[1][0:-1] #exclude the \n
		cmd="ssh android.intel.com gerrit query --format JSON --current-patch-set "+change_number
		p=subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
		s=p.stdout.read()
		a=json.loads(s.split('\n')[0])
		if (a['status'] != u'ABANDONED' and a['status']!= u'MERGED'):
			subject=a['subject']
			project=a['project']
			owner_email=a['owner']['email']
			commitid=a['currentPatchSet']['revision']
			info_dict = {'project':project, 'change_number':change_number, 'patchset_number':patchset_number, 'owner_email':owner_email, 'commitid':commitid,'subject':subject}
			x_sub[change_number]=info_dict
			print (change_number+': ', x_sub[change_number])
	f.close()

def ParseTable():
	""" Parse result table, if the patch that merged or abandoned remove it """
	conn = sqlite3.connect('result.sqlite')
        c = conn.cursor()
	c.execute("select * from check_result")
	a=c.fetchall()
	for one in a:
		change_number=one[1]
		if (CheckMergedorAbandoned(change_number)==True):
			print "delete change_number:"+change_number
			cmd="delete from check_result where changeid=%d"%one[0]
			c.execute(cmd)
			conn.commit()
	c.close()

def DeleteConflict(conflict_number):
	conn = sqlite3.connect('result.sqlite')
        c = conn.cursor()
	print "delete conflict_number:"+conflict_number
	cmd="delete from check_result where change_number=%d"%int(conflict_number)
	c.execute(cmd)
	conn.commit()
	c.close()
	

def main():
	""" Main function """
	CreateSqlite()
	ParseTable()
	if os.path.exists(date_dir) == True:
		os.system('rm -rf ' + date_dir)
	os.system('mkdir ' + date_dir)

	os.chdir(date_dir)
	patch_list_file = date+'_patch_list'
	if os.path.exists(work_dir + '/manual_patch'):
		HandleMannualPatch('/home/matthew/jasper/buildbot/manual_patch')
	os.chdir(code_dir)
	while os.system('repo sync') != 0:   #If sync failed, retry
		print 'repo sync fails, retry after 600 secs'
		time.sleep(600)	
	os.chdir(work_dir)

	while True:
		clock = time.strftime("%H-%M-%S",time.localtime(time.time()))
		hour = clock.split("-")[0]
		if hour == "08" or hour == "09":
			print "end time is:",clock
			exit(0)
		if DEBUG == True:	
			PatchCreate('/home/matthew/jasper/buildbot/db/state.sqlite')
			print 'x_sub in main'
			print x_sub
		else:
			PatchCreate('/home/matthew/buildbot/master/state.sqlite')
		for change_number in x_conflict.keys():
			if  CheckMergedorAbandoned(change_number)==False:
				x_sub[change_number]=x_conflict[change_number]
			else:
				del x_conflict[change_number]
				DeleteConflict(change_number)
		DownloadPatch()
		x_sub.clear()
		
		if DEBUG == True:
			print 'x_conflict has keys:'
			print x_conflict					
			print 'Dictionary x has keys:\n'
			print x
		
		print "sleep"
		time.sleep(600)

if __name__ == "__main__":
	DEBUG = False 
	work_dir = os.getcwd()	#current work directory
	code_dir = '/vobs'	#code tree directory
	out_dir = '/out'	#out directory
	date = time.strftime("%Y-%m-%d",time.localtime(time.time())) #directory where stores a specific day's status
	date_dir = date
	x={}
	x_sub={}
	x_conflict={}
	patch_valid={}
	patch_temp={}
	main()
