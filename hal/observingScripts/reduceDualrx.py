'''Script to reduce cpoint data
written by Kieran Finn 06/27/2012
contact kieran.finn@hotmail.com'''
import sys
import os
from commands import getoutput
import pylab as pl
RECFILE='/sma/observingScripts/receivers.txt' #this is the list of receivers as mentioned later. EDIT THIS IF YOU CHANGE ITS LOCATION

def usage():
    print 'script to reduce dualrx cpoint data.'
    print 'options (option=default):'
    print 'antenna=False: antenna to get data for'
    print 'date=False: date in YYYMMDD format will use files date.low.cpoint and date.high.cpoint'
    print 'low=False, high=False: specify files to use'
    print 'example: python dualrx.py date=20120703 antenna=7'
    sys.exit()
    return 

def recread(f):
'''the details about each receiver are stored in a seperate file receivers.txt in plain
text so it can easily be updated if new receivers are added. The file has th following format:
each line represents a receiver and has its name, code and whether it is a high(1) or low(0) frequency receiver'''
    code={}
    high={}
    f=open(f, 'r')
    for line in f:
        if '#' in line: # any line with # in is treated as a comment and ignored
            continue
        words=line.split()
        if len(words)!=3:
            print 'ERROR IN RECFILE. %s is not in the correct format' %line
            continue
        code[words[0]]=words[1]
        high[words[0]]=bool(int(words[2]))
    return [code,high]#this process converts the recfile into two dictionaries which are used later

def linesplit(l): #turns a list of lines of a cpoint file into a long list of values so that the program can quickly check if a receiver is present
    out=[]
    for i in l:
        for j in i.split(','):
            out.append(j.strip().rstrip())
    return out

def getdate(s): #gets the date from a file name for use in naming future files
'''NOTE: THIS FUNCTION MAY CAUSE ERRORS IF THE FORMAT OF THE CPOINT FILE NAMES IS CHANGED.
CURRENTLY ASSUMING YYYMMDD.low.cpoint OR YYYMMDD.high.cpoint'''
    j=0
    while not s[j].isdigit():
        j+=1
    i=j
    while s[i].isdigit():
        i+=1
    return s[j:i] #simply picks out the first digit in the string

def checkall(l,line): #returns true only if every item of l is in line
    for item in l:
        if item not in line:
            return False
    return True

def grep(l, strings, outname): #writes every line in 'strings' that contains all items of 'l' to a new file 'outname'
    f=open(outname,'w')
    for line in strings:
        if checkall(l,line):
            f.write(line)
    f.close()
    return(True)

def cpdata(infile, file):   #this is a rewriting of cpdata.pl so that I have more control over the format
    f=open(infile,'r')      #it reduces the cpoint file to the much simpler pdata file which is used in subsequent calculations
    lines=[]
    for line in f:
        x=line.split(',')
        temp=[]
        for i in x:
            temp.append(i.strip().rstrip())#removes any whitespace which can cause errors
        lines.append(temp)
    f.close()
    unraw=open(file,'w')
    no=1
    for words in lines:
        source=words[3]
        azact=float(words[40])
        elact=float(words[41])
        azoff=float(words[-4])
        eloff=float(words[-2])
        azofferror=float(words[-3])
        elofferror=float(words[-1])
        utc=float(words[16])
        sptype=words[108]
        unraw.write('%d %s \t %.6f %.6f %.2f %.2f %.2f %.2f %.1f %s\n' %(no, source, azact, elact, azoff, eloff, azofferror, elofferror, utc, sptype))
        no+=1
    unraw.close()
    return(True)

def cp(start, end):#copies file 'start' to file 'end' (could also do os.system('cp %s %s' %(start end) but this is more foolproof)
    s=open(start,'r')
    e=open(end,'w')
    for line in s:
        e.write(line)
    s.close()
    e.close()
    return(True)

