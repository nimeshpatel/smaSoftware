#include <curses.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <termio.h>
#include <time.h>

void timeStamp(void)
{
  time_t curTime;

  initscr();
#ifdef LINUX
  clear();
#endif
  move(0,0);
  curTime = time((long *)0);
  printw("TIMESTAMP %9d", (int)curTime);
  refresh();
  return;
}
