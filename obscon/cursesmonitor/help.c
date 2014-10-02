#include <curses.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <termio.h>
#include <time.h>

struct page_descriptions {
    char cmd;		/* The char one types to get that page */
    char num_slots;	/* the number of 1/4 line units. 0 means unused cmd */
    char *desc;		/* A short description the user will recognize */
} pd[] = {
    {'1', 2, "-> \"8\" Individual antenna info."},
    {'a', 1, "Array summary"},
    {'A', 1, "Air Conditioner"},
    {'b', 1, "Blower page"},
    {'B', 1, "BDC STATUS"},
    {'c', 1, "Correlator page"},
    {'C', 1, "crate pages"},
    {'d', 1, "DDS page"},
    {'D', 1, "De-ice"},
    {'e', 1, "Error messages"},
    {'E', 1, "eSMA xterm mode"},
    {'f', 1, "First dwncvtr"},
    {'F', 1, "Flagging"},
    {'g', 1, "MRG page"},
    {'G', 1, "GPS receiver"},
    {'h', 1, "\"?\" This page"},
    {'H', 1, "Hangar page"},
    {'i', 1, "C. Room IF/LO"},
    {'I', 1, "Antenna IF/LO"},
    {'j', 1, "Genset status"},
    {'J', 0, ""},
    {'k', 1, "Coherence"},
    {'K', 0, ""},
    {'l', 1, "Command hist."},
    {'L', 0, ""},
    {'m', 1, "2op Messages"},
    {'M', 1, "Map of array"},
    {'n', 1, "iPCensus page"},
    {'N', 0, ""},
    {'o', 1, "Optics page"},
    {'O', 1, "Operator Mess."},
    {'p', 1, "Project info."},
    {'P', 1, "Pointing models"},
    {'q', 1, "Quit"},
    {'Q', 0, ""},
    {'r', 1, "Receiver"},
    {'R', 1, "Rscan"},
    {'s', 1, "smainit stat."},
    {'S', 1, "Phase Stability"},
    {'t', 1, "Chopper"},
    {'T', 1, "Tilt meter page"},
    {'u', 1, "Antenna UPS"},
    {'U', 1, "PowerPC uptimes"},
    {'v', 1, "deviceDrivers"},
    {'V', 1, "Allan Variance"},
    {'w', 1, "Weather page"},
    {'W', 1, "Weather page2"},
    {'x', 1, "List dead proc."},
    {'X', 1, "SWARM Status"},
    {'y', 1, "Cryostat"},
    {'Y', 1, "YIG Frequencies"},
    {'z', 2, "Toggle Op. messages ignore flag"},
    {'Z', 0, ""},
    {'0', 0, ""},
    {'9', 0, ""},
    {'+', 1, "Cycle subpage"},
    {'%', 1, "Polarization"},
    {' ', 1, "Array Summary"},
    {'#', 2, "Change IF/LO units"},
    {'*', 1, "Ant. Effeciency"},
    {'/', 2, "List monitor users."}
};
#define NUM_PD (sizeof(pd) / sizeof(pd[0]))

void help(int count) {
  int line, col;
  int pdn;			/* page description number */
  char unused[24], *uup;	/* For the list of unused cmds */

  /* The help page only changes when the structure above is changed
   * and this file recompiled, so there is no need to recalculate the
   * display except when it is being redrawn.
   */
  if ((count % 120) != 1) return;

  /* Initialize Curses Display */
  initscr();
#ifdef LINUX
  clear();
#endif
  move(1,1);
  refresh();
  move(0,28);
  printw("\"Curses Monitor\" Help");
  move(2,0);
  printw(
"This program is controlled by single character commands which do not need to\n"
"be terminated by a carriage return.   If the display is jumbled, check that\n"
  );
  printw(
"the window size is set to 24 lines of 80 characters, and that the TERM environ"
"-\nment variable is set to something with an entry in the /etc/termcap file.\n"
  );
  move(7,0);
  printw("Commands:");

  uup = unused;
  line = 9;
  col = 0;
  for(pdn = 0; pdn < NUM_PD; pdn++) {
    if(pd[pdn].num_slots == 0) {
      *uup++ = pd[pdn].cmd;
    } else {
      if(pd[pdn].num_slots > 1 && col > 40) {
	col = 0;
	line++;
      }
      move(line, col);
      printw("\"%c\" %s", pd[pdn].cmd, pd[pdn].desc);
      col += 20 * pd[pdn].num_slots;
      if(col >= 80) {
	col = 0;
	line++;
      }
    }
  }
  *uup = 0;
  move(23, 0);
  printw("Major unused keys remaining: %s (They're going fast)", unused);
  refresh();
}
