#include <curses.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <termio.h>
#include <time.h>

void intruderPage(int count)
{

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
  move(11,28);
  printw("INTRUDER DETECTOR TRIPPED!");
  move(23,0);
  if (!(count % 3))
    printw("\a");
  refresh();
  usleep(600000);
}
