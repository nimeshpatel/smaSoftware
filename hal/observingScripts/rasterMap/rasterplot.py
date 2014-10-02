''' a reworking of plotscans in python to work alongside the grid cpoint script; rastermap.pl
written by Kieran Finn 28/06/12
contact kieran.finn@hotmail.com'''
#import modules
import sys
import time
import os
import numpy as np
if 'win' in sys.platform: #need different modules depending on operating system in order to have non-blocking keyboard input
    import msvcrt
    winflag=True
else:
    import select
    import termios
    import tty
    winflag=False
try:
    import pylab as p
    pflag=True
except ImportError:
    print "This computer doesn't have pylab installed. Turning plotting capabilities off."
    pflag=False
print '\n\n'

#define functions
def usage():
    print 'continually looks for updated files and when a new copy is available adds \nits data to the database and builds up a 2D plot.'
    print 'options (option=default):'
    print 'update=1; how often to check for a new file.(seconds)'
    print 'rx=L; whether to use the low (l,L,low) or high (h,H,high) frequency continuum channel'
    print 'old; will plot old data immediately without waiting for an update. This will also give you the opportunity to choose your own files.'
    print 'Example: python rasterplot.py update=2 rx=high old'
    print '\n'
    print 'At any time during operation:'
    print 'q will quit the program and show the latest plot(s) and approximate location of the planet'
    print 'd will redraw the plot(s)'
    sys.exit()
    return

def checkforfile(fname): #performs a non-blocking check to see if the file has been updated since the last check
    global lastupdate
    try:
        currentupdate=os.path.getmtime(fname)
    except:
        return False #the file does not exist so definitely doesn't contain new data
    out=(currentupdate>lastupdate[fdict[fname]])
    lastupdate[fdict[fname]]=currentupdate
    return out

def updatedata(fname): #collects new data from the file fname and adds it to the database then calls plotdata to update the plots
    print '%s Updating' %time.ctime()
    global data
    f=open(fname,'r')
    temp=[]
    i=0
    for line in f:
        if '#' in line:
            continue #ignore comments and header
        words=line.strip().split() #.strip makes sure any indent is ignored
        try:
            azoff=float(words[0].strip().rstrip())#0
            eloff=float(words[1].strip().rstrip())#1
            power=float(words[rx].strip().rstrip())#2 or 3
        except ValueError:
            print 'ERROR IN FILE'
            print 'trying to read %s' %fname
            print 'content:'
            print '\n'
            f.seek(0)
            for line in f:
                print line
            f.close()
            sys.exit()
        temp.append([azoff,eloff,power])
    data[fdict[fname]]=temp #writes over any existing data so will make new plots if the rastermap is restarted
    findmax(fdict[fname])#shows current maximum so you can check on its progress
    if pflag:
        plotdata(fdict[fname])
    return True

def plotdata(index): #updates the plots. redraws them from scratch in case rastermap is restarted
    global figs
    global nums
    antenna=index+1
    if not figs[index]:
        f=p.figure(antenna)
        nums.append(f.number)#this is used to redraw the plot if it goes stale
        figs[index]=f.add_subplot(111)#this is used to refer to each figure directly
    x=[]
    y=[]
    z=[]
    xmin=np.inf
    ymin=np.inf
    xmax=-np.inf
    ymax=-np.inf
    for datum in data[index]:
        x.append(datum[0])
        y.append(datum[1])
        if datum[0]<xmin:
            xmin=datum[0]
        if datum[0]>xmax:
            xmax=datum[0]
        if datum[1]<ymin:
            ymin=datum[1]
        if datum[1]>ymax:
            ymax=datum[1]
        z.append(datum[2])
    try:
        xh=data[index][1][0] #assumes the data is an approximate grid with an az scan at each el position
        xl=data[index][0][0] #findes the az grid size by taking the difference of the first two data entries.
    except IndexError:
        print 'not enough data to plot'
        return False
    try:
        yl=data[index][0][1]
        i=1
        while data[index][i][0]>data[index][i-1][0]: #increments i until it reaches the end of the az scan. then determins el grid size by taking the difference between this and the first entry
            i+=1
        yh=data[index][i][1]
    except IndexError: #occurs if only one azscan has completed
        yh=3
        yl=0 #plots a single line with width 3 arcsecs
    xres=(xh-xl)/3
    yres=(yh-yl)/3
    x=[]
    y=[]
    i=xmin-xres
    while (i<=(xmax+xres)):
        x.append(i)
        i+=xres
    i=ymin-yres
    while (i<=(ymax+yres)):
        y.append(i)
        i+=yres
    z=np.zeros((len(y),len(x)))
    for i in range(len(x)):
        for j in range(len(y)):
            z[len(y)-1-j][i]=nearest(x[i],y[j], index)#p.imshow plots the z[0][i] at the top so need to invert the y values
    try:
        figs[index].imshow(z, cmap=p.cm.gray, extent=[xmin-(3*xres/2),xmax+(3*xres/2),ymin-(3*yres/2),ymax+(3*yres/2)], interpolation='nearest') #x=az, y=el, z=continuum power
        figs[index].set_xlabel('azoff')
        figs[index].set_ylabel('eloff')
        figs[index].set_title('Antenna %d' %antenna)
    except:
        print 'could not update figure %d.' %antenna
        pass
    return True

def nearest(X,Y, index): #finds the data point nearest to the given x,y coordinate and returns the power
    rmin=np.inf
    for datum in data[index]:
        x,y,z=datum
        r=(x-X)*(x-X)+(Y-y)*(Y-y)
        if r<rmin:
            rmin=r
            out=z
    return out

