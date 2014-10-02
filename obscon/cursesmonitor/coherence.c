#include <curses.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <termio.h>
#include <time.h>
#include "esma.h"
#include "dsm.h"
#include "monitor.h"
#include "upspage.h"
#include "coherence.h"
#include "dDSCursesMonitor.h" 
#define N_RECEIVERS 2
#define PRINT_DSM_ERRORS 0

extern int quit;
extern int verbose;

void coherence(int count)
{
  static int firstCall = TRUE;
  static float elevation = 0;
  int s, ant1, ant2, bslnCount, offset;
  int doWeCare[11], nAntAve[MAX_NUMBER_ANTENNAS+1];
  int isReceiverInArray[3];
  float coh[11][11][2], antAve[MAX_NUMBER_ANTENNAS+1];
  float coh2[11][11][2], antAve2[MAX_NUMBER_ANTENNAS+1];
  float aveUSB, aveLSB;
  float ave2USB, ave2LSB;
  char string1[100], source[24], antSource[34];
  static dsm_structure visibilityInfo;
  time_t timestamp, curTime;

  if ((count % 60) == 1) {
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
  if (firstCall) {
    s = dsm_structure_init(&visibilityInfo, "LAST_SCAN_VISIBILITES_X");
    if (s != DSM_SUCCESS) {
      dsm_error_message(s, "dsm_structure_init(LAST_SCAN_VISIBILITES_X) (0)");
      exit(1);
    } 
    firstCall = FALSE;
  }
  getAntennaList(doWeCare);
  getReceiverList(isReceiverInArray);
  s = dsm_read("m5",
	       "DSM_AS_SCAN_SOURCE_C24", 
	       (char *)source,
	       &timestamp);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_read - DSM_AS_SCAN_SOURCE_C24");
    quit = TRUE;
    return;
  }
  
  s = dsm_read("m5", "LAST_SCAN_VISIBILITES_X", &visibilityInfo, &timestamp);
  if (s != DSM_SUCCESS) { dsm_error_message(s, "LAST_SCAN_VISIBILITES_X (1)"); exit(1); }
  s = dsm_structure_get_element(&visibilityInfo, "CORR_V11_V11_V2_F",(void *)coh);
  if (s != DSM_SUCCESS) { dsm_error_message(s, "dsm_get_element(CORR_V11_V11_V2_F) (2)"); exit(1); }
  s = dsm_structure_get_element(&visibilityInfo, "CORR_H_V11_V11_V2_F",(void *)coh2);
  if (s != DSM_SUCCESS) { dsm_error_message(s, "dsm_get_element(CORR_H_V11_V11_V2_F) (3)"); exit(1); }

    /* Old, pre-structures, code
    s = dsm_read("m5",
		 "DSM_AS_SCAN_CORR_V11_V11_V2_F", 
		 (char *)coh,
		 &timestamp);

    s = dsm_read("m5",
		 "DSM_AS_SCAN_CORR_H_V11_V11_V2_F", 
		 (char *)coh2,
		 &timestamp);
    if (s != DSM_SUCCESS) {
      if (PRINT_DSM_ERRORS)
        dsm_error_message(s, "dsm_read - DSM_AS_SCAN_CORR_V11_V11_V2_F");
      quit = TRUE;
      return;
    }
    */
  strcpy(string1, ctime(&timestamp));
  string1[strlen(string1)-1] = (char)0;
  move(0,0);
  printw("Source %s", source);
  move(0,29);
  printw("Coherence for scan stored %s", string1);
  curTime = time((long *)0);
  strcpy(string1, ctime(&curTime));
  string1[strlen(string1)-1] = (char)0;
  for (ant1=1; ant1<=numberAntennas; ant1++) {
    if (doWeCare[ant1] == 1) {
      (void)rm_read(ant1,"RM_SOURCE_C34",antSource);
      if (strncmp(antSource,source,24) == 0) {
	(void)rm_read(ant1,"RM_ACTUAL_EL_DEG_F",&elevation);
      }
    }
  }
  if (numberAntennas <= 8) {
    offset = 1;
    move(1,0);
    printw("elevation ");
    if (elevation > 1) {
      if (elevation > 90) {
	printw("wacko");
      } else { 
	printw("%.0f",elevation);
      }
    } else {
      /* if you have just started monitor in the middle of a source change,
       * then you don't know the elevation of the prior source scan. So
       * instead of printing 0 degrees, print a question mark. */
      printw("?");
    }
    if (elevation <= 90) {
      printw(" deg");
    }
    move(1,42);
    printw("current time %s", string1);
    move(2,0);
  } else {
    offset = 0;
    printw("  (");
    printAgeNoStandout(curTime,timestamp);
    printw("ago)");
    move(1,0);
    move(23,82);
    printw("elevation ");
    if (elevation > 1) {
      if (elevation > 90) {
	printw("wacko");
      } else { 
	printw("%.0f",elevation);
      }
    } else {
      printw("?");
    }
    if (elevation <= 90) {
      printw(" deg");
    }
    move(1,0);
  }
  printw("Antenna      1");
  for (ant1=2; ant1<=numberAntennas; ant1++) {
    switch (ant1) {
    case CSO_ANTENNA_NUMBER:
      printw("      CSO");
      break;
    case JCMT_ANTENNA_NUMBER:
      printw("     JCMT");
      break;
    default:
      printw("        %d",ant1);
    }
  }
  bslnCount = 0;
  aveUSB = aveLSB = 0.0;
  for (ant1 = 1; ant1 <= numberAntennas; ant1++) {
    antAve[ant1] = 0.0;
    antAve2[ant1] = 0.0;
    nAntAve[ant1] = 0;
  }
  for (ant1 = 1; ant1 <= numberAntennas; ant1++) {
    move(2*ant1+offset,0);
    switch (ant1) {
    case CSO_ANTENNA_NUMBER:
      printw(" CSO USB ");
      break;
    case JCMT_ANTENNA_NUMBER:
      printw("JCMT USB ");
      break;
    default:
      printw("%4d USB ", ant1);
    }
    for (ant2 = 1; ant2 <= numberAntennas; ant2++) {
      if ((antsAvailable[ant1] && antsAvailable[ant2] &&
	  doWeCare[ant1] && doWeCare[ant2]) &&
	  (ant1 != ant2)) {
	if (ant1 > ant2) {
	  /* show low-freq rx = IF1 */
#define WACKO_COHERENCE 200
	  if (fabs(coh[ant2][ant1][1]) > WACKO_COHERENCE) {
	    printw("  wacko  ");
	  } else {
	    antAve[ant2] += coh[ant2][ant1][1];
	    antAve2[ant2] += coh2[ant2][ant1][1];
	    nAntAve[ant2] += 1;
	    printw("  %5.3f  ", coh[ant2][ant1][1]);
	  }
	  aveUSB += coh[ant2][ant1][1];
	  ave2USB += coh2[ant2][ant1][1];
	  bslnCount++;
	} else {
	  if (isReceiverInArray[2] == 0) {
	    printw("   off   ");
	  } else {
	    /* show high-freq rx = IF2 */
	    if (fabs(coh2[ant1][ant2][1]) > WACKO_COHERENCE) {
	      printw("  wacko  ");
	    } else {
	      antAve[ant2] += coh[ant1][ant2][1];
	      antAve2[ant2] += coh2[ant1][ant2][1];
	      nAntAve[ant2] += 1;
	      printw("  %5.3f  ", coh2[ant1][ant2][1]);
	    }
	  }
	}
      } else {
	if (ant1 == 1 && ant2 == 1) {
	  printw(" IF2 ===>");
	} else if (ant1 == numberAntennas && ant2 == numberAntennas) {
	  printw("         ");
	} else {
	  printw("  -----  ");
	}
      }
    }
    move(1+2*ant1+offset,0);
    printw("     LSB ");
    for (ant2 = 1; ant2 <= numberAntennas; ant2++) {
      if ((antsAvailable[ant1] && antsAvailable[ant2] &&
	  doWeCare[ant1] && doWeCare[ant2]) &&
	  (ant1 != ant2)) {
	if (ant1 > ant2) {
	  /* show low-freq rx = IF1 */
	  if (fabs(coh[ant2][ant1][0]) > WACKO_COHERENCE) {
	    printw("  wacko  ");
	  } else {
	    antAve[ant2] += coh[ant2][ant1][0];
	    antAve2[ant2] += coh2[ant2][ant1][0];
	    nAntAve[ant2] += 1;
	    printw("  %5.3f  ", coh[ant2][ant1][0]);
	  }
	  aveLSB += coh[ant2][ant1][0];
	  ave2LSB += coh2[ant2][ant1][0];
	} else {
	  if (isReceiverInArray[2] == 0) {
	    printw("   off   ");
	  } else { 
	    /* show hi-freq rx = IF2 */
	    if (fabs(coh2[ant1][ant2][0]) > WACKO_COHERENCE) {
	      printw("  wacko  ");
	    } else {
	      antAve[ant2] += coh[ant1][ant2][0];
	      antAve2[ant2] += coh2[ant1][ant2][0];
	      nAntAve[ant2] += 1;
	      printw("  %5.3f  ", coh2[ant1][ant2][0]);
	    }
	  }
	}
      } else {
	if (ant1 == 1 && ant2 == 1) {
	  printw("         ");
	} else if (ant1 == numberAntennas && ant2 == numberAntennas) {
	  printw("<=== IF1");
	} else {
	  printw("  -----  ");
	}
      }
    }
  }
  if (numberAntennas > 8) {
    move(22,0);
    printw("Ant Ave  ");
    for (ant1 = 1; ant1 <= numberAntennas; ant1++) {
      if (antsAvailable[ant1] &&
	  doWeCare[ant1] && (nAntAve[ant1] != 0)) {
	if (fabs(antAve[ant1]/((float)nAntAve[ant1])) > WACKO_COHERENCE) {
	  printw("  wacko  ");
	} else {
	  printw(" .%02d\\.%02d ", 
		 (short)floor(100*antAve[ant1]/((float)nAntAve[ant1])),
		 (short)floor(100*antAve2[ant1]/((float)nAntAve[ant1])));
	  /*	     ((float)nAntAve[ant1]));*/
	}
      } else {
	printw("  -----  ");
      }
    }
  } else {
    if (isReceiverInArray[2]) {
      move(21,0);
      printw("AntAveIF2");
      for (ant1 = 1; ant1 <= numberAntennas; ant1++) {
	if (antsAvailable[ant1] &&
	    doWeCare[ant1] && (nAntAve[ant1] != 0)) {
	  if (fabs(antAve2[ant1]/((float)nAntAve[ant1])) > WACKO_COHERENCE) {
	    printw("  wacko  ");
	  } else {
	    printw("  %5.3f  ", antAve2[ant1]/((float)nAntAve[ant1]));
	    /*	     ((float)nAntAve[ant1]));*/
	  }
	} else {
	  printw("  -----  ");
	}
      }
    }
    move(22,0);
    printw("AntAveIF1");
    for (ant1 = 1; ant1 <= numberAntennas; ant1++) {
      if (antsAvailable[ant1] &&
	  doWeCare[ant1] && (nAntAve[ant1] != 0)) {
	if (fabs(antAve[ant1]/((float)nAntAve[ant1])) > WACKO_COHERENCE) {
	  printw("  wacko  ");
	} else {
	  printw("  %5.3f  ", antAve[ant1]/((float)nAntAve[ant1]));
	  /*	     ((float)nAntAve[ant1]));*/
	}
      } else {
	printw("  -----  ");
      }
    }
  }
  move(23,0);
  aveUSB /= (float)bslnCount;
  aveLSB /= (float)bslnCount;
  ave2USB /= (float)bslnCount;
  ave2LSB /= (float)bslnCount;
  printw("IF1 AveUSB ");
  if (fabs(aveUSB) > WACKO_COHERENCE) {
    printw(" wacko");
  } else {
    printw("%5.3f",aveUSB);
  }
  printw(", LSB ");
  if (fabs(aveLSB) > WACKO_COHERENCE) {
    printw(" wacko");
  } else {
    printw("%5.3f",aveLSB);
  }
  printw(", both ");
  if (fabs((aveUSB+aveLSB)*0.5) > WACKO_COHERENCE) {
    printw(" wacko");
  } else {
    printw("%5.3f",(aveUSB+aveLSB)*0.5);
  }
  if (isReceiverInArray[2]) {
    printw("; IF2 AveUSB ");
    if (fabs(ave2USB) > WACKO_COHERENCE) {
      printw(" wacko");
    } else {
      printw("%5.3f",ave2USB);
    }
    printw(", LSB ");
    if (fabs(ave2LSB) > WACKO_COHERENCE) {
      printw(" wacko");
    } else {
      printw("%5.3f",ave2LSB);
    }
    printw(", both ");
    if (fabs((ave2USB+ave2LSB)*0.5) > WACKO_COHERENCE) {
      printw(" wacko");
    } else {
      printw("%5.3f",(ave2USB+ave2LSB)*0.5);
    }
  }
  if (numberAntennas <= 8) {
    move(20,32);
  }
  move(0,79);
  refresh();
  return;
}
