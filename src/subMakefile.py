import os,sys,re,fileinput

basedir = os.path.abspath(os.getcwd())+'/'
projectnames = ['ipmain','rtimage','encbox','mp_hdmodule','mp_hdmodule_m','mp_diskman','mp_diskman_m','recordplay','watch_process']
newline=''
tempdir = sys.argv[1]
targetdir = '\tcp -frv $(EXEC) '+tempdir+'\n'

rpattern = '\tcp[-f ]*?[^ ]*? (.*?)\n'
rp = re.compile(rpattern)

for filename in projectnames:
    fn = basedir + filename
    fn += '/Makefile'
    newmakefile=''
    print fn
    for line in fileinput.input(fn):
	if line.startswith('\tcp '):
                print line
		cpline = line
		match = rp.match(cpline)
                if match:
                    newline = re.sub(rp,targetdir,cpline)
                    print cpline.rstrip()+'-->'+newline
                    newmakefile+=newline
                else:
                    newmakefile+=line
                    print 'nothing match'
        else:
            newmakefile+=line
    f = open(fn,'w')
    f.write(newmakefile)
    f.close()





            
    
