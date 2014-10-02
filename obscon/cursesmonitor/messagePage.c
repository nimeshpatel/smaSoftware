#include <curses.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <termio.h>
#include <time.h>

void messagePage(int count, int kind)
{
  static int localCount = 0;

  if ((count % 120) == 0) {
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
  move(11,26);
  if (kind == 1)
    printw("A message has been sent to you.");
  else if (kind == 2)
    printw("An Op. message has been posted.");
  move(13,30);
  if (kind == 1)
    printw("Type \"m\" to retrieve it.");
  else if (kind == 2)
    printw("Type \"O\" to see new message.");
  move(23,0);
  if (count == 0) {
    if ((localCount++ % 100) < 16) { /* % 60 yields a 36-second period */
      /*      printw("%d/%d",count,localCount);*/
      printw("\a",count,localCount);
    }
  } else {
    localCount = 0;
  }
  refresh();
  usleep(600000);
}
