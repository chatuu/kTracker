#!/usr/bin/python

import os
import sys
import time
from optparse import OptionParser

import warnings
warnings.filterwarnings(action = 'ignore', category = RuntimeWarning)

## get number of events
def getNEvents(filename):
    from ROOT import TFile
    from ROOT import TTree
    if 'root' in name:
		dataFile = TFile(name, 'READ')
		return dataFile.Get('save').GetEntries()
    else:
		return 0

## Run one job on a given schema
def runCmd(cmd):
    print cmd
    os.system('nice ' + cmd)

## command line parser
parser = OptionParser('Usage: %prog executable sources targets [options]')
parser.add_option('-l', '--list', type = 'string', dest = 'list', help = 'List of run IDs', default = '')
parser.add_option('-m', '--jobs', type = 'int', dest = 'nJobsMax', help = 'Maximum number of jobs running', default = 6)
parser.add_option('-n', '--notify', type = 'string', dest = 'notify', help = 'E-mail sent to notify the end of jobs', default = '')
parser.add_option('-o', '--output', type = 'string', dest = 'output', help = 'Output file name (i.e. call hadd at the end)', default = '')
parser.add_option('-s', '--suffix', type = 'string', dest = 'suffix', help = 'Additional arguments needed in commands', default = '')
parser.add_option('-r', '--refresh', type = 'int', dest = 'refresh', help = 'Number of seconds for process monitor', default = 30)
parser.add_option('-d', '--divide', type = 'int', dest = 'divide', help = 'Divided one job into many jobs with number of events less than divide', default = 0)
(options, args) = parser.parse_args()

exe = args[0]
pattern1 = args[1]
pattern2 = args[2].replace('.root', '')
suffix = options.suffix
runlist = options.list
nJobsMax = options.nJobsMax
sleepTime = options.refresh
nEventsMax = options.divide

## get the username
username = os.environ['USER']

## Read in run list
if os.path.isfile(runlist):
    fin = open(runlist, 'r')
    schemas = [line.strip() for line in fin.readlines()]
    fin.close()
else:
    runlist = raw_input('Input the run list separated by space: ')
    schemas = [word.strip() for word in runlist.strip().split()]

## prepare the command list
cmds = []
for schema in schemas:
    inputFile = pattern1.replace('?', schema)
    arguments = suffix.replace('?', schema)
    outputFile = pattern2.replace('?', schema)
    logFile = 'log_%s_%s' % (exe, schema)
    if nEventsMax == 0:
        cmds.append('./%s %s %s.root %s > %s &' % (exe, inputFile, outputFile, arguments, logFile))
    else:
        nEvents = getNEvents(inputFile)
        nJobs = nEvents/nEventsMax + 1
        nEvtMaxOpt = nEvents/nJobs

        for i in range(nJobs - 1):
            cmds.append('./%s %s %s_%03d.root %d %d %s > %s_%03d &' % (exe, inputFile, outputFile, i, nEvtMaxOpt*i, nEvtMaxOpt, arguments, logFile, i))
        cmds.append('./%s %s %s_%03d.root %d %d %s > %s_%03d &' % (exe, inputFile, outputFile, nJobs-1, nEvtMaxOpt*(nJobs-1), nEventsMax, arguments, logFile, nJobs-1))

## Check the active job list
nSubmitted = 0
nMinutes = 0.
while nSubmitted < len(cmds):
    # control the total number of running programs, no matter it's from this thread or nor
    nRunning = int(os.popen('pgrep -u %s %s | wc -l' % (username, exe)).read().strip())
    print(exe+': '+str(nMinutes)+' minutes passed, '+str(nSubmitted)+'/'+str(len(cmds))+' submitted, '+str(nRunning)+' running ...' )
    for i in range(nRunning, nJobsMax):
        ## check if all jobs are submitted
    	if nSubmitted >= len(cmds): break

        runCmd(cmds[nSubmitted])
        nSubmitted = nSubmitted + 1

    time.sleep(sleepTime)
    nMinutes = nMinutes + sleepTime/60.

## Send out notification if required
if '@' in options.notify:
    subject = '%s finished successfully on %d/%d jobs after %f minutes' % (exe, nSubmitted, len(schemas), nMinutes)
    content = str(sys.argv).strip('[]') + '\n' + str(schemas).strip('[]') + '\n'
    for cmd in cmds:
        content += (cmd + '\n')
        os.system('echo "%s" | mail -s "%s" %s' % (content, subject, options.notify))
