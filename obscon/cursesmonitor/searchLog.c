#include <stdio.h>
#include <sys/utsname.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/file.h>
#include <termio.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>

int user;
struct termio   tio, tin;
FILE *logFile;

char fourQueue[5] = {0, 0, 0, 0, 0};
char home[5] = {0x1b, 0x5b, 0x3b, 0x48, 0};
char top[4] = {0x1b, 0x5b, 0x48, 0};
char clearScreen[9] = {0x1b, 0x5b, 0x3b, 0x48, 0x1b,
		       0x5b, 0x32, 0x4a, 0};

int mostRecentTimestamp;
int printedTimestamp;
int jumpIncrement = 15;

void handlerForSIGINT(int signum);

void backwardTwoPages()
{
  int currentLocation, i, fseekReturn;
  int clearsSeen = 0;
  int processed = 0;

  currentLocation = ftell(logFile);
  fourQueue[1] = (char)0;
  while (clearsSeen < 2) {
    currentLocation--;
    fseekReturn = fseek(logFile, currentLocation, SEEK_SET);
    if (fseekReturn) {
      perror("fseek in backwardTwoPages()");
      exit(-1);
    }
    for (i = 3; i > 0; i--)
      fourQueue[i] = fourQueue[i-1];
    fourQueue[0] = getc(logFile);
    processed++;
    if (processed > 4)
      if (!strcmp(fourQueue, home))
	clearsSeen++;
  }
  currentLocation--;
  fseek(logFile, currentLocation, SEEK_SET);
}

void forwardOnePage()
{
  int i, startTime;
  int processed = 0;
  char timeStamp[10];

  startTime = mostRecentTimestamp;
  fourQueue[1] = (char)0;
  timeStamp[9] = (char)0;
  printf("%s", clearScreen);
  putc(getc(logFile), stdout);
  putc(getc(logFile), stdout);
  putc(getc(logFile), stdout);
  while (strcmp(fourQueue, home)) {
    char dropped;
    
    dropped = fourQueue[0];
    for (i = 0; i < 3; i++)
      fourQueue[i] = fourQueue[i+1];
    fourQueue[3] = getc(logFile);
    for (i = 0; i < 8; i++)
      timeStamp[i] = timeStamp[i+1];
    timeStamp[8] = fourQueue[3];
    if (!strcmp(timeStamp, "TIMESTAMP")) {
      fscanf(logFile, "%d", &mostRecentTimestamp);
    }
    processed++;
    if (processed > 4)
      putc(dropped, stdout);
  }
  fflush(stdout);
  if (mostRecentTimestamp == startTime)
    printedTimestamp = 0;
  else
    printedTimestamp = 1;
}

void printHelp(void)
{
  printf("%s", clearScreen);
  printf("Once the requested page is found, you can:\n");
  printf("\tPress \"+\" to move forward one page\n");
  printf("\tPress \"-\" to move backwards one page\n");
  printf("\tPress \"i\" to change jump increment\n");
  printf("\tPress \"t\" to jump forward one time interval\n");
  printf("\tPress \"T\" to jump backwards one time interval\n");
  printf("\tPress \"q\" to quit this Mickey Mouse program\n");
  printf("\n\n\n\n\n\n\n\n\nSearching log file, please wait...\n");
}

