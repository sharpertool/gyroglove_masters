#!/usr/bin/env python

import os.path
import re
import time
from time import localtime

fname = 'Revision.h'

if not os.path.isfile(fname):
    print("File %s does not exist." % fname)
    os.sys.exit(-1)


b = os.path.basename(fname)
bufile = b + '.bak'

''' Open both files, write out to the backup file '''
fpbu = open(bufile,'w')
fp = open(fname,'r')

''' make a copy '''
for line in fp:
    fpbu.write(line)

fpbu.close()
fp.close()

''' Now open the backup and write to the original file '''
fpbu = open(bufile,'r')
fp = open(fname,'w')

for line in fpbu:
    m = re.search('RevInc\s*=(\s*)(\d+)\s*;',line,re.IGNORECASE)
    if m:
        spc = m.group(1)
        inc = int(m.group(2))
        #inc = inc + 1
        line = re.sub("=\s*\d+;","=%s%s;" % (spc,inc),line)
        print("Updated RevInc")

    m = re.search('DateMonth\s*=(\s*)(\d+)\s*;',line,re.IGNORECASE)
    if m:
        spc = m.group(1)
        line = re.sub("=\s*\d+;","=%s%s;" % (spc,(time.strftime("%m",localtime()))),line)

    m = re.search('DateDay\s*=(\s*)(\d+)\s*;',line,re.IGNORECASE)
    if m:
        spc = m.group(1)
        line = re.sub("=\s*\d+;","=%s%s;" % (spc,int(time.strftime("%d",localtime()))),line)

    m = re.search('DateYear\s*=(\s*)(\d+)\s*;',line,re.IGNORECASE)
    if m:
        spc = m.group(1)
        line = re.sub("=\s*\d+;","=%s%s;" % (spc,time.strftime("%Y",localtime())),line)

    m = re.search('TimeHour\s*=(\s*)(\d+)\s*;',line,re.IGNORECASE)
    if m:
        spc = m.group(1)
        line = re.sub("=\s*\d+;","=%s%s;" % (spc,int(time.strftime("%H",localtime()))),line)

    m = re.search('TimeMin\s*=(\s*)(\d+)\s*;',line,re.IGNORECASE)
    if m:
        spc = m.group(1)
        line = re.sub("=\s*\d+;","=%s%s;" % (spc,int(time.strftime("%M",localtime()))),line)

    fp.write(line)



fp.close()
fpbu.close()
