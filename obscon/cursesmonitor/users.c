#include <curses.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <termio.h>
#include <time.h>
#include <string.h>

int users(int count) {

  FILE *ps_pipe;
  FILE *who_pipe;

  int bytes_read;
  int nbytes = 80;
  char my_string[80];
  char who_string[80];
  char monitor_user[80];


  my_string[0]=(char)0;
  who_string[0]=(char)0;
  monitor_user[0]=(char)0;
  
  if ((count % 120) == 1) {
    /*
      Initialize Curses Display
    */
    initscr();
#ifdef LINUX
    clear();
#endif
    move(1,1);
    refresh();
  }
  move(0,28);
  printw("\"Curses Monitor\" Users");
  move(2,0);

#ifdef LINUX
  ps_pipe = popen ("ps au | grep monitor | awk '{print $1,\"  \",$7}'", "r");
#else
  ps_pipe = popen ("ps -ax | grep monitor | awk '{print $10,\"  \",$9}'", "r");
#endif
  if (!ps_pipe) {
    fprintf (stderr, "Opening ps pipe failed.\n");
    return EXIT_FAILURE;
  }
  
  do {
    char *fgetsPtr;

    fgetsPtr = fgets(my_string,30,ps_pipe);
    if (!feof(ps_pipe)) {
      printw ("%s", my_string);
#ifndef LINUX
      my_string[strlen(my_string)-1]=(char)0;
      who_string[0]=(char)0;
      sprintf(who_string,"who | grep \"%s\"",my_string);
      who_pipe= popen(who_string,"r");
      if (!who_pipe) {
	fprintf(stderr,"Opening who pipe failed.\n");
	return EXIT_FAILURE;
      }
      while (!feof(who_pipe)) {
	fgets(monitor_user,nbytes,who_pipe);
	if (!feof(who_pipe)) {
	  if(strcmp(monitor_user,""))
	    printw(":%s",monitor_user);
	}
	monitor_user[0]=(char)0;
      }
      pclose(who_pipe);
#endif
    }
  } while (!feof(ps_pipe));
  
  /* Close ps_pipe, checking for errors */
  if (pclose (ps_pipe) != 0) {
    fprintf (stderr, "Could not run 'ps'.\n");
  }
  move(23,0);
  refresh();
}
