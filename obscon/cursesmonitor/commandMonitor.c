#include <curses.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "monitor.h"
#define DEBUG 0
#define MAX_LINE_LENGTH 1000
#define BELL 7
#define SPACE 1

FILE *commands;
static int firstCall = 1;
static int lastLinePtr;
void chomp(char *a);

int ggetline(char s[], int lim)
{
  int c, i, j, k;
  char *stripped;

  for (i = 0; i < lim - 1 && (c = getc(commands)) != EOF && c != '\n'; i++) {
    if (c == BELL)
      c = (int)SPACE;
    s[i] = c;
  }
  if (c == '\n')
    s[i++] = c;
  s[i] = '\0';
  stripped = malloc(strlen(s)+1);
  strcpy(stripped, s);
  k = 0;
  for (j = 0; i < strlen(stripped); j++)
    if (isupper(stripped[j]))
      s[k++] = stripped[j];
  i = strlen(s);
  free(stripped);
  return i;
}

void commandDisplay(int count, int *offset, int *searchMode, char *searchString,
		    char *fileName, int nLines, int xorigin, int yorigin, 
		    int lineWidth)
{
  int i, linePtr = 0;
  char **lines;
  char buffer[MAX_LINE_LENGTH];
  time_t system_time;
  int col;
  char segment[MAX_LINE_LENGTH];
  int maxLineWidth = 80;
  int extraLines,newy;
  int mallocCount;

#if DEBUG
  printw("Entered commandDisplay() with count=%d, offset=%d, searchMode=%d\n",count,*offset,*searchMode);
  refresh();
  printw("nLines=%d, xorigin=%d, yorigin=%d, lineWidth=%d\n",nLines,xorigin,yorigin,lineWidth);
  refresh();
  printw("searchString=%s, fileName=%s\n",searchString,fileName);
  refresh();
#endif
  mallocCount = 0;
  if (lineWidth > maxLineWidth)
    maxLineWidth = lineWidth;
  if (xorigin != 0)
    /* if we have to use 2 lines on the 'a' page, may as well show more of a long line */
    maxLineWidth = 2*lineWidth;
  do {
    if (count)
      firstCall = 0;
    else
      firstCall = 1;
    if ((count % 15) == 1) {
      if ((xorigin == 0) && (yorigin < 20)) {
	initscr();
#ifdef LINUX
	clear();
#endif
 	move(yorigin+1,xorigin+1);
	refresh();
      }
    }
    if ((*offset) == 0) {
      nLines += 1;
    }
    if (xorigin == 0) {
      lines = (char **)malloc(nLines*((*offset)+1)*sizeof (char *));
      mallocCount++;
    } else {
      /* the extra factor of 2 is needed for wide-screen display in 
      * which we might actually be able to print twice as many lines
      * as specified if people are typing short messages to one another. */
      lines = (char **)malloc(2*nLines*((*offset)+1)*sizeof (char *));
      mallocCount++;
    }
    if (lines == NULL) {
      perror("malloc of lines\n");
      exit(-1);
    }
    for (i = 0; i < nLines*((*offset)+1); i++) {
      lines[i] = NULL;
    }
    commands = fopen(fileName, "r");
    if (commands == NULL) {
      if (mallocCount != 0) {
	fprintf(stderr, "\n\n\ncommandMonitor returning from point 1 with mallocCount = %d\n", mallocCount);
	fprintf(stderr, "Please email this error message to Taco\n");
      }
      return;
    }
    if (fseek(commands, -1920*2*((*offset)+1), SEEK_END)) {
      fclose(commands);
      if (mallocCount != 0) {
        fprintf(stderr, "\n\n\ncommandMonitor returning from point 2 with mallocCount = %d\n", mallocCount);
	fprintf(stderr, "Please email this error message to Taco\n");
      }
      return;
    }
    while (!feof(commands)) {
      ggetline(buffer, sizeof(buffer));
      if (lines[linePtr] != NULL) {
	free(lines[linePtr]);
	mallocCount--;
	lines[linePtr] = NULL;
      }
      lines[linePtr] = malloc(strlen(buffer)+1);
      mallocCount++;
      if (lines[linePtr] == NULL) {
	perror("malloc of lines");
	exit(-1);
      }
      strcpy(lines[linePtr], buffer);
      if (strlen(lines[linePtr]) > 0) {
	if (strlen(lines[linePtr]) >= maxLineWidth) {
	  /* truncate the line at 79 chars (or more if the screen is wider) */
	  lines[linePtr][maxLineWidth-1] = (char)0;
	}
	linePtr++;
	if (linePtr == nLines*((*offset)+1))
	  linePtr = 0;
      }
    }
    move(yorigin,xorigin);
    if (firstCall)
      lastLinePtr = linePtr;
    system_time = time(NULL);  
    if ((xorigin == 0) && (yorigin < 20))
      printw("Last entries in %s as of %s\n",
	     fileName, asctime(gmtime(&system_time)));
    extraLines = 0;
    for (i = linePtr; i < (linePtr+nLines); i++) {
      int indx;

      indx = i - ((*offset)+1)*(nLines-1);
      if (indx < 0)
	indx += ((*offset)+1)*(nLines);
      if (indx >= ((*offset)+1)*(nLines))
	indx -= ((*offset)+1)*(nLines);
      if (xorigin == 0) {
	/* standard display on the 'm' page */
	move(yorigin+i-linePtr+2, xorigin);
	for (col=xorigin; col<(xorigin+lineWidth); col++)
	  printw(" ");
	move(yorigin+i-linePtr+2, xorigin);
	printw("%s", lines[indx]);
      } else {
	/* see how many lines can be fit onto a single line on the extended-width 'a' page */
	if (strlen(&lines[indx][0]) <= lineWidth)
	  extraLines++;
      }
      if ((*searchMode))
	if (strstr(lines[indx], searchString))
	  (*searchMode) = 0;
    }
    if ((xorigin == 0) && (yorigin < 20)) {
      move(yorigin+nLines+2,xorigin);
      for (col=xorigin; col<(xorigin+lineWidth); col++)
        printw(" ");
      if ((*offset) == 0) {
	move(yorigin+nLines+2,xorigin);
	printw("\"-\" backward \"+\" forward \".\" end \"S\" search   ");
	printw("          You are at the file end.");
      } else if ((*offset) == 1) {
	move(yorigin+nLines+3,xorigin);
	printw("\"-\" backward \"+\" forward \".\" end \"S\" search   ");
	printw("You're one page from the file end.");
      } else {
	move(yorigin+nLines+3,xorigin);
	printw("\"-\" backward \"+\" forward \".\" end \"S\" search   ");
	printw("You're %d pages from the file end.", (*offset));
      }
      /* clrtobot(); */
    } else if (yorigin < 20) {
      nLines += extraLines/2;
#if DEBUG
      printw("extraLines = %d\n",extraLines);
      refresh();
#endif
      linePtr = 0;

      /* if this is not here, then monitor segfaults if the xterm is wider than
       * or equal to MINIMUM_SCREEN_WIDTH_FOR_MESSAGES_ON_MAIN_PAGE chars wide. */
      for (i = 0; i < nLines*((*offset)+1); i++)
	lines[i] = NULL;
      if (fseek(commands, -1920*2*((*offset)+1), SEEK_END)) {
	fclose(commands);
	if (mallocCount != 0) {
	  fprintf(stderr, "\n\n\ncommandMonitor returning from point 3 with mallocCount = %d\n", mallocCount);
	  fprintf(stderr, "Please email this error message to Taco\n");
	}
	return;
      }
      while (!feof(commands)) {
	ggetline(buffer, sizeof(buffer));
	if (lines[linePtr] != NULL) {
	  free(lines[linePtr]);
	  mallocCount--;
	  lines[linePtr] = NULL;
	}
	lines[linePtr] = malloc(strlen(buffer)+1);
	mallocCount++;
	if (lines[linePtr] == NULL) {
	  perror("malloc of lines");
	  exit(-1);
	}
	strcpy(lines[linePtr], buffer);
	if (strlen(lines[linePtr]) > 0) {
	  if (strlen(lines[linePtr]) > maxLineWidth) {
	    /* truncate the line at 80 chars */
	    lines[linePtr][maxLineWidth] = (char)0;
	  }
	  linePtr++;
	  if (linePtr == nLines*((*offset)+1)) {
	    linePtr = 0;
	  }
	}
      }
      fclose(commands);
      extraLines = 0;
      /* now actually print the messages */
      for (i = linePtr; i < (linePtr+nLines); i++) {
	int indx;

	indx = i - ((*offset)+1)*(nLines-1);
	if (indx < 0)
	  indx += ((*offset)+1)*(nLines);
	if (indx >= ((*offset)+1)*(nLines))
	  indx -= ((*offset)+1)*(nLines);
	/* auxiliary display on the extended-width 'a' page */
	move(yorigin+2*(i-linePtr)+1-extraLines, xorigin);
	for (col=xorigin; col<(xorigin+lineWidth); col++)
	  /* clear previous line */
	  printw(" ");
	newy = yorigin+2*(i-linePtr)+1-extraLines;
	move(newy, xorigin);
	bzero(segment,sizeof(segment));
	strncpy(segment,&lines[indx][0],lineWidth);
	segment[lineWidth] = 0;
	chomp(segment);
	printw("%s", segment);
	if (strlen(&lines[indx][0]) > lineWidth) {
	  bzero(segment,sizeof(segment));
	  strncpy(segment,&lines[indx][lineWidth],lineWidth);
	  segment[lineWidth] = 0;
	  chomp(segment);
          newy = yorigin+2*(i-linePtr)+2-extraLines;
	  move(newy, xorigin);
	  for (col=xorigin; col<(xorigin+lineWidth); col++)
	    /* clear previous line */
	    printw(" ");
	  move(yorigin+2*(i-linePtr)+2-extraLines, xorigin);
	  printw("%s",segment);
	} else
	  extraLines++;
	if ((*searchMode))
	  if (strstr(lines[indx], searchString))
	    (*searchMode) = 0;
      } /* end 'for i' loop */
      for (i=newy+1; i<yorigin+10; i++) {
	move(yorigin+i, xorigin);
	for (col=xorigin; col<(xorigin+lineWidth); col++)
	  /* clear previous line */
	  printw(" ");
      }
      move(yorigin+10, xorigin);
      switch (upperRightWindow) {
      case UR_MESSAGES:
	printw("Hit <esc> for SMAshLog");
	/*	printw("Hit ^e for stderr, ^l for SMAshLog");	*/
	break;
      case UR_SMASHLOG:
	printw("Hit <esc> for corr.log");
	/*	printw("Hit ^m for messages, ^e for stderr");*/
	break;
      case UR_CORRELATOR:
	printw("Hit <esc> for messages");
	break;
      case UR_STDERR:
	/* still a mess, it wraps around the 'a' page */
	printw("Hit <esc> for messages");
	/*	printw("Hit ^m for messags, ^l for SMAshLog");*/
	break;
      }
    }
    fclose(commands);
    for (i = 0; i < nLines*((*offset)+1); i++)
      if (lines[i] != NULL) {
	free(lines[i]);
	mallocCount--;
	lines[i] = NULL;
      }
    free(lines);
    mallocCount--;
    if ((*searchMode))
      (*offset)++;
    else
      refresh();
  } while ((*searchMode) == 1);
  if (mallocCount != 0 && COLS<MINIMUM_SCREEN_WIDTH_FOR_MESSAGES_ON_MAIN_PAGE){
    fprintf(stderr, "\n\n\ncommandMonitor returning from normal return point with mallocCount = %d\n", mallocCount);
    fprintf(stderr, "Please email this error message to Taco\n");
  }
}

void chomp(char *a) {
  int b = strlen(a)-1; 

  if (a[b] == '\n')
    a[b] = 0;
}