def findmax(index): #finds the maximum power in the data for a given antenna and prints its position
    if not data[index]:
        return False #doesn't show anything if no data exists for that antenna
    maxpow=-np.inf
    for i in range(len(data[index])):
        if data[index][i][2]>maxpow:
            maxpow=data[index][i][2]
            azoff,eloff=data[index][i][:2]
    print 'Maximum power for antenna %d is %.5f found at azoff=%.1f, eloff=%.1f.' %(index+1, maxpow, azoff, eloff) #make edit here to change precision of data shown
    return True

def minmax(index): #finds the minimum and maximum power for a given antenna, then prints their location as well as the mid point between them (which is the most likely spot for the planet)
    if not data[index]:
        return False #doesn't show anything if no data exists for that antenna
    print '\n------ANTENNA %d------' %(index+1)
    maxpow=-np.inf
    minpow=np.inf
    for i in range(len(data[index])):
        if data[index][i][2]>maxpow:
            maxpow=data[index][i][2]
            maxazoff,maxeloff=data[index][i][:2]
        if data[index][i][2]<minpow:
            minpow=data[index][i][2]
            minazoff,mineloff=data[index][i][:2]
    avaz=(minazoff+maxazoff)/2
    avel=(mineloff+maxeloff)/2
    print 'Maximum power for antenna %d is %.5f found at azoff=%.1f, eloff=%.1f.' %(index+1, maxpow, maxazoff, maxeloff) #make edit here to change precision of data shown
    print 'Minimum power for antenna %d is %.5f found at azoff=%.1f, eloff=%.1f.' %(index+1, minpow, minazoff, mineloff) #make edit here to change precision of data shown
    print 'Assuming chopping, do a cpoint for antenna %d at azoff=%.1f, eloff=%.1f.' %(index+1, avaz,avel)
    return True

def getkey(): #non-blocking check if key has been pressed
    if winflag: #needs different method for windows and linux
        if msvcrt.kbhit():
            out=msvcrt.getch()
        else:
            return False
    else:
        if select.select([sys.stdin], [], [], 0) == ([sys.stdin], [], []):
            out=sys.stdin.read(1)
        else:
            return False
    if out=='d':
        pupdate()
    return out=='q'

def pupdate(): #redraws the figures if they go stale
    if pflag:
        for num in nums:
            p.figure(num)
            p.draw()
            p.draw()
    return True

#get options
if len(set(sys.argv[1:])&set(['help','h','-h','--help',])):#doesn't matter where help option is it will be found
    usage()

args={}
for i in sys.argv[1:]:
    if '=' in i:
        words=i.split('=')
        args[words[0]]=words[1]
    else:
        args[i]=True

update=1.0
try:
    update=int(args['update'])
except ValueError:
    print 'update must be an integer, using default of %d' %update
except KeyError:
    print 'Update not specified. Using default of %d' %update

rx=2
try:
    if args['rx']in ['l','L','low']:
        rx=2
    elif args['rx'] in ['h','H','high']:
        rx=3
    else:
        print 'rx must be l,L,low, h, H or high. Using default of l'
except KeyError:
    print 'rx not specified. Using default of L'

try:
    oldflag=args['old']
except KeyError:
    oldflag=False

fdict={}
lastupdate=[]#list containing the last time the file was modified for each antenna
data=[]#contains all data in the following format data[antenna][point][az,el,pow]
if oldflag:
    start=0 #if the 'old' option is specified the program will plot any data available
else:
    start=time.time()#otherwise it will only start plotting when new data becomes available
figs=[]
nums=[]
for antenna in range(8):#will look at all antenna and plot any that get updated, no need to specify antenna number at the start
    fdict['/sma/rtdata/engineering/cpoint/ant%d/raster.dat' %(antenna+1)]=antenna #this is used so that the correct index (for lists such as data,figs etc) can be found directly from the filename without having to parse it seperately
    lastupdate.append(start)
    data.append([])
    figs.append(False)

if oldflag:#offer the user the opportunity to specify their own files
    if raw_input('Would you like to use the default files? ') not in ['y','Y','yes','Yes','1']:
        fdict={}
        ants=raw_input('Which antennas would you like to use (separate with space)? ').split()
        for ant in ants:
            try:
                ant=int(ant)
                fdict[raw_input('Which file would you like to use for antenna %d? ' %ant)]=ant-1
            except ValueError:
                print 'Error: %s is not a number' %ant
                
FILES=fdict.keys()#list of all files to look at and plot


#BEGIN MAIN PROGRAM

print 'Beginning search. press q at any time to find maximum and quit.'
if pflag:
    print 'Press d at any time to redraw the figures'
if not winflag:
    old_settings=termios.tcgetattr(sys.stdin) #set up keyboard for non-blocking input
    tty.setcbreak(sys.stdin.fileno())
try: #the main loop is contained in a try: finally: wrapper so that the keyboard can be reset regardless of how it quits
    if pflag:
        p.ion()   #sets pylab in interactive mode so plots can be shown in a non-blocking way
#main loop
    while True:
        for f in FILES:
            if checkforfile(f):
                updatedata(f)
                pupdate()
        start=time.clock()
        while time.clock()-start<update:
            if getkey(): #getkey returns true only if the q key is pressed
                for i in range(8):
                    minmax(i)
                if pflag: 
                    p.ioff()
                    p.show()
                sys.exit()
finally:
    if not winflag:
        termios.tcsetattr(sys.stdin, termios.TCSADRAIN, old_settings) #reset keyboard