def stripfile(name, remove): #removes all lines whose number is in 'remove' from file 'name'
    s=''
    for i in remove:
        s+=str(i+1)+', '
    print 'removing lines %s from %s.' %(s, name)
    f=open(name,'r')
    lines=f.readlines()
    f.close()
    f=open(name,'w')
    for i in range(len(lines)):
        if i not in remove:
            f.write(lines[i])
    f.close()
    return(True)

def sort(list): #uses a bubble sort algorithm to sort 'list' into ascending order
    changed=True
    while changed:
        changed=False
        for i in range(len(list)-1):
            if list[i]>list[i+1]:
                list[i],list[i+1]=list[i+1],list[i]
                changed=True
    return list

pdict={0:'rx',1:'bo'}#the markers to use in the following plotting function
def plot(file,i): #plots az vs el as read from 'file' using marker 'pdict[i]'
    f=open(file,'r')
    x=y=[]
    for line in f:
        words=line.split()
        x.append(float(words[2]))
        y.append(float(words[3]))
    pl.plot(x,y,pdict[i])

lowfile=highfile=False

#get options
'''this program uses a non-standard method to get options I believe there is a python module dedicated
specifically to this but I have never used it. It's called getopt so take a look at the documentation
for that if you want to standardise the input'''
if len(set(sys.argv[1:])&set(['help','h','-h','--help',])):
    usage()

args={}
for i in sys.argv[1:]: #options with values should be of the form e.g. 'date=20110414'
    if '=' in i:
        words=i.split('=')
        args[words[0]]=words[1]
    else:
        args[i]=True

date=antenna=False
try:
    antenna=args['antenna']
    date=args['date']
    lowfile='/sma/rtdata/engineering/cpoint/ant'+antenna+'/'+date+'.low.cpoint'
    highfile='/sma/rtdata/engineering/cpoint/ant'+antenna+'/'+date+'.high.cpoint'
except KeyError:
    if not len(set(args.keys())&set(['lowfile','highfile'])):
        print 'you must choose either a date and antenna or at least one file'
        usage()

try:
    lowfile=args['lowfile']
except KeyError:
    pass
try:
    highfile=args['highfile']
except KeyError:
    pass
        
if not date:
    if lowfile:
        date=getdate(lowfile)
    else:
        date=getdate(highfile)
#BEGIN MAIN PROGRAM

joinflag=True
if lowfile:
    print 'Using data from %s' %lowfile
    f=open(lowfile,'r')
    lowfile=f.readlines()
    f.close()
if highfile:
    print 'Using data from %s' %highfile
    f=open(highfile,'r')
    highfile=f.readlines()
    f.close()

print 'using list of recievers stored in %s' %RECFILE

#populate the list of reciever codes to use

recievers,high=recread(RECFILE)
lines={}
for x in high.keys():
    if high[x]:
        lines[x]=highfile
    else:
        lines[x]=lowfile
        
available=[]
print 'recievers available:'
for rec in recievers.keys():
    if (lines[rec] and recievers[rec] in linesplit(lines[rec])): #lines[rec] will be false if the relevant (high or low) .cpoint file was not given
        print rec
        available.append(rec)
used=[]
while len(used)==0:
    recs=raw_input('Choose receivers to use (seperate with space or type a for all): ').split()
    if 'a' in recs:
        used=available
    else:
        for x in recs:
            if x in available: #can only use available receivers
                used.append(x)
            else:
                print 'the '+x+' reciever is not available'
    if len(used)==0:
        print 'ERROR: no recievers chosen' #if no receivers are chosen program will prompt again

if len(used)!=2:
'''unless the user specifies exactly 2 receivers then we cannot find differences
however the program can still split the files by receiver and can create the pdata files'''
    print 'Cannot join outputs as that requires 2 recievers'
    if raw_input('Procede anyway? ') not in ['yes','y','Yes','Y','1']:
        sys.exit()
    joinflag=False


