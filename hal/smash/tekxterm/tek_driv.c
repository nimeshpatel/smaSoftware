/******************************
 * tektronix terminal drivers *
 *****************************/

/* Modified for stand alone use by RWW 7/17/98 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "tek_driv.h"

#define PRINT 0

# define ESC 27			/* ascii code for escape character */

static FILE *tout, *tin;
static FILE *savetout;
static char termType[12];


/* Cursor (current location) storage */
static int cx = -20,cy = -20;
static int needsmove = 1;
static int xyvalid = 0;

/* The fflush associated with these patches seems to fix an illusive bug
 * which shows up in Solaris 2.x in which the \037 preceeding a text
 * write gets lost occasionally when writing to an xterm through a pty.
 */
#if SUN_OS && SYSTEM_V
static int iocount = 0;
#endif /* SUN_OS && SYSTEM_V */

#define NORMAL 0
#define XTERM 1
static int tType = 0;

#if PRINT
static int ItsATekFile=0;
static int ItsATekPrinter=0;
static char *startqms = {
"\n\
^PY^-\n\
^IOL^-\n\
^IMH0038010620^IMV0010008100^-\n\
^IGT1^-\n\
^PN^-\n\
"
};
static char *endqms = {
"\n\
^PY^-\n\
^IGE^-\n\
^PN^-"
}; 
#endif /* PRINT */

#if __STDC__ || defined(__cplusplus)
#define P_(s) s
#else
#define P_(s) ()
#endif

/* tek.driver.c */
static void xy P_((register int x, register int y));
#undef P_

/************************************/
/* xterm - use driver for tek401x */
/************************************/

void xterm(void)
{
	int XtermCleanup();
	int XtermSetup();
	static char name[] = "xterm";

	/* if not set up to drive xterm, set it up */
	if(strcmp(termType,name))
	{
		/* in case something happens while setting up */
		/* driver variables, make terminal illegal */
		*termType = 0;
		tType = XTERM;
		tout = stdout;
		tin = stdin;

		/* indicate that driver is set for xterm */
		strcpy(termType,name);
	}
}

/********************************************/
/* setup - set up the terminal for graphics */
/********************************************/

void setup()
{
  if(tType == XTERM) {
		fputs("\033[?38h",tout);
  }
  linetype(255);
  xyvalid = 0;
}

/***********************************/
/* cleanup - clean up the terminal */
/***********************************/

void doflush() {
  fflush(tout);
}

void cleanup()
{
	char ts[128];
	if(tType == XTERM) {
	  /*		fputs("\037\030\033\003", tout);*/
	  /* ESC ETX = \033\03 = go back to VT mode */
		fputs("\033\03", tout);
	} else {
		fputs("\037\030", tout);
	}
	fflush(tout);

#if SUN_OS && SYSTEM_V
	iocount = 0;
#endif /* SUN_OS && SYSTEM_V */
}

/**********************/
/* move - move cursor */
/**********************/

void move(x,y)
	int x,y;
{
	cx = x;
	cy = y;
	needsmove++;
}

/********************/
/* line - draw line */
/********************/

void line(x,y)
	int x,y;
{
	if(needsmove) {
		putc(29,tout);
		xy(cx,cy);
		needsmove = 0;
	}
	xy((int)x,(int)y);
}

/****************************/
/* linetype - set line type */
/****************************/

void linetype(type)
	register int type;
{
	static int otype = -1;
	static char ch[] = "acbd`";	/* See 4014 man appendix F */
	/* dotted,short dashed,dot dashed,longdashed,continuous */

	if(type >= 200)
		type = (type == GRIDlt)? 0:9;
	else if(type < 100)
		type = type % 10;
	else
		type = (type - 100)/10;
	type >>= 1;
	if(type == otype)
		return;
	putc(ESC,tout);
	putc(ch[type],tout);
	otype = type;
}

/********************/
/* text - draw text */
/********************/

void text(str)
	char *str;
{
#if SUN_OS && SYSTEM_V
	if(++iocount > 10) {
#if PRINT
		if(!ItsATekFile && !ItsATekPrinter)
#endif /* PRINT */
			fflush(tout);
		iocount = 0;
	}
#endif /* SUN_OS && SYSTEM_V */
	putc(29,tout);
	if(strlen(str) == 1 && !isdigit(*str)) {
		int yOff;

		if(islower(*str))
			yOff = 4;
		else if(*str == '*')
			yOff = 7;
		else if(*str == '.')
			yOff = 1;
		else
			yOff = 6;
		xy(cx - 5,cy - yOff);
	} else
		xy(cx,cy - 8);
	fprintf(tout,"\037%s",str);
	xyvalid = 0;
}

/************************/
/* center - center text */
/************************/

void center(str)
	char *str;
{
	cx -= strlen(str)*7;
	text(str);
}

/**************************************/
/* clear - clear the screen */
/**************************************/

void clear(void)
{
	fprintf(tout,"\035\033\f");
	xyvalid = 0;
}

/*************************************/
/* cursor - read the graphics cursor */
/*************************************/

void cursor(x,y,flag)
	int *x,*y;
	char *flag;
{
	char response[20];
	int ch;
	static int informed = 0;
	if(!informed) {
		printf("Place cursor, hit space (e to end sequence)\n");
		if(tType == XTERM) {
			printf("You must type rtn after 'space' or 'e'\n");
		}
		informed++;
	}
	if(tType == XTERM) {
		fputs("\033[?38h",tout);
	}
	fprintf(tout,"\035\033\032");		/* Print mode, turn on cursor */
	fflush(tout);
	fgets(response,20,tin);
	if(*response == '\n')
		fgets(response + 1,20,tin);
	*x = ((response[1] & 31) << 5) | (response[2] & 31);
	*y = ((response[3] & 31) << 5) | (response[4] & 31);
	*flag = (*response != 'e');
	xyvalid = 0;
	cx = *x - 4;
	cy = *y;
	text("X");
	cleanup();
	/* fflush(tout); */
}

/**********************************************/
/* xy - send xy coordinates in tek401x format */
/**********************************************/

static void xy(x,y)
register int x,y;
{
	char yh1,yl1,xh1,xl1;
	static char yh2,yl2,xh2;
	yh1 = 32 + ((y) >> 5);
	yl1 = 96 + ((y) & 31);
	xh1 = 32 + ((x) >> 5);
	xl1 = 64 + ((x) & 31);
	if(!xyvalid) {
		fprintf(tout,"%c%c%c%c",yh1,yl1,xh1,xl1);
		xyvalid++;
	} else {
		if(yh1 != yh2)
			putc(yh1,tout);
		if(yl1 != yl2 || xh1 != xh2)
			putc(yl1,tout);
		if(xh1 != xh2)
			putc(xh1,tout);
		putc(xl1,tout);
	}
	yh2 = yh1;
	yl2 = yl1;
	xh2 = xh1;
	cx = x;
	cy = y;
#if SUN_OS && SYSTEM_V
	iocount++;
#endif /* SUN_OS && SYSTEM_V */
}