main(int argc, char **argv)
{
  int currentHour = -100;
  int currentMinute = -100;
  int i, hh, mm, ss;
  int targetTime = 0;
  int timeBase;
  int firstTimeFound = 0;
  int timeStamp;
  int displayedHelp = 0;
  char searchString[10];
  struct sigaction action, old_action; int sigactionInt;

  if (argc < 3) {
    fprintf(stderr, "Usage: %s {log file name} HH:MM:SS\n", argv[0]);
    fprintf(stderr, "Log files usually reside in /data/engineering/monitorLogs/\n");
    exit(-1);
  }
  logFile = fopen(argv[1], "r");
  if (logFile == NULL) {
    perror("Open on log file");
    exit(-1);
  }
  sscanf(argv[2], "%d:%d:%d", &hh, &mm, &ss);
  targetTime = hh*3600 + mm*60 + ss;
  searchString[9] = (char)0;
  

  /* signal handler for control C */
  action.sa_flags=0;
  sigemptyset(&action.sa_mask);
  action.sa_handler = handlerForSIGINT;
  sigactionInt = sigaction(SIGINT,&action, &old_action);

  /*
   * This is to get the user input as a single unbuffered char and
   * zero-wait
   */
  
  ioctl(0, TCGETA, &tio);
  
  tin = tio;
  tin.c_lflag &= ~ECHO;
  tin.c_lflag &= ~ICANON;
  
  tin.c_cc[VMIN] = 0;
  tin.c_cc[VTIME] = 0;
  
  ioctl(0, TCSETA, &tin);

  printHelp();
  
  while (!feof(logFile)) {
    char inChar;
    int currentLocation;

    inChar = getc(logFile);
    for (i = 0; i < 8; i++)
      searchString[i] = searchString[i+1];
    searchString[8] = inChar;
    if (!strcmp(searchString, "TIMESTAMP")) {
      fscanf(logFile, "%d", &timeStamp);
      mostRecentTimestamp = timeStamp;
      if (!firstTimeFound) {
	timeBase = (timeStamp / 86400) * 86400;
	targetTime += timeBase;
	firstTimeFound = 1;
      }
      if (!displayedHelp)
	if (currentMinute != ((timeStamp-timeBase)/600) - 6*currentHour) {
	  currentHour = (timeStamp-timeBase) / 3600;
	  currentMinute = ((timeStamp-timeBase)/600) - 6*currentHour;
	  printf("\rprocessed to %02d:%02d:00 target %02d:%02d:%02d     ",
		 currentHour, currentMinute*10, hh, mm, ss);
	  fflush(stdout);
	} 
      if (timeStamp > targetTime) {
	displayedHelp = 1;
	forwardOnePage();
	while (((inChar = getc(stdin)) != 'q') &&
	  ((timeStamp > targetTime))) {
	  switch (inChar) {
	  case '+':
	    forwardOnePage();
	    if (printedTimestamp)
	      forwardOnePage();
	    break;
	  case '-':
	    backwardTwoPages();
	    forwardOnePage();
	    if (printedTimestamp)
	      forwardOnePage();
	    break;
	  case 'i':
	    ioctl(0, TCSETA, &tio);
	    printf("\n\nEnter new jump increment (in seconds): ");fflush(stdout);
	    scanf("%d", &jumpIncrement);
	    ioctl(0, TCSETA, &tin);
	    backwardTwoPages();
	    forwardOnePage();
	    forwardOnePage();
	    break;
	  case 't':
	    targetTime = mostRecentTimestamp + jumpIncrement - 5;
	    break;
	  case 'T':
	    currentLocation = ftell(logFile) - (25000 * jumpIncrement/15);
	    if (fseek(logFile, currentLocation, SEEK_SET)) {
	      fprintf(stderr, "fseek error\n");
	      perror("fseek");
	      exit(-1);
	    }
	    targetTime = mostRecentTimestamp - jumpIncrement - 15;
	    timeStamp = -1000;
	    break;
	  }
	  usleep(100000);
	}
	if (timeStamp > targetTime) {
	  printf("Bye.\n");
	  ioctl(0, TCSETA, &tio);
	  exit(0);
	}
      }
    } else {
      /*
      for (i = 0; i < 9; i++)
	if ((searchString[i] < (char)32) ||
	    (searchString[i] > 'z'))
	  searchString[i] = ' ';
      printf("\"%s\"\n", searchString);
      */
    }
  }
}

void handlerForSIGINT(int signum)
{
        user='q'; /* 'q' for quit command */
        fprintf(stderr,"Got the control C signal. Quitting.\n");
	printf("Bye.\n");
	ioctl(0, TCSETA, &tio);
	exit(0);
}