pfiles=[]
for rec in used:
    #split the original file by reciever
    cpoint=date+'.'+rec+'.cpoint'#name of the raw data file for that receiver
    pdata='pdata.'+rec+'.unraw'#name of the pdata file for that receiver
    print 'Extracting data for %s reciever and storing in %s.' %(rec,cpoint)
    if joinflag and (high[used[0]]!=high[used[1]]): #if we choose a high and a low rx we must look for lines that contain both their codes
        grep([recievers[used[0]],recievers[used[1]]],lines[rec],cpoint)
    else:
        grep([recievers[rec]],lines[rec],cpoint)
    #reduce to simpler form
    cpdata(cpoint, pdata)
    print 'reducing to simpler form and storing in %s.' %pdata
    cp(pdata,'pdata')
    #run fit4 and look for outliers
    print 'running fit4 for %s reciever' %rec
    os.system('fit4 pdata')
    os.system('perl offsetsvsel2.pl %s presults n j' %pdata) #plots offsets vs el so we can inspect for outliers
    outliers=raw_input('Which points are outliers? (seperate with space): ').split()
    #remove outliers
    temp=[]
    for i in outliers: #turn point numbers from 1 indexed strings to 0 indexed and ints
        try:
            temp.append(int(i)-1)
        except ValueError:
            print '%s is not a number' %i
    outliers=temp
    stripfile(pdata, outliers)
    pfiles.append(pdata)

if joinflag:
    #check for missing pairs
    #this code assumes pairs will alternate in their azimuth
    
    print 'checking for missing pairs'
    pairs={}
    f=open(pfiles[0],'r')
    for line in f:
        pairs[float(line.split()[2])]=0
    f.close()
    f=open(pfiles[1],'r')
    for line in f:
        pairs[float(line.split()[2])+0.0000001]=1   #if a high and a low rx are chosen their az and el values will be identical which causes problems identifying pairs with this algorithm.
    f.close()                                       #so a minute offset is added to one set of recievers so they will alternate in az
    p=sort(pairs.keys())
    start=pairs[p[0]]
    x=start
    remove=[[],[]]
    for i in range(1,len(p)):
        if pairs[p[i]]==x: #scans the az of each point and if it comes across the same rx twice in a row adds that to the list for removal
            remove[x].append(i-int(x==start))
        x=pairs[p[i]]
    for i in range(len(remove)):
        if len(remove[i]):
            stripfile(pfiles[i],remove[i])
    print "perform visual check that the code has succesfully deleted points that aren't pair-wise."   
    for i in range(2):
        plot(pfiles[i],i)
    pl.xlabel('az')
    pl.ylabel('el')
    pl.show() #possibility to add user input to pick up anything the above code missed
    
    print 'subtracting %s from %s' %(pfiles[0], pfiles[1])
    os.system('perl feedOffset_subtract.pl %s %s > pdata' %(pfiles[0],pfiles[1])) #the first rx entered is treated as the reference
    print 'final fitting'
    #automatic way of collecting the parameters, currently not working
    '''
    f=os.popen('fitFeedOffsets pdata')
    r=[]
    for line in f:
        r.append(line.split())
    f.close()
    A1a=r[1][3]
    A2a=r[2][3]
    A1e=r[8][3]
    A2e=r[9][3]
    '''
    
    # manual method fallback if errors with automatic way
    os.system('fitFeedOffsets pdata')
    while True:
        params=raw_input('Please enter the parameters to use in the fit (A1a,A2a, A1e, A2e): ')#add error checking
        try:
            A1a,A2a,A1e,A2e=params.split()
            break
        except ValueError:
            print 'Error. please enter four parameters.'
    if not antenna: #occurs if only file names have been specified. Could implement an automatic way to get this but probably not worth it.
        antenna=raw_input('which antenna is this for? ')
    psname='ant%s_%sto%s_feedoffsets.ps' %(antenna,used[0],used[1])
    print 'running plotFeedOffsets and saving the plot to %s' %psname
    os.system('plotFeedOffsets.pl pdata %s %s %s %s %s/cps' %(A1a, A2a, A1e, A2e, psname))
    os.system('plotFeedOffsets.pl pdata %s %s %s %s' %(A1a, A2a, A1e, A2e))

print 'program finished'
